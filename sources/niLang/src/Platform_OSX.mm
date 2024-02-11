#include "API/niLang/Types.h"

#if defined niOSX || defined niIOS

#include "Platform_OSX.h"
#include "Lang.h"
#include "FileFd.h"
#include "API/niLang/Utils/CrashReport.h"

#include <mach/mach_time.h>
#include <sys/sysctl.h>

using namespace ni;

NSString* _ToNSString(const ni::achar* aString) {
  return [[NSString alloc] initWithUTF8String:aString];
}

///////////////////////////////////////////////
niExportFunc(uint64_t) __niMach_TimerNano() {
  static uint64_t start = 0;
  static mach_timebase_info_data_t info = {0,0};
  if (info.denom == 0) {
    mach_timebase_info(&info);
    start = mach_absolute_time();
  }
  uint64_t end = mach_absolute_time();
  uint64_t difference = end - start;
  return difference * (info.numer / info.denom);
}

///////////////////////////////////////////////
niExportFunc(int) __niOSX_AmIBeingDebugged(void)
// Returns true if the current process is being debugged (either
// running under the debugger or has a debugger attached post facto).
{
  int                 junk;
  int                 mib[4];
  struct kinfo_proc   info;
  size_t              size;

  // Initialize the flags so that, if sysctl fails for some bizarre
  // reason, we get a predictable result.
  info.kp_proc.p_flag = 0;

  // Initialize mib, which tells sysctl the info we want, in this case
  // we're looking for information about a specific process ID.
  mib[0] = CTL_KERN;
  mib[1] = KERN_PROC;
  mib[2] = KERN_PROC_PID;
  mib[3] = getpid();

  // Call sysctl.
  size = sizeof(info);
  junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
  assert(junk == 0);
  niUnused(junk);

  // We're being debugged if the P_TRACED flag is set.
  return ( (info.kp_proc.p_flag & P_TRACED) != 0 );
}

///////////////////////////////////////////////
niExportFunc(int) _OSXSetClipboardText(const char *text)
{
  if (!niStringIsOK(text)) {
    text = "";
  }
#ifdef niIOS
  UIPasteboard *thePasteboard = [UIPasteboard generalPasteboard];
  thePasteboard.string = [NSString stringWithUTF8String:text];
  return 0;
#else
  // niDebugFmt(("... _OSXSetClipboardText: %s", text));
  NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
  NSString* string = [[NSString alloc] initWithBytesNoCopy:(void*)text
                      length:StrSize(text)
                      encoding:NSUTF8StringEncoding
                      freeWhenDone:NO];
  [pasteboard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];
  return [pasteboard setString:string forType:NSStringPboardType];
#endif
}

///////////////////////////////////////////////
niExportFunc(int) _OSXOpenURL(const char* aUrl)
{
  if (!niStringIsOK(aUrl)) {
    return 0;
  }

  // niDebugFmt(("... _OSXOpenURL: '%s'", aUrl));

  NSString* nsUrl = [NSString stringWithUTF8String:aUrl];
#ifdef niIOS
  [[UIApplication sharedApplication] openURL:[NSURL URLWithString:nsUrl]];
#else
  [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:nsUrl]];
#endif
  return 1;
}

niExportFuncCPP(cString) _OSXGetClipboardText()
{
#ifdef niIOS
  UIPasteboard *thePasteboard = [UIPasteboard generalPasteboard];
  NSString *pasteboardString = thePasteboard.string;
  return [pasteboardString UTF8String];
#else
  NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
  NSString* string = [pasteboard stringForType:NSPasteboardTypeString];
  return [string UTF8String];
#endif
}

///////////////////////////////////////////////
cString _GetCommandLine(void) {
  cString o;
  NSArray* args = [[NSProcessInfo processInfo] arguments];
  niLoop(i,[args count]) {
    NSString* arg = [args objectAtIndex:i];
    const char* argUtf8 = [arg UTF8String];
    if (ni::StrChr(argUtf8,' ') || ni::StrChr(argUtf8,'\t')) {
      o << "\"" << argUtf8 << "\" ";
    }
    else {
      o << argUtf8 << " ";
    }
  }
  return o;
}

///////////////////////////////////////////////
void cLang::_PlatformExit(tU32 aulErrorCode) {
  exit(aulErrorCode);
}

///////////////////////////////////////////////
void cLang::SetEnv(const achar* aaszEnv, const achar* aaszValue) const {
  cString envStr;
  envStr = aaszEnv;
  envStr += "=";
  envStr += aaszValue;
  aputenv(envStr.Chars());
}
cString cLang::GetEnv(const achar* aaszEnv) const {
  return agetenv(aaszEnv);
}

///////////////////////////////////////////////
niExportFunc(achar*) ni_get_exe_path(ni::achar* buffer) {
  NSArray* args = [[NSProcessInfo processInfo] arguments];
  NSString* arg = [args objectAtIndex:0];
  StrZCpy(buffer,AMAX_PATH,[arg UTF8String]);
  return buffer;
}

///////////////////////////////////////////////
static achar* _GetDirSpecial(achar* buffer, NSSearchPathDirectory aDir) {
  // Get documents directory
  NSString* dir = [NSSearchPathForDirectoriesInDomains(aDir, NSUserDomainMask, YES) lastObject];
  StrZCpy(buffer,AMAX_PATH,[dir UTF8String]);
  return buffer;
}

achar* _AppleGetDirDocs(achar* buffer) {
  return _GetDirSpecial(buffer,NSDocumentDirectory);
}
achar* _AppleGetDirDownloads(achar* buffer) {
  return _GetDirSpecial(buffer,NSDownloadsDirectory);
}
achar* _AppleGetDirHome(achar* buffer) {
  NSString* dir = NSHomeDirectory();
  StrZCpy(buffer,AMAX_PATH,[dir UTF8String]);
  return buffer;
}
achar* _AppleGetDirTemp(achar* buffer) {
  NSString* dir = NSTemporaryDirectory();
  StrZCpy(buffer,AMAX_PATH,[dir UTF8String]);
  return buffer;
}

cString _AppleGetInfoPlistPropertyValue(const achar* aProperty) {
  @autoreleasepool {
    NSBundle *mainBundle = [NSBundle mainBundle];
    niCheck(mainBundle, AZEROSTR);
    NSString *propertyKey = _ToNSString(aProperty);
    niCheck(propertyKey, AZEROSTR);
    NSString *propertyValue = [mainBundle objectForInfoDictionaryKey:propertyKey];
    niCheck(propertyValue, AZEROSTR);
    return _ASTR([propertyValue UTF8String]);
  }
}

//--------------------------------------------------------------------------------------------
//
//  iOS only
//
//--------------------------------------------------------------------------------------------

#ifdef niIOS
///////////////////////////////////////////////
void cLang::FatalError(const achar* aszMsg) {
  // Log message
  cString logMessage = niFmt(_A("[FATAL ERROR]\n%s\n"), aszMsg);
  niLog(Error,logMessage);

  _PlatformExit(0xDEADBEEF);
}
#endif

#endif // niOSX || niIOS
