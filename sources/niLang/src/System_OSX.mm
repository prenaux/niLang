#include "API/niLang/Types.h"

#ifdef niOSX
#include "Platform_OSX.h"
#include "Lang.h"
#include "API/niLang_ModuleDef.h"
#include "API/niLang/IOSWindow.h"
#include <niLang/Utils/CollectionImpl.h>
#include <niLang/Utils/TimerSleep.h>
#include "API/niLang/Platforms/OSX/osxgl.h"
#include <CoreVideo/CVDisplayLink.h>
#include <niLang/STL/scope_guard.h>
#include <niLang/STL/set.h>

// Needed for _NSGetProgname
#include <crt_externs.h>

using namespace ni;

#define OSX_SWITCH_INOUT_ON_FOCUS

#define TRACE_OSX(X) // niDebugFmt(X)

//
// TouchEvents seem to be normalized (in the 0-1 range) - so not very useful
// as-is. It leads to the mouse position being stuck in the top corner of the
// screen.
//
// #define HANDLE_TOUCH_EVENTS

static id<NSApplicationDelegate> _appDelegate;
static tBool _bAppHasBeenRegistered = eFalse;

// In the foreground, simulate a purely event driven loop. Use 100ms
// (10fps) so that the app still feels reactive when it reactivates.
static const tU32 _knEventDrivenWindowTimerMs_Active = 100;
// Minimized, agressively wait.
static const tU32 _knEventDrivenWindowTimerMs_Minimized = 1500;
// In the background, longer wait to consume as little bg resource as possible.
static const tU32 _knEventDrivenWindowTimerMs_Background = 500;

static void _CreateApplicationMenus(id<NSApplicationDelegate> appDelegate);
static tU32 _PumpEvents();

static tBool _bSetDockTileLabelFromWindowTitle = eFalse;
niExportFunc(void) osxSetDockTileLabelFromWindowTitle(const tBool abDoSet) {
  _bSetDockTileLabelFromWindowTitle = abDoSet;
}
niExportFunc(void) osxSetDockTileLabel(const achar* aaszLabel);

/* Mac virtual key code to SDL scancode mapping table
   Sources:
   - Inside Macintosh: Text <http://developer.apple.com/documentation/mac/Text/Text-571.html>
   - Apple USB keyboard driver source <http://darwinsource.opendarwin.org/10.4.6.ppc/IOHIDFamily-172.8/IOHIDFamily/Cosmo_USB2ADB.c>
   - experimentation on various ADB and USB ISO keyboards and one ADB ANSI keyboard
*/
struct sOSXScanCode {
  eKey niKey;
  tBool canPrint;
};
static const sOSXScanCode _ScancodeUnknown = { eKey_Unknown, eFalse };
static const sOSXScanCode _ScancodeTable[] = {
  /* 00 */ { eKey_A, eTrue },
  /* 01 */ { eKey_S, eTrue },
  /* 02 */ { eKey_D, eTrue },
  /* 03 */ { eKey_F, eTrue },
  /* 04 */ { eKey_H, eTrue },
  /* 05 */ { eKey_G, eTrue },
  /* 06 */ { eKey_Z, eTrue },
  /* 07 */ { eKey_X, eTrue },
  /* 08 */ { eKey_C, eTrue },
  /* 09 */ { eKey_V, eTrue },
  /* 0a */ { eKey_Grave, eTrue },
  /* 0b */ { eKey_B, eTrue },
  /* 0c */ { eKey_Q, eTrue },
  /* 0d */ { eKey_W, eTrue },
  /* 0e */ { eKey_E, eTrue },
  /* 0f */ { eKey_R, eTrue },
  /* 10 */ { eKey_Y, eTrue },
  /* 11 */ { eKey_T, eTrue },
  /* 12 */ { eKey_n1, eTrue },
  /* 13 */ { eKey_n2, eTrue },
  /* 14 */ { eKey_n3, eTrue },
  /* 15 */ { eKey_n4, eTrue },
  /* 16 */ { eKey_n6, eTrue },
  /* 17 */ { eKey_n5, eTrue },
  /* 18 */ { eKey_Equals, eTrue },
  /* 19 */ { eKey_n9, eTrue },
  /* 1a */ { eKey_n7, eTrue },
  /* 1b */ { eKey_Minus, eTrue },
  /* 1c */ { eKey_n8, eTrue },
  /* 1d */ { eKey_n0, eTrue },
  /* 1e */ { eKey_RBracket, eTrue },
  /* 1f */ { eKey_O, eTrue },
  /* 20 */ { eKey_U, eTrue },
  /* 21 */ { eKey_LBracket, eTrue },
  /* 22 */ { eKey_I, eTrue },
  /* 23 */ { eKey_P, eTrue },
  /* 24 */ { eKey_Enter, eTrue },
  /* 25 */ { eKey_L, eTrue },
  /* 26 */ { eKey_J, eTrue },
  /* 27 */ { eKey_Apostrophe, eTrue },
  /* 28 */ { eKey_K, eTrue },
  /* 29 */ { eKey_Semicolon, eTrue },
  /* 2a */ { eKey_BackSlash, eTrue },
  /* 2b */ { eKey_Comma, eTrue },
  /* 2c */ { eKey_Slash, eTrue },
  /* 2d */ { eKey_N, eTrue },
  /* 2e */ { eKey_M, eTrue },
  /* 2f */ { eKey_Period, eTrue },
  /* 30 */ { eKey_Tab, eTrue },
  /* 31 */ { eKey_Space, eTrue },
  /* 32 */ { eKey_Grave, eTrue },
  /* 33 */ { eKey_BackSpace, eFalse },
  /* 34 */ { eKey_Unknown, eFalse },
  /* 35 */ { eKey_Escape, eFalse },
  /* 36 */ { eKey_RWin, eFalse },
  /* 37 */ { eKey_LWin, eFalse },
  /* 38 */ { eKey_LShift, eFalse },
  /* 39 */ { eKey_CapsLock, eFalse },
  /* 3a */ { eKey_LAlt, eFalse },
  /* 3b */ { eKey_LControl, eFalse },
  /* 3c */ { eKey_RShift, eFalse },
  /* 3d */ { eKey_RAlt, eFalse },
  /* 3e */ { eKey_RControl, eFalse },
  /* 3f */ { eKey_Unknown, eFalse }, /* Function */
  /* 40 */ { eKey_Unknown, eFalse }, /* eKey_F17 */
  /* 41 */ { eKey_NumPadPeriod, eTrue },
  /* 42 */ { eKey_Unknown, eFalse },
  /* 43 */ { eKey_NumPadStar, eTrue },
  /* 44 */ { eKey_Unknown, eFalse },
  /* 45 */ { eKey_NumPadPlus, eTrue },
  /* 46 */ { eKey_Unknown, eFalse },
  /* 47 */ { eKey_NumLock, eFalse }, /* Really KeypadClear... */
  /* 48 */ { eKey_Unknown, eFalse }, /* VolumeUp */
  /* 49 */ { eKey_Unknown, eFalse }, /* VolumeDown */
  /* 4a */ { eKey_Unknown, eFalse }, /* Mute */
  /* 4b */ { eKey_NumPadSlash, eTrue },
  /* 4c */ { eKey_NumPadEnter, eFalse },
  /* 4d */ { eKey_Unknown, eFalse },
  /* 4e */ { eKey_NumPadMinus, eTrue },
  /* 4f */ { eKey_Unknown, eFalse }, /* F18 */
  /* 50 */ { eKey_Unknown, eFalse }, /* F19 */
  /* 51 */ { eKey_NumPadEquals, eTrue },
  /* 52 */ { eKey_NumPad0, eTrue },
  /* 53 */ { eKey_NumPad1, eTrue },
  /* 54 */ { eKey_NumPad2, eTrue },
  /* 55 */ { eKey_NumPad3, eTrue },
  /* 56 */ { eKey_NumPad4, eTrue },
  /* 57 */ { eKey_NumPad5, eTrue },
  /* 58 */ { eKey_NumPad6, eTrue },
  /* 59 */ { eKey_NumPad7, eTrue },
  /* 5a */ { eKey_Unknown, eFalse }, /* F20 */
  /* 5b */ { eKey_NumPad8, eTrue },
  /* 5c */ { eKey_NumPad9, eTrue },
  /* 5d */ { eKey_Unknown, eFalse },
  /* 5e */ { eKey_Unknown, eFalse },
  /* 5f */ { eKey_Unknown, eFalse },
  /* 60 */ { eKey_F5, eFalse },
  /* 61 */ { eKey_F6, eFalse },
  /* 62 */ { eKey_F7, eFalse },
  /* 63 */ { eKey_F3, eFalse },
  /* 64 */ { eKey_F8, eFalse },
  /* 65 */ { eKey_F9, eFalse },
  /* 66 */ { eKey_Unknown, eFalse },
  /* 67 */ { eKey_F11, eFalse },
  /* 68 */ { eKey_Unknown, eFalse },
  /* 69 */ { eKey_PrintScreen, eFalse },
  /* 6a */ { eKey_Unknown, eFalse }, /* F16 */
  /* 6b */ { eKey_F14, eFalse },
  /* 6c */ { eKey_Unknown, eFalse },
  /* 6d */ { eKey_F10, eFalse },
  /* 6e */ { eKey_Unknown, eFalse },
  /* 6f */ { eKey_F12, eFalse },
  /* 70 */ { eKey_Unknown, eFalse },
  /* 71 */ { eKey_F15, eFalse },
  /* 72 */ { eKey_Insert, eFalse }, /* Really Help... */
  /* 73 */ { eKey_Home, eFalse },
  /* 74 */ { eKey_PgUp, eFalse },
  /* 75 */ { eKey_Delete, eFalse },
  /* 76 */ { eKey_F4, eFalse },
  /* 77 */ { eKey_End, eFalse },
  /* 78 */ { eKey_F2, eFalse },
  /* 79 */ { eKey_PgDn, eFalse },
  /* 7a */ { eKey_F1, eFalse },
  /* 7b */ { eKey_Left, eFalse },
  /* 7c */ { eKey_Right, eFalse },
  /* 7d */ { eKey_Down, eFalse },
  /* 7e */ { eKey_Up, eFalse },
  /* 7f */ { eKey_Unknown, eFalse },
};

//--------------------------------------------------------------------------------------------
//
//  Cocoa Wrapper Definition
//
//--------------------------------------------------------------------------------------------
class cOSXWindow;

static void _RegisterWindow(cOSXWindow* apWindow);
static void _UnregisterWindow(cOSXWindow* apWindow);
static void _TerminateApp();

static void _SendWindowTextInput(cOSXWindow* apWindow, const char* aaszText) {
  TRACE_OSX(("... OSX: _SendWindowTextInput: %s", aaszText));
}

static void _SendWindowEditingText(cOSXWindow* apWindow, const char* aaszText, int loc, int len) {
  TRACE_OSX(("... OSX: _SendWindowEditingText: %s [%d,%d]",
             aaszText, loc, len));
}

static void _FlipNSRect(const cOSXWindow* apWindow, NSRect *r)
{
  /* FIXME: Cache the display used for this window */
  r->origin.y = CGDisplayPixelsHigh(kCGDirectMainDisplay) - r->origin.y - r->size.height;
}
static NSRect _ToNSRect(const cOSXWindow* apWindow, const sRecti& aRect) {
  NSRect r;
  r.origin.x = aRect.x;
  r.origin.y = aRect.y;
  r.size.width = aRect.GetWidth();
  r.size.height = aRect.GetHeight();
  _FlipNSRect(apWindow,&r);
  return r;
}

static unsigned int _ToNSWindowStyle(tOSWindowStyleFlags aStyle) {
  unsigned int style;
  if (aStyle & eOSWindowStyleFlags_Overlay) {
    style = NSBorderlessWindowMask;
  }
  else {
    style = NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask;
  }
  if (niFlagIsNot(aStyle,eOSWindowStyleFlags_FixedSize)) {
    style |= NSResizableWindowMask;
  }
  return style;
}
static NSString* _ToNSString(const ni::achar* aString) {
  return [[NSString alloc] initWithUTF8String:aString];
}

static bool _cursorVisible = true;
static void _ShowCursor() {
	if (!_cursorVisible) {
		_cursorVisible = true;
		[NSCursor unhide];
	}
}
static void _HideCursor() {
	if (_cursorVisible) {
		_cursorVisible = false;
		[NSCursor hide];
	}
}

static void _WrapCursorPosition(const sVec2i& abAbsPos) {
  CGPoint location;
  location.x = abAbsPos.x;
  location.y = abAbsPos.y;
  CGWarpMouseCursorPosition(location);
}

//////////////////////////////////////////////////////////////////////////////////////////////
@interface NIWindowDelegate : NSResponder <NSWindowDelegate>
{
  cOSXWindow* _wnd;
}

-(void) listen:(cOSXWindow*)data;
-(void) close;

/* Window delegate functionality */
-(BOOL) windowShouldClose:(id) sender;
-(void) windowDidExpose:(NSNotification *) aNotification;
-(void) windowDidMove:(NSNotification *) aNotification;
-(void) windowDidResize:(NSNotification *) aNotification;
-(void) windowDidMiniaturize:(NSNotification *) aNotification;
-(void) windowDidDeminiaturize:(NSNotification *) aNotification;
-(void) windowDidBecomeKey:(NSNotification *) aNotification;
-(void) windowDidResignKey:(NSNotification *) aNotification;
-(void) windowDidChangeBackingProperties:(NSNotification *) aNotification;
-(void) windowWillEnterFullScreen:(NSNotification *) aNotification;
-(void) windowDidEnterFullScreen:(NSNotification *) aNotification;
-(void) windowWillExitFullScreen:(NSNotification *) aNotification;
-(void) windowDidExitFullScreen:(NSNotification *) aNotification;
@end

//////////////////////////////////////////////////////////////////////////////////////////////
@interface NIWindow : NSWindow
@end

@implementation NIWindow
- (BOOL) canBecomeKeyWindow {
  return YES;
}
- (BOOL) canBecomeMainWindow {
  return YES;
}
@end

//////////////////////////////////////////////////////////////////////////////////////////////
@interface NIApplication : NSApplication
{
}
@end

@implementation NIApplication

// From http://cocoadev.com/index.pl?GameKeyboardHandlingAlmost
// This works around an AppKit bug, where key up events while holding
// down the command key don't get sent to the key window.
- (void)sendEvent:(NSEvent *)event
{
  if ([event type] == NSEventTypeKeyUp &&
  ([event modifierFlags] & NSEventModifierFlagCommand))
  {
    [[self keyWindow] sendEvent:event];
  }
  else
    [super sendEvent:event];
}


// No-op thread entry point
//
- (void)doNothing:(id)object
{
}

- (void)loadMainMenu
{
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 100800
  NSArray* nibObjects;
  [[NSBundle mainBundle] loadNibNamed:@"MainMenu"
   owner:NSApp
   topLevelObjects:&nibObjects];
#else
  [[NSBundle mainBundle] loadNibNamed:@"MainMenu" owner:NSApp];
#endif
}
@end

// setAppleMenu disappeared from the headers in 10.4
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
@interface NSApplication(NSAppleMenu)
- (void)setAppleMenu:(NSMenu *)menu;
@end
#endif

@implementation NSApplication(TS)
- (void)setRunning
{
}
@end

@interface NIAppDelegate : NSObject <NSApplicationDelegate>
- (BOOL)application:(NSApplication *)sender openFile:(NSString *)aFilename;
- (BOOL)application:(NSApplication *)sender openFiles:(NSArray *)aFilenames;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
- (void)requestClose:(NSApplication *)sender;
@end

@implementation NIAppDelegate : NSObject
- (void)applicationWillFinishLaunching:(NSNotification *)notification
{
  TRACE_OSX(("... OSX: applicationWillFinishLaunching"));

  [NSApp activateIgnoringOtherApps:YES];

  NSDictionary *appDefaults = [[NSDictionary alloc] initWithObjectsAndKeys:
                                 [NSNumber numberWithBool:NO], @"AppleMomentumScrollSupported",
                                 [NSNumber numberWithBool:NO], @"ApplePressAndHoldEnabled",
                                 [NSNumber numberWithBool:YES], @"ApplePersistenceIgnoreState",
                                 nil];
  [[NSUserDefaults standardUserDefaults] registerDefaults:appDefaults];
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
  TRACE_OSX(("... OSX: applicationDidFinishLaunching"));
  _PumpEvents();
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
  TRACE_OSX(("... OSX: applicationShouldTerminate"));
  _TerminateApp();
  return NSTerminateCancel;
}

-(BOOL)application:(NSApplication *)sender openFile:(NSString *)aFilename
{
  NSLog(@"openFile: %@", aFilename);
  const cString strFilename = [aFilename UTF8String];
  ni::GetLang()->SetProperty("OpenFile", strFilename.Chars());
  // ni::GetLang()->MessageBox(NULL, "OSX Info", niFmt("openFile: %s", strFilename), ni::eOSMessageBoxFlags_Ok);
  ni::SendMessages(
      ni::GetLang()->GetSystemMessageHandlers(),
      eSystemMessage_OpenFile,strFilename,niVarNull);
  return YES;
}

-(BOOL)application:(NSApplication *)sender openFiles:(NSArray *)aFilenames
{
  NSLog(@"openFiles: %@", aFilenames);
  niLoop(i,[aFilenames count]) {
    const cString strFilename = [aFilenames[i] UTF8String];
    ni::GetLang()->SetProperty("OpenFile", strFilename.Chars());
    // ni::GetLang()->MessageBox(NULL, "OSX Info", niFmt("openFiles[%d]: %s", i, strFilename), ni::eOSMessageBoxFlags_Ok);
    ni::SendMessages(
        ni::GetLang()->GetSystemMessageHandlers(),
        eSystemMessage_OpenFile,strFilename,niVarNull);
  }
  return YES;
}
- (void)requestClose:(NSApplication *)sender {
  TRACE_OSX(("... OSX: requestClose"));
  _TerminateApp();
}
@end

static NSString* _GetApplicationName() {
  NSString *appName = 0;
  if (!appName) {
    cString propAppName = GetLang()->GetProperty("ni.app.name");
    if (propAppName.IsNotEmpty()) {
      appName = _ToNSString(propAppName.Chars());
    }
    else {
      char** progname = _NSGetProgname();
      if (progname && *progname)
        appName = _ToNSString(*progname);
      else
        appName = _ToNSString("niApp");
    }
  }
  return appName;
}

static void _CreateApplicationMenus(id<NSApplicationDelegate> appDelegate)
{
  NSString *appName;
  NSString *title;
  NSMenu *appleMenu;
  NSMenu *windowMenu;
  NSMenuItem *menuItem;

  /* Create the main menu bar */
  [NSApp setMainMenu:[[NSMenu alloc] init]];

  /* Create the application menu */
  appName = _GetApplicationName();
  appleMenu = [[NSMenu alloc] initWithTitle:@""];

  /* Add menu items */
  title = [@"About " stringByAppendingString:appName];
  [appleMenu addItemWithTitle:title action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];

  [appleMenu addItem:[NSMenuItem separatorItem]];

  title = [@"Hide " stringByAppendingString:appName];
  [appleMenu addItemWithTitle:title action:@selector(hide:) keyEquivalent:@/*"h"*/""];

  menuItem = (NSMenuItem *)[appleMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@/*"h"*/""];
  [menuItem setKeyEquivalentModifierMask:(NSAlternateKeyMask|NSCommandKeyMask)];

  [appleMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];

  [appleMenu addItem:[NSMenuItem separatorItem]];

  title = [@"Quit " stringByAppendingString:appName];
  menuItem = [appleMenu addItemWithTitle:title action:@selector(requestClose:) keyEquivalent:@"q"];
  [menuItem setTarget: appDelegate];

  /* Put menu into the menubar */
  menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
  [menuItem setSubmenu:appleMenu];
  [[NSApp mainMenu] addItem:menuItem];

  /* Tell the application object that this is now the application menu */
  [NSApp setAppleMenu:appleMenu];

  /* Create the window menu */
  windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];

  /* "Minimize" item */
  menuItem = [[NSMenuItem alloc] initWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@/*"m"*/""];
  [windowMenu addItem:menuItem];

  /* Put menu into the menubar */
  menuItem = [[NSMenuItem alloc] initWithTitle:@"Window" action:nil keyEquivalent:@""];
  [menuItem setSubmenu:windowMenu];
  [[NSApp mainMenu] addItem:menuItem];

  /* Tell the application object that this is now the window menu */
  [NSApp setWindowsMenu:windowMenu];
}

niExportFunc(void) osxSetDockTileLabel(const achar* aaszLabel) {
  NSDockTile* dockTile = [[NSApplication sharedApplication] dockTile];
  if (!dockTile)
    return;

  if (niStringIsOK(aaszLabel)) {
    [dockTile setBadgeLabel:_ToNSString(aaszLabel)];
  }
  else {
    [dockTile setBadgeLabel:nil];
  }
}

static tBool _RegisterApp(void)
{
  if (_bAppHasBeenRegistered)
    return eTrue;

  [NIApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

  // Menu bar setup must go between sharedApplication above and
  // finishLaunching below, in order to properly emulate the behavior of
  // NSApplicationMain
  if ([[NSBundle mainBundle] pathForResource:@"MainMenu" ofType:@"nib"]) {
    [NSApp loadMainMenu];
  }
  else {
    _CreateApplicationMenus(_appDelegate);
  }

  [NSApp finishLaunching];

  // Create the app delegate
  _appDelegate = [[NIAppDelegate alloc] init];
  [NSApp setDelegate:_appDelegate];

  _bAppHasBeenRegistered = eTrue;
  return eTrue;
}

static tU32 _PumpEvents()
{
  @autoreleasepool {
    tU32 numEvents = 0;
    for (;;) {
      NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                        untilDate:[NSDate distantPast]
                        inMode:NSDefaultRunLoopMode
                        dequeue:YES];
      if ( event == nil ) {
        break;
      }
      [NSApp sendEvent:event];
      ++numEvents;
    }

    // this stays here commented as future reference if it becomes ever necessary
    //     switch ([event type]) {
    //     case NSKeyDown:
    //     case NSKeyUp:
    //     case NSFlagsChanged:
    //         break;
    //     default:
    //         [NSApp sendEvent:event];
    //         break;
    //     }

    return numEvents;
  }
}

/* This function assumes that it's called from within an autorelease pool */
static NSImage* _CreateNSImage(const tU32* apImgData,
                               const tU32 anWidth,
                               const tU32 anHeight)
{
  const tU32 nPitch = anWidth * sizeof(tU32);

  NSBitmapImageRep* imgrep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes: NULL
                              pixelsWide: anWidth
                              pixelsHigh: anHeight
                              bitsPerSample: 8
                              samplesPerPixel: 4
                              hasAlpha: YES
                              isPlanar: NO
                              colorSpaceName: NSDeviceRGBColorSpace
                              bytesPerRow: nPitch
                              bitsPerPixel: 32];
  if (imgrep == nil) {
    return nil;
  }

  /* Copy the pixels */
  tU8* pixels = [imgrep bitmapData];
  ni::MemCopy((tPtr)pixels, (tPtr)apImgData, nPitch * anHeight);

  /* Premultiply the alpha channel */
  for (int i = (anWidth * anHeight); i--; ) {
    const tU8 alpha = pixels[3];
    pixels[0] = (tU8)(((tU16)pixels[0] * alpha) / 255);
    pixels[1] = (tU8)(((tU16)pixels[1] * alpha) / 255);
    pixels[2] = (tU8)(((tU16)pixels[2] * alpha) / 255);
    pixels += 4;
  }

  NSImage* img = [[NSImage alloc] initWithSize: NSMakeSize(anWidth, anHeight)];
  if (img != nil) {
    [img addRepresentation: imgrep];
  }

  return img;
}

static NSCursor* _CreateNSCursor(const tU32* apImgData,
                                 const tU32 anWidth,
                                 const tU32 anHeight,
                                 const tI32 anHotX,
                                 const tI32 anHotY)
{
  NSImage* nsimage = _CreateNSImage(apImgData, anWidth, anHeight);
  if (nsimage) {
    return [[NSCursor alloc] initWithImage: nsimage hotSpot: NSMakePoint(anHotX, anHotY)];
  }
  return NULL;
}

@interface NSCursor (InvisibleCursor)
+ (NSCursor *)invisibleCursor;
@end

@implementation NSCursor (InvisibleCursor)
+ (NSCursor *)invisibleCursor
{
  static NSCursor *invisibleCursor = NULL;
  if (!invisibleCursor) {
    /* RAW 16x16 transparent GIF */
    static unsigned char cursorBytes[] = {
      0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x10, 0x00, 0x10, 0x00, 0x80,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0xF9, 0x04,
      0x01, 0x00, 0x00, 0x01, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x10,
      0x00, 0x10, 0x00, 0x00, 0x02, 0x0E, 0x8C, 0x8F, 0xA9, 0xCB, 0xED,
      0x0F, 0xA3, 0x9C, 0xB4, 0xDA, 0x8B, 0xB3, 0x3E, 0x05, 0x00, 0x3B
    };

    NSData *cursorData = [NSData dataWithBytesNoCopy:&cursorBytes[0]
                          length:sizeof(cursorBytes)
                          freeWhenDone:NO];
    NSImage *cursorImage = [[NSImage alloc] initWithData:cursorData];
    invisibleCursor = [[NSCursor alloc] initWithImage:cursorImage
                       hotSpot:NSZeroPoint];
  }

  return invisibleCursor;
}
@end

///--------------------------------------------------------------------------------------------
///
/// NSTranslator implementation (IME)
///
///--------------------------------------------------------------------------------------------
#define DEBUG_IME NSLog
// #define DEBUG_IME(...)

#ifndef NX_DEVICERCTLKEYMASK
#define NX_DEVICELCTLKEYMASK    0x00000001
#endif
#ifndef NX_DEVICELSHIFTKEYMASK
#define NX_DEVICELSHIFTKEYMASK  0x00000002
#endif
#ifndef NX_DEVICERSHIFTKEYMASK
#define NX_DEVICERSHIFTKEYMASK  0x00000004
#endif
#ifndef NX_DEVICELCMDKEYMASK
#define NX_DEVICELCMDKEYMASK    0x00000008
#endif
#ifndef NX_DEVICERCMDKEYMASK
#define NX_DEVICERCMDKEYMASK    0x00000010
#endif
#ifndef NX_DEVICELALTKEYMASK
#define NX_DEVICELALTKEYMASK    0x00000020
#endif
#ifndef NX_DEVICERALTKEYMASK
#define NX_DEVICERALTKEYMASK    0x00000040
#endif
#ifndef NX_DEVICERCTLKEYMASK
#define NX_DEVICERCTLKEYMASK    0x00002000
#endif

@interface NiNSTranslatorResponder : NSView <NSTextInput>
{
  cOSXWindow* _wnd;
  NSString*   _markedText;
  NSRange     _markedRange;
  NSRange     _selectedRange;
  sRecti _inputRect;
}
- (void) doCommandBySelector:(SEL)myselector;
- (void) setInputRect:(const sRecti *) rect;
@end

@implementation NiNSTranslatorResponder

- (void) setInputRect:(const sRecti *) rect
{
  _inputRect = *rect;
}

- (void) insertText:(id) aString
{
  const char *str;

  DEBUG_IME(@"insertText: %@", aString);

  /* Could be NSString or NSAttributedString, so we have
   * to test and convert it before return as SDL event */
  if ([aString isKindOfClass: [NSAttributedString class]])
    str = [[aString string] UTF8String];
  else
    str = [aString UTF8String];

  _SendWindowTextInput(_wnd, str);
}

- (void) doCommandBySelector:(SEL) myselector
{
  /* No need to do anything since we are not using Cocoa
     selectors to handle special keys, instead we use SDL
     key events to do the same job.
  */
}

- (BOOL) hasMarkedText
{
  return _markedText != nil;
}

- (NSRange) markedRange
{
  return _markedRange;
}

- (NSRange) selectedRange
{
  return _selectedRange;
}

- (void) setMarkedText:(id) aString
selectedRange:(NSRange) selRange
{
  if ([aString isKindOfClass: [NSAttributedString class]])
    aString = [aString string];

  if ([aString length] == 0)
  {
    [self unmarkText];
    return;
  }

  if (_markedText != aString) {
    _markedText = aString;
  }

  _selectedRange = selRange;
  _markedRange = NSMakeRange(0, [aString length]);

  _SendWindowEditingText(_wnd, [aString UTF8String], selRange.location, selRange.length);

  DEBUG_IME(@"setMarkedText: %@, (%d, %d)",
            _markedText,
            (int)selRange.location, (int)selRange.length);
}

- (void) unmarkText {
  _markedText = nil;
  _SendWindowEditingText(_wnd, "", 0, 0);
}

- (NSRect) firstRectForCharacterRange: (NSRange) theRange
{
  NSWindow *window = [self window];
  NSRect contentRect = [window contentRectForFrameRect: [window frame]];
  float windowHeight = contentRect.size.height;
  NSRect rect = NSMakeRect(_inputRect.x, windowHeight - _inputRect.y - _inputRect.GetHeight(),
                           _inputRect.GetWidth(), _inputRect.GetHeight());

  DEBUG_IME(@"firstRectForCharacterRange: (%d, %d): windowHeight = %g, rect = %@",
            (int)theRange.location, (int)theRange.length,
            windowHeight,
            NSStringFromRect(rect));
  rect.origin = [[self window] convertBaseToScreen: rect.origin];

  return rect;
}

- (NSAttributedString *) attributedSubstringFromRange: (NSRange) theRange
{
  DEBUG_IME(@"attributedSubstringFromRange: (%d, %d)",
            (int)theRange.location, (int)theRange.length);
  return nil;
}

- (NSInteger) conversationIdentifier
{
  return (NSInteger) self;
}

/* This method returns the index for character that is
 * nearest to thePoint.  thPoint is in screen coordinate system.
 */
- (NSUInteger) characterIndexForPoint:(NSPoint) thePoint
{
  DEBUG_IME(@"characterIndexForPoint: (%g, %g)", thePoint.x, thePoint.y);
  return 0;
}

/* This method is the key to attribute extension.
 * We could add new attributes through this method.
 * NSInputServer examines the return value of this
 * method & constructs appropriate attributed string.
 */
- (NSArray *) validAttributesForMarkedText
{
  return [NSArray array];
}

@end

///--------------------------------------------------------------------------------------------
///
/// cOSXWindow implementation
///
///--------------------------------------------------------------------------------------------
class cOSXWindow : public ni::cIUnknownImpl<ni::iOSWindow,ni::eIUnknownImplFlags_Default,ni::iOSWindowOSX> {
  niBeginClass(cOSXWindow);

 public:
  ///////////////////////////////////////////////
  cOSXWindow(iOSWindow* apParent)
      : mptrParentWindow(apParent)
  {
    mCursor = eOSCursor_Arrow;
    mbRequestedClose = eFalse;
    mbMouseOverClient = eFalse;
    mbCursorCapture = eFalse;
    mnEatRelativeMouseMove = 0;
    mbDropTarget = eFalse;
    mfRefreshTimer = -1;
    nsWindow = nil;
    nsResponder = nil;
    nsOwned = eFalse;
    nsModifierFlags = 0;
    nsIsFullscreen = eFalse;

    mbIsActive = eFalse;

    mptrMT = tMessageHandlerSinkLst::Create();
    SetCursor(eOSCursor_Arrow);
  }

  ///////////////////////////////////////////////
  ~cOSXWindow() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    osxglDestroyContext(this);
    mbRequestedClose = eTrue;
    _ReleaseWindow();
    mptrParentWindow = NULL;
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const niImpl {
    niClassIsOK(cOSXWindow);
    return nsWindow != nil;
  }

  ///////////////////////////////////////////////
  iOSWindow* __stdcall GetParent() const niImpl {
    return mptrParentWindow;
  }
  tIntPtr __stdcall GetPID() const niImpl {
    return (tIntPtr)0;
  }

  ///////////////////////////////////////////////
  void __stdcall SetClientAreaWindow(tIntPtr aHandle) niImpl {
  }
  tIntPtr __stdcall GetClientAreaWindow() const niImpl {
    return 0;
  }

  ///////////////////////////////////////////////
  virtual tIntPtr __stdcall GetHandle() const niImpl {
    return (tIntPtr)nsWindow;
  }
  virtual tBool __stdcall GetIsHandleOwned() const niImpl {
    return nsOwned;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall ActivateWindow() niImpl {
    [nsWindow makeKeyAndOrderFront:nil];
  }
  virtual tBool __stdcall SwitchIn(tU32 anReason) niImpl {
    TRACE_OSX(("... OSX: switchIn"));
    _SendMessage(eOSWindowMessage_SwitchIn,anReason);
    mbIsActive = eTrue;
    return eTrue;
  }
  virtual tBool __stdcall SwitchOut(tU32 anReason) niImpl {
    TRACE_OSX(("... OSX: switchOut"));
    _SendMessage(eOSWindowMessage_SwitchOut,anReason);
    mbIsActive = eFalse;
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetIsActive() const niImpl {
    return mbIsActive;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetTitle(const achar* aaszTitle) niImpl {
    mstrTitle = aaszTitle;
    NSString* str = _ToNSString(mstrTitle.Chars());
    [nsWindow setTitle:str];

    if (_bSetDockTileLabelFromWindowTitle) {
      if (mstrTitle.contains(" - ")) {
        cString badge = mstrTitle.Before(" - ");
        badge.Trim();
        osxSetDockTileLabel(badge.Chars());
      }
      else if (mstrTitle.contains(": ")) {
        cString badge = mstrTitle.Before(": ");
        badge.Trim();
        osxSetDockTileLabel(badge.Chars());
      }
      else if (mstrTitle.contains(" (")) {
        cString badge = mstrTitle.Before(" (");
        badge.Trim();
        osxSetDockTileLabel(badge.Chars());
      }
      else {
        osxSetDockTileLabel(mstrTitle.Chars());
      }
    }
  }
  virtual const achar* __stdcall GetTitle() const niImpl {
    return mstrTitle.Chars();
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetStyle(tOSWindowStyleFlags aStyle) niImpl {
  }
  virtual tOSWindowStyleFlags __stdcall GetStyle() const niImpl {
    return 0;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetShow(tOSWindowShowFlags aShow) niImpl {
    // Needed here so that "[NSApp activateIgnoringOtherApps:YES]" below works
    // as expected. That is so that we have a functioning menu bar when an app
    // starts from a terminal.
    _PumpEvents();

    if (aShow & eOSWindowShowFlags_Hide) {
      [nsWindow orderOut:nil];
    }
    else if (aShow & eOSWindowShowFlags_Restore) {
      if ([nsWindow isMiniaturized]) {
        [nsWindow deminiaturize:nil];
      } else if ([nsWindow isZoomed]) {
        [nsWindow zoom:nil];
      }
    }
    else {
      if (aShow & eOSWindowShowFlags_Maximize) {
        [nsWindow zoom:nil];
      }
      else if (aShow & eOSWindowShowFlags_Minimize) {
        [nsWindow miniaturize:nil];
      }
      if (niFlagIs(aShow,eOSWindowShowFlags_Show)) {
        /* makeKeyAndOrderFront: has the side-effect of deminiaturizing and showing
           a minimized or hidden window, so check for that before showing it.
        */
        if (![nsWindow isMiniaturized]) {
          [NSApp activateIgnoringOtherApps:YES];
          [nsWindow makeKeyAndOrderFront:nil];
        }
      }
    }
  }
  virtual tOSWindowShowFlags __stdcall GetShow() const niImpl {
    return 0;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetZOrder(eOSWindowZOrder aZOrder) niImpl {
  }
  virtual eOSWindowZOrder __stdcall GetZOrder() const niImpl {
    return eOSWindowZOrder_Normal;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetSize(const sVec2i& avSize) niImpl {
    sRecti rect = GetRect();
    rect.SetSize(avSize);
    SetRect(rect);
  }
  virtual sVec2i __stdcall GetSize() const niImpl {
    return GetRect().GetSize();
  }
  virtual void __stdcall SetPosition(const sVec2i& avPos) niImpl {
    sRecti rect = GetRect();
    rect.MoveTo(avPos);
    SetRect(rect);
  }
  virtual sVec2i __stdcall GetPosition() const niImpl {
    return GetRect().GetTopLeft();
  }
  virtual void __stdcall SetRect(const sRecti& aRect) niImpl {
    NSRect rect;
    rect.origin.x = aRect.x;
    rect.origin.y = aRect.y;
    rect.size.width = aRect.GetWidth();
    rect.size.height = aRect.GetHeight();
    _FlipNSRect(this,&rect);
    rect = [nsWindow frameRectForContentRect:rect];
    [nsWindow setFrameOrigin:rect.origin];
    [nsWindow setContentSize:rect.size];
  }
  virtual sRecti __stdcall GetRect() const niImpl {
    NSRect rect = [nsWindow contentRectForFrameRect:[nsWindow frame]];
    _FlipNSRect(this,&rect);
    return sRecti(rect.origin.x,rect.origin.y,rect.size.width,rect.size.height);
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetClientSize(const sVec2i& avSize) niImpl {
    return SetSize(avSize);
  }
  virtual sVec2i __stdcall GetClientSize() const niImpl {
    sVec2i sz;
    if (nsView) {
      if (nsViewRect.size.width == 0 || nsViewRect.size.height == 0) {
        niThis(cOSXWindow)->nsViewRect = [nsView bounds];
      }
      sz = Vec2i(nsViewRect.size.width,nsViewRect.size.height);
    }
    else {
      sz = GetSize();
    }
    return sz * GetContentsScale();
  }

  ///////////////////////////////////////////////
  virtual tF32 __stdcall GetContentsScale() const {
    if (nsGL.context && nsGL.useBestResolution) {
      return [nsWindow backingScaleFactor];
    }
    return 1.0f;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall Clear() niImpl {
  }

  ///////////////////////////////////////////////
  virtual tMessageHandlerSinkLst* __stdcall GetMessageHandlers() const niImpl {
    return mptrMT;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall UpdateWindow(tBool abBlockingMessages) niImpl {
    const tU32 numEvents = _PumpEvents();
    if (nsGL.context) {
      // If we have an OpenGL context we send paint right after update.  For
      // Metal we don't do that since MTKView drives the paint message.
      this->_SendMessage(eOSWindowMessage_Paint);
    }
    if (!numEvents) {
      return eFalse;
    }
    else {
      return eTrue;
    }
  }

  virtual tBool __stdcall RedrawWindow() niImpl {
    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall CenterWindow() niImpl {
    //
    // NOTE: This does not put the window "dead center" on the screen
    //
    // See the rationale at: https://developer.apple.com/documentation/appkit/nswindow/1419090-center
    //
    // "The window is placed exactly in the center horizontally and somewhat
    //  above center vertically. Such a placement carries a certain visual
    //  immediacy and importance."
    //
    [nsWindow center];
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetRequestedClose() const niImpl {
    return mbRequestedClose;
  }
  virtual void __stdcall SetRequestedClose(tBool abRequested) niImpl {
    mbRequestedClose = abRequested;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetCursor(eOSCursor aCursor) niImpl {
    if (mCursor == aCursor)
      return;
    mCursor = aCursor;
    _UpdateCursor(mCursor);
  }

  virtual eOSCursor __stdcall GetCursor() const niImpl {
    return mCursor;
  }

  void _UpdateCursor(const eOSCursor aCursor) {
    NSCursor* newCursor = nil;
    switch (aCursor) {
      case eOSCursor_None: {
        newCursor = [NSCursor invisibleCursor];
        break;
      }
      case eOSCursor_Wait: {
        newCursor = [NSCursor arrowCursor];
        break;
      }
      case eOSCursor_Text: {
        newCursor = [NSCursor IBeamCursor];
        break;
      }
      case eOSCursor_ResizeHz: {
        newCursor = [NSCursor resizeLeftRightCursor];
        break;
      }
      case eOSCursor_ResizeVt: {
        newCursor = [NSCursor resizeUpDownCursor];
        break;
      }
      case eOSCursor_Custom:
        if (nsCustomCursor) {
          newCursor = nsCustomCursor;
          break;
        }
      default:
      case eOSCursor_Arrow:
        newCursor = [NSCursor arrowCursor];
        break;
    }
    if (nsCursor == newCursor) {
      return;
    }

    nsCursor = newCursor;
    TRACE_OSX(("... OSX: Setting cursor: %p", (tIntPtr)nsCursor));

    [nsCursor set];
    [nsWindow invalidateCursorRectsForView:nsView];

    if (aCursor == eOSCursor_None) {
      _HideCursor();
    }
    else {
      _ShowCursor();
    }
  }

  virtual tBool __stdcall InitCustomCursor(tIntPtr aID, tU32 anWidth, tU32 anHeight, tU32 anPivotX, tU32 anPivotY, const tU32* apData) niImpl {
    if (aID == mnCustomCursorID)
      return eTrue;

    if (!apData) // we're just checking size support...
      return eTrue;

    nsCustomCursor = _CreateNSCursor(apData,anWidth,anHeight,anPivotX,anPivotY);
    if (!nsCustomCursor) {
      niWarning("Couldn't create custom cursor !");
      return eFalse;
    }

    mnCustomCursorID = aID;
    return eTrue;
  }
  virtual tIntPtr __stdcall GetCustomCursorID() const niImpl {
    return mnCustomCursorID;
  }

  virtual void __stdcall SetCursorPosition(const sVec2i& avCursorPos) niImpl {
    const sRecti rect = this->GetRect();
    const tF32 contentsScale = GetContentsScale();
    const sVec2i pos = {
      rect.x + (avCursorPos.x/contentsScale),
      rect.y + (avCursorPos.y/contentsScale)
    };
    _WrapCursorPosition(pos);
  }
  virtual sVec2i __stdcall GetCursorPosition() const niImpl {
    return mvPrevMousePos;
  }

  virtual void __stdcall SetCursorCapture(tBool abCapture) niImpl {
    if (abCapture == mbCursorCapture)
      return;
    mbCursorCapture = abCapture;
    _SendMouseMove(NULL);
    // We skip the first two relative mouse move after cursor capture so that
    // we dont get a big jump between the current position and the center of
    // the screen position set by the cursor capture mode.
    mnEatRelativeMouseMove = 2;
  }
  virtual tBool __stdcall GetCursorCapture() const niImpl {
    return mbCursorCapture;
  }

  virtual tBool __stdcall GetIsCursorOverClient() const niImpl {
    return mbMouseOverClient;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall TryClose() niImpl {
    if (nsWindow != nil) {
      [nsWindow performClose:nsWindow];
    }
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetDropTarget(tBool abDropTarget) niImpl {
    mbDropTarget = abDropTarget;
  }
  virtual tBool __stdcall GetDropTarget() const niImpl {
    return mbDropTarget;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetMonitor() const niImpl {
    return 0;
  }

  ///////////////////////////////////////////////
  tBool __stdcall SetFullScreen(tU32 anScreenId) niImpl {
    if (anScreenId == eInvalidHandle) {
      // exit full screen
      if (nsIsFullscreen) {
        [nsWindow toggleFullScreen:nil];
      }
    }
    else {
      // enter full screen
      if (!nsIsFullscreen) {
        [nsWindow toggleFullScreen:nil];
      }
    }
    return eFalse;
  }
  tU32 __stdcall GetFullScreen() const niImpl {
    return nsIsFullscreen ? 1 : eInvalidHandle;
  }
  tBool __stdcall GetIsMinimized() const niImpl {
    if (nsWindow) {
      return [nsWindow isMiniaturized];
    }
    else {
      return eFalse;
    }
  }
  tBool __stdcall GetIsMaximized() const niImpl {
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetFocus() niImpl {
  }
  virtual tBool __stdcall GetHasFocus() const niImpl {
    return mbIsActive;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetRefreshTimer(tF32 afRefreshTimer) niImpl {
    // niDebugFmt(("... SetRefreshTimer: %g", afRefreshTimer));
    mfRefreshTimer = afRefreshTimer;
  }
  virtual tF32 __stdcall GetRefreshTimer() const niImpl {
    return mfRefreshTimer;
  }

  ///////////////////////////////////////////////
  virtual tIntPtr __stdcall GetParentHandle() const niImpl {
    return 0;
  }
  virtual tU32 __stdcall IsParentWindow(tIntPtr aHandle) const niImpl {
    return 0;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall AttachGraphicsAPI(iOSGraphicsAPI* apAPI) {
    niCheckIsOK(apAPI,eFalse);
    if (nsGL.context) {
      niError("An OpenGL graphics context is already attached to this window.");
      return eFalse;
    }
    if (mptrAPI.IsOK()) {
      niError("A Metal API context is already attached to this window.");
      return eFalse;
    }
    if (StrIEq(apAPI->GetName(),"metal")) {
      mptrAPI = apAPI;
    }
    else {
      niError(niFmt("Unknown graphics api '%s'.", apAPI->GetName()));
      return eFalse;
    }
    return eTrue;
  }
  virtual iOSGraphicsAPI* __stdcall GetGraphicsAPI() const {
    return mptrAPI;
  }

  ///////////////////////////////////////////////
  virtual void* __stdcall GetNSWindow() const {
    return (__bridge void*)nsWindow;
  }

  ///////////////////////////////////////////////
  tBool _SendMessage(eOSWindowMessage aMsg, const Var& avarA = niVarNull, const Var& avarB = niVarNull) {
    if (nsGL.context) {
      const int cat = niMessageID_GetCharD(aMsg);
      switch (cat) {
        case 'S':
        case 'K':
        case 'I':
        case 'F':
        case 'G':
          nsGL.drawEvent.Signal();
          break;
      }
    }
    return ni::SendMessages(mptrMT,aMsg,avarA,avarB);
  }
  tBool _RetainWindow(NSWindow* apNSWindow, NSView* apNSView, tBool abOwned, tBool abSetWindowProc)
  {
    _ReleaseWindow();
    nsWindow = apNSWindow;
    nsView = apNSView;
    nsOwned = abOwned;
    if (abSetWindowProc) {
      nsResponder = [[NIWindowDelegate alloc] init];
      [nsResponder listen:this];
    }
    _RegisterWindow(this);
    return eTrue;
  }
  tBool _ReleaseWindow() {
    _UnregisterWindow(this);
    nsCursor = nil;
    nsCustomCursor = nil;
    if (nsResponder != nil) {
      [nsResponder close];
      nsResponder = nil;
    }
    nsView = nil;
    if (nsWindow != nil) {
      [nsWindow close];
      nsWindow = nil;
    }
    return eTrue;
  }

  void _SendMouseMove(NSEvent* theEvent) {
    if (theEvent) {
      const sVec2i vRelMove = {
        (tI32)([theEvent deltaX]),
        (tI32)([theEvent deltaY])
      };
      if (vRelMove != sVec2i::Zero()) {
        if (mnEatRelativeMouseMove <= 0) {
          _SendMessage(eOSWindowMessage_RelativeMouseMove,vRelMove);
        }
        else {
          --mnEatRelativeMouseMove;
        }
      }
    }

    const tF32 contentsScale = GetContentsScale();
    const NSRect rect = [nsWindow contentRectForFrameRect:[nsWindow frame]];
    if (mbCursorCapture) {
      SetCursorPosition(
        Vec2i(rect.size.width * contentsScale / 2,
              rect.size.height * contentsScale / 2));
    }
    else if (theEvent) {
      sVec2i vMousePos;
      {
        NSPoint point = [theEvent locationInWindow];
        vMousePos.x = point.x;
        vMousePos.y = rect.size.height - point.y;
      }

      if (vMousePos != mvPrevMousePos) {
        const tBool wasMouseOverClient = mbMouseOverClient;
        if (vMousePos.x < 0 || vMousePos.x >= rect.size.width ||
            vMousePos.y < 0 || vMousePos.y >= rect.size.height)
        {
          mbMouseOverClient = eFalse;
        }
        else {
          mbMouseOverClient = eTrue;
        }

        if (mbMouseOverClient != wasMouseOverClient) {
          if (mbMouseOverClient) {
            // enter
            _UpdateCursor(mCursor);
            TRACE_OSX(("... OSX: MouseEnter"));
          }
          else {
            // leave
            _UpdateCursor(eOSCursor_Arrow);
            TRACE_OSX(("... OSX: MouseLeave"));
          }
        }
        else {
          _UpdateCursor(mbMouseOverClient ? mCursor : eOSCursor_Arrow);
        }

        _SendMessage(eOSWindowMessage_MouseMove,vMousePos * GetContentsScale());
        mvPrevMousePos = vMousePos;
        // TRACE_OSX(("... OSX: MouseMoved: abs: %s, rel: %s.", vMousePos, vRelMove));
      }
    }
  }

 public:
  ni::Ptr<iOSWindow>          mptrParentWindow;
  cString                     mstrTitle;
  ni::Ptr<tMessageHandlerSinkLst> mptrMT;
  tBool                       mbRequestedClose;
  tBool                       mbCursorCapture;
  tI32                        mnEatRelativeMouseMove;
  tBool                       mbMouseOverClient;
  tBool                       mbDropTarget;
  tBool                       mbIsActive;
  tF32                        mfRefreshTimer;
  sVec2i                      mvPrevMousePos = ni::Vec2<tI32>(eInvalidHandle,eInvalidHandle);

  eOSCursor        mCursor;
  NSCursor*        nsCursor;
  tIntPtr          mnCustomCursorID;
  NSCursor*        nsCustomCursor;
  NSWindow*        nsWindow;
  NSView*          nsView;
  NSRect           nsViewRect;
  NIWindowDelegate*   nsResponder;
  tBool            nsOwned;
  tU32             nsModifierFlags;
  tBool            nsIsFullscreen;

  struct _GLContext {
    ThreadEvent vsyncEvent;
    SyncCounter vsyncCount;
    ThreadEvent drawEvent;
    id pixelFormat;
    id context;
    CVDisplayLinkRef displayLink;
    int swapInterval;
    tBool useBestResolution;
  } nsGL;

  ni::Ptr<iOSGraphicsAPI> mptrAPI;

  niEndClass(cOSXWindow);
};

///--------------------------------------------------------------------------------------------
///
/// Content View implementation
///
///--------------------------------------------------------------------------------------------

// Returns the backing rect of the specified window
//
static NSRect convertRectToBacking(cOSXWindow* window, NSRect contentRect)
{
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
  if (floor(NSAppKitVersionNumber) >= NSAppKitVersionNumber10_7)
    return [window->nsView convertRectToBacking:contentRect];
  else
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/
    return contentRect;
}

@interface NIContentView : NSView
{
  cOSXWindow* _wnd;
}
- (id)initWithOSXWindow:(cOSXWindow*)initWindow;
@end

@implementation NIContentView

+ (void)initialize
{
  if (self == [NIContentView class]) {
  }
}

- (id)initWithOSXWindow:(cOSXWindow*)initWindow
{
  self = [super init];
  if (self != nil) {
    _wnd = initWindow;
  }

  return self;
}

- (BOOL)isOpaque
{
  return YES;
}

- (BOOL)canBecomeKeyView
{
  return YES;
}

- (BOOL)acceptsFirstResponder
{
  return YES;
}

///////////////////////////////////////////////
-(void) mouseDown:(NSEvent *) theEvent {
  if (!_wnd) return;
  _wnd->_SendMouseMove(theEvent);

  int eventBt = [theEvent buttonNumber];
  ni::tU32 bt;
  switch (eventBt) {
    case 0: bt = ePointerButton_Left; break;
    case 1: bt = ePointerButton_Right; break;
    case 2: bt = ePointerButton_Middle; break;
    default: bt = ePointerButton_Bt0+eventBt; break;
  }
  _wnd->_SendMessage(eOSWindowMessage_MouseButtonDown, (ni::tU32)bt);
}
-(void) rightMouseDown:(NSEvent *) theEvent {
  if (!_wnd) return;
  [self mouseDown:theEvent];
}
-(void) otherMouseDown:(NSEvent *) theEvent {
  if (!_wnd) return;
  [self mouseDown:theEvent];
}

///////////////////////////////////////////////
-(void) mouseUp:(NSEvent *) theEvent {
  if (!_wnd) return;
  _wnd->_SendMouseMove(theEvent);

  int eventBt = [theEvent buttonNumber];
  ni::tU32 bt;
  switch (eventBt) {
    case 0: bt = ePointerButton_Left; break;
    case 1: bt = ePointerButton_Right; break;
    case 2: bt = ePointerButton_Middle; break;
    default: bt = ePointerButton_Bt0+eventBt; break;
  }
  _wnd->_SendMessage(eOSWindowMessage_MouseButtonUp, (ni::tU32)bt);

  const NSInteger clickCount = [theEvent clickCount];
  if (clickCount == 2) {
    _wnd->_SendMessage(eOSWindowMessage_MouseButtonDoubleClick,
                       (ni::tU32)(ePointerButton_Bt0+eventBt));
  }
}
-(void) rightMouseUp:(NSEvent *) theEvent {
  if (!_wnd) return;
  [self mouseUp:theEvent];
}
-(void) otherMouseUp:(NSEvent *) theEvent {
  if (!_wnd) return;
  [self mouseUp:theEvent];
}

///////////////////////////////////////////////
-(void) mouseMoved:(NSEvent *) theEvent {
  if (!_wnd  || !_wnd->mbIsActive) return;
  _wnd->_SendMouseMove(theEvent);
}
-(void) mouseDragged:(NSEvent *) theEvent {
  if (!_wnd) return;
  [self mouseMoved:theEvent];
}
-(void) rightMouseDragged:(NSEvent *) theEvent {
  if (!_wnd) return;
  [self mouseMoved:theEvent];
}
-(void) otherMouseDragged:(NSEvent *) theEvent {
  if (!_wnd) return;
  [self mouseMoved:theEvent];
}

///////////////////////////////////////////////
- (void)scrollWheel:(NSEvent *)event
{
  if (!_wnd  || !_wnd->mbIsActive) return;

  double deltaX, deltaY;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
  if (floor(NSAppKitVersionNumber) >= NSAppKitVersionNumber10_7)
  {
    deltaX = [event scrollingDeltaX];
    deltaY = [event scrollingDeltaY];

    if ([event hasPreciseScrollingDeltas])
    {
      deltaX *= 0.1;
      deltaY *= 0.1;
    }
  }
  else
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/
  {
    deltaX = [event deltaX];
    deltaY = [event deltaY];
  }

  if (fabs(deltaX) > 0.0 || fabs(deltaY) > 0.0) {
    TRACE_OSX(("... OSX: SCROLL WHEEL: %g, %g",deltaY,deltaX));
    _wnd->_SendMessage(eOSWindowMessage_MouseWheel,deltaY,deltaX);
  }
}

///////////////////////////////////////////////
- (void)magnifyWithEvent:(NSEvent *)event {
  const float mag = [event magnification];
  TRACE_OSX(("... OSX: MAGNIFY: %g",mag));
  niUnused(mag);
}

///////////////////////////////////////////////
- (void)rotateWithEvent:(NSEvent *)event {
  const float rot = [event rotation];
  TRACE_OSX(("... OSX: ROTATE: %g",rot));
  niUnused(rot);
}

///////////////////////////////////////////////
- (void)swipeWithEvent:(NSEvent *)event {
  CGFloat x = [event deltaX];
  CGFloat y = [event deltaY];
  TRACE_OSX(("... OSX: SWIPE: %g, %g",x,y));
  niUnused(x);
  niUnused(y);
}

///////////////////////////////////////////////
- (void)_SendKey:(eKey)aKey isDown:(tBool)abIsDown {
  _wnd->_SendMessage(
    abIsDown ? eOSWindowMessage_KeyDown : eOSWindowMessage_KeyUp,
    (tU32)aKey,
    niVarNull);
}

///////////////////////////////////////////////
-(void) keyDown:(NSEvent *) theEvent {
  if (!_wnd  || !_wnd->mbIsActive) return;

  const tU32 macKeyCode = [theEvent keyCode];
  const sOSXScanCode& scanCode = (macKeyCode < niCountOf(_ScancodeTable)) ?
      _ScancodeTable[macKeyCode] : _ScancodeUnknown;

  if (scanCode.niKey != eKey_Unknown) {
    [self _SendKey:scanCode.niKey isDown:eTrue];
  }

  if (scanCode.canPrint) {
    // Send after KeyDown - to mimic Windows' behavior
    NSString* characters = [theEvent characters];
    NSUInteger length = [characters length];
    niLoop(i,length) {
      const ni::tU32 c = [characters characterAtIndex:i];
      TRACE_OSX(("... OSX: KEY CHAR: %d (%c)",c,c));
      _wnd->_SendMessage(eOSWindowMessage_KeyChar,c,scanCode.niKey);
    }
  }

  TRACE_OSX(("... OSX: KEY DOWN: %s, Mac:0x%02X, eKey: 0x%02X = %s",
             [[theEvent characters] UTF8String],
             macKeyCode,scanCode.niKey,
             niEnumToChars(eKey,scanCode.niKey)));
}
-(void) keyUp:(NSEvent *) theEvent {
  if (!_wnd  || !_wnd->mbIsActive) return;

  const tU32 macKeyCode = [theEvent keyCode];
  const sOSXScanCode& scanCode = (macKeyCode < niCountOf(_ScancodeTable)) ?
      _ScancodeTable[macKeyCode] : _ScancodeUnknown;

  if (scanCode.niKey != eKey_Unknown) {
    [self _SendKey:scanCode.niKey isDown:eFalse];
  }

  TRACE_OSX(("... OSX: KEY UP: %s, Mac:0x%02X, eKey: 0x%02X = %s",
             [[theEvent characters] UTF8String],
             macKeyCode,scanCode.niKey,
             niEnumToChars(eKey,scanCode.niKey)));
}
-(void) autoKey:(NSEvent *) theEvent {
  if (!_wnd  || !_wnd->mbIsActive) return;
  TRACE_OSX(("... OSX: AUTOKEY: %s",[[theEvent characters] UTF8String]));
}
-(void) flagsChanged:(NSEvent *) theEvent {
  if (!_wnd) return;

  TRACE_OSX(("... OSX: FLAGS CHANGED"));

  unsigned int flags = [theEvent modifierFlags];
  if (flags == _wnd->nsModifierFlags) {
    return;
  }

#define DIFFPREV(F) ((isDown = niFlagIs(flags,F)), (niFlagIs(flags,F) != niFlagIs(_wnd->nsModifierFlags,F)))

  tBool isDown = eFalse;
  if (DIFFPREV(NSAlphaShiftKeyMask)) {
    [self _SendKey:eKey_CapsLock isDown:isDown];
  }
  if (DIFFPREV(NSShiftKeyMask)) {
    [self _SendKey:eKey_LShift isDown:isDown];
  }
  if (DIFFPREV(NSControlKeyMask)) {
    [self _SendKey:eKey_LControl isDown:isDown];
  }
  if (DIFFPREV(NSCommandKeyMask)) {
    [self _SendKey:eKey_LWin isDown:isDown];
  }
  if (DIFFPREV(NSAlternateKeyMask)) {
    [self _SendKey:eKey_LAlt isDown:isDown];
  }
  if (DIFFPREV(NSNumericPadKeyMask)) {
    [self _SendKey:eKey_NumLock isDown:isDown];
  }
  if (DIFFPREV(NSFunctionKeyMask)) {
    // [self _SendKey:eKey_Fn isDown:isDown];
  }

#undef DIFFPREV

  _wnd->nsModifierFlags = flags;
}

///////////////////////////////////////////////
- (void)resetCursorRects
{
  if (!_wnd) return;
  TRACE_OSX(("... OSX: resetCursorRects"));
  [self discardCursorRects];
  [self addCursorRect:[self bounds] cursor:_wnd->nsCursor];
}

#ifdef HANDLE_TOUCH_EVENTS
///////////////////////////////////////////////
const tI32 MAX_TOUCHES = 10;
static ni::sVec2f  _lastTouchPosition[MAX_TOUCHES] = {0};
static ni::tIntPtr _touches[MAX_TOUCHES] = {0};

static tI32 _GetFingerIdFromTouch(tIntPtr touch) {
  for (tI32 i = 0; i < MAX_TOUCHES; ++i) {
    if (_touches[i] == touch) {
      return i;
    }
  }
  return -1;
}
static tI32 _AddNewTouch(tIntPtr touch) {
  for (tI32 i = 0; i < MAX_TOUCHES; ++i) {
    if (!_touches[i]) {
      _touches[i] = touch;
      return i;
    }
  }
  return -1;
}
static tI32 _RemoveTouch(const int fingerId) {
  niAssert(fingerId < MAX_TOUCHES);
  _touches[fingerId] = 0;
  return -1;
}
static tI32 _CountNumTouches() {
  int count = 0;
  for (int i = 0; i < MAX_TOUCHES; ++i) {
    if (_touches[i]) {
      ++count;
    }
  }
  return count;
}
const ni::sVec2f _GetTouchLocation(NSTouch* touch, NSView* view) {
#if 0
  NSPoint locationInView = [touch locationInView: view];
  ni::sVec2f r = {
    (float)locationInView.x * 1,
    (float)locationInView.y * 1
  };
#else
  ni::sVec2f r = {
    [touch normalizedPosition].x,
    1.0f - [touch normalizedPosition].y
  };
#endif
  return r;
}

- (void)touchesBeganWithEvent:(NSEvent *) theEvent
{
  NSSet *touches = [theEvent touchesMatchingPhase:NSTouchPhaseAny inView:nil];
  int existingTouchCount = 0;

  for (NSTouch* touch in touches) {
    if ([touch phase] != NSTouchPhaseBegan) {
      existingTouchCount++;
    }
  }
  if (existingTouchCount == 0) {
    // const tIntPtr touchDeviceID = (tIntPtr)[[touches anyObject] device];
    tI32 numFingers = _CountNumTouches();
    TRACE_OSX(("... OSX: Reset Lost Fingers: %d", numFingers));
    for (--numFingers; numFingers >= 0; --numFingers) {
      if (_wnd) {
        _wnd->_SendMessage(eOSWindowMessage_FingerUp,numFingers,Vec3f(0,0,0));
      }
      _RemoveTouch(numFingers);
    }
  }

  TRACE_OSX(("... OSX: Began Fingers: %lu .. existing: %d", (unsigned long)[touches count], existingTouchCount));
  [self handleTouches:NSTouchPhaseBegan withEvent:theEvent];
}

- (void)touchesMovedWithEvent:(NSEvent *) theEvent
{
  [self handleTouches:NSTouchPhaseMoved withEvent:theEvent];
}

- (void)touchesEndedWithEvent:(NSEvent *) theEvent
{
  [self handleTouches:NSTouchPhaseEnded withEvent:theEvent];
}

- (void)touchesCancelledWithEvent:(NSEvent *) theEvent
{
  [self handleTouches:NSTouchPhaseCancelled withEvent:theEvent];
}

- (void)handleTouches:(NSTouchPhase) phase withEvent:(NSEvent *) theEvent
{
  NSSet *touches = [theEvent touchesMatchingPhase:phase inView:nil];

  for (NSTouch *touch in touches) {
    // const tIntPtr touchDeviceID = (tIntPtr)[touch device];
    const tIntPtr touchIdentity = (tIntPtr)[touch identity];

    // PIERRE: This returns normalized coordinates which we can't use
    // directly. The only thing we'd want this for is to recognized things
    // like "3 fingers pan".
    const sVec2f touchLocation = _GetTouchLocation(touch,self);

    switch (phase) {
      case NSTouchPhaseBegan: {
        const tI32 fingerIndex = _AddNewTouch(touchIdentity);
        if (fingerIndex >= 0) {
          _lastTouchPosition[fingerIndex] = touchLocation;
          const sVec3f v = Vec3f(touchLocation.x,touchLocation.y,1);
          TRACE_OSX(("... OSX: FingerDown: %d, %s", fingerIndex, v));
          // if (_wnd) {
          // _wnd->_SendMessage(eOSWindowMessage_FingerDown,fingerIndex,v);
          // }
        }
        break;
      }
      case NSTouchPhaseEnded:
      case NSTouchPhaseCancelled: {
        const tI32 fingerIndex = _GetFingerIdFromTouch(touchIdentity);
        if (fingerIndex >= 0) {
          _lastTouchPosition[fingerIndex] = touchLocation;
          const sVec3f v = Vec3f(touchLocation.x,touchLocation.y,0);
          TRACE_OSX(("... OSX: FingerUp: %d, %s", fingerIndex, v));
          // if (_wnd) {
          // _wnd->_SendMessage(eOSWindowMessage_FingerUp,fingerIndex,v);
          // }
          _RemoveTouch(fingerIndex);
        }
        break;
      }
      case NSTouchPhaseMoved: {
        const tI32 fingerIndex = _GetFingerIdFromTouch(touchIdentity);
        if (fingerIndex >= 0) {
          _lastTouchPosition[fingerIndex] = touchLocation;
          const sVec3f v = Vec3f(touchLocation.x,touchLocation.y,1);
          TRACE_OSX(("... OSX: FingerMove: %d, %s", fingerIndex, v));
          // if (_wnd) {
          // _wnd->_SendMessage(eOSWindowMessage_FingerMove,fingerIndex,v);
          // }
        }
        break;
      }
      default: {
        break;
      }
    }
  }
}
#endif
@end

///--------------------------------------------------------------------------------------------
///
/// OpenGL implementation
///
///--------------------------------------------------------------------------------------------
static struct sOSXGL {
  // dlopen handle for dynamically loading OpenGL extension entry points
  void*           framework;
  // TLS key for per-thread current context/window
  pthread_key_t   current;
  sOSXGL() {
    framework = NULL;
  }
} _osxgl;

#define OSXGL_WINDOW()                                          \
  cOSXWindow* window = reinterpret_cast<cOSXWindow*>(apWindow);

static CVReturn _osxglDisplayLinkCallback(CVDisplayLinkRef displayLink,
                                          const CVTimeStamp* now,
                                          const CVTimeStamp* outputTime,
                                          CVOptionFlags flagsIn,
                                          CVOptionFlags* flagsOut,
                                          void* apWindow)
{
  OSXGL_WINDOW();
  if (window->nsGL.swapInterval > 0 &&
      window->nsGL.vsyncCount.Inc() >= window->nsGL.swapInterval)
  {
    window->nsGL.vsyncCount.Set(0);
    // Signal the vsyncEvent
    window->nsGL.vsyncEvent.Signal();
  }
  return kCVReturnSuccess;
}

niExportFunc(tBool) osxglIsStarted() {
  return _osxgl.framework != NULL;
}

niExportFunc(tBool) osxglStartup(void)
{
  if (_osxgl.framework) {
    return eTrue;
  }

  if (pthread_key_create(&_osxgl.current, NULL) != 0) {
    niError("OSXGL: Failed to create context TLS");
    return eFalse;
  }

  if (!_osxgl.framework) {
    _osxgl.framework = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));
    if (_osxgl.framework == NULL) {
      niError("OSXGL: Failed to locate OpenGL framework");
      return eFalse;
    }
  }

  return eTrue;
}

niExportFunc(void) osxglShutdown(void) {
  if (_osxgl.framework) {
    pthread_key_delete(_osxgl.current);
    _osxgl.framework = NULL;
  }
}

struct sGLRenderer {
  int rendererId;
  int displayMask;
  int vidMem;
  int texMem;
};
static astl::vector<sGLRenderer> _vGLRenderers;

static const char* _kPropertyOSXGLDeviceIndex = "OSXGL.DeviceIndex";
static const char* _kPropertyOSXGLBestResolution = "OSXGL.BestResolution";

niExportFunc(tBool) osxglCreateContext(iOSWindow* apWindow, sOSXGLConfig* nsglConfig)
{
  OSXGL_WINDOW();

  unsigned int attributeCount = 0;

  // OS X needs non-zero color size, so set resonable values
  const int colorBits = (nsglConfig->colorBits == 0) ? 24 : nsglConfig->colorBits;

#define ADD_ATTR(x) { attributes[attributeCount++] = x; }
#define ADD_ATTR2(x, y) { ADD_ATTR(x); ADD_ATTR(y); }

  // Arbitrary array size here
  NSOpenGLPixelFormatAttribute attributes[40];

  ADD_ATTR(NSOpenGLPFADoubleBuffer);
  ADD_ATTR(NSOpenGLPFAClosestPolicy);

  if (_vGLRenderers.empty()) {
    int rendererCount = 0;
    CGLRendererInfoObj info;
    CGLQueryRendererInfo(0xFFFFFFFF, &info, &rendererCount);
    niDebugFmt(("... OSXGL Renderer: count: %d", rendererCount));
    for (int i = 0; i < rendererCount; ++i) {
      int rendererId = 0;
      CGLDescribeRenderer(info, i, kCGLRPRendererID, &rendererId);
      int displayMask = 0;
      CGLDescribeRenderer(info, i, kCGLRPDisplayMask, &displayMask);

      int vidMem = -1, texMem = -1;
      CGLDescribeRenderer(info, i, kCGLRPVideoMemoryMegabytes, &vidMem);
      CGLDescribeRenderer(info, i, kCGLRPTextureMemoryMegabytes, &texMem);

      niDebugFmt(("... OSXGL Renderer[%d]: id: %p, displayMask: %x, vidMem: %d, texMem: %d",
                  i, rendererId, displayMask, vidMem, texMem));

      sGLRenderer r = {
        rendererId,
        displayMask,
        vidMem,
        texMem
      };
      _vGLRenderers.push_back(r);
    }
  }

  int deviceIndex = ni::GetProperty(_kPropertyOSXGLDeviceIndex,"-1").Long();
  if (deviceIndex >= 0) {
    if (deviceIndex >= _vGLRenderers.size()) {
      niError(niFmt("OSXGL: Invalid OpenGL device index '%d', only '%d' available. Using default renderer.", deviceIndex, _vGLRenderers.size()));
    }
    else {
      const sGLRenderer& r = _vGLRenderers[deviceIndex];
      ADD_ATTR2(NSOpenGLPFARendererID, r.rendererId);
      ADD_ATTR2(NSOpenGLPFAScreenMask, r.displayMask);
      niDebugFmt(("... OSXGL Using Renderer[%d]: id: %p, displayMask: %x, vidMem: %d, texMem: %d",
                  deviceIndex, r.rendererId, r.displayMask, r.vidMem, r.texMem));
    }
  }
  else {
    niDebugFmt(("... OSXGL Using default renderer"));
  }

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
  if (nsglConfig->glMajor > 2)
    ADD_ATTR2(NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core);
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/

  ADD_ATTR2(NSOpenGLPFAColorSize, colorBits);

  if (nsglConfig->alphaBits > 0)
    ADD_ATTR2(NSOpenGLPFAAlphaSize, nsglConfig->alphaBits);

  if (nsglConfig->depthBits > 0)
    ADD_ATTR2(NSOpenGLPFADepthSize, nsglConfig->depthBits);

  if (nsglConfig->stencilBits > 0)
    ADD_ATTR2(NSOpenGLPFAStencilSize, nsglConfig->stencilBits);

  const int accumBits = nsglConfig->accumBits;
  if (accumBits > 0)
    ADD_ATTR2(NSOpenGLPFAAccumSize, accumBits);

  if (nsglConfig->auxBuffers > 0)
    ADD_ATTR2(NSOpenGLPFAAuxBuffers, nsglConfig->auxBuffers);

  if (nsglConfig->stereo)
    ADD_ATTR(NSOpenGLPFAStereo);

  if (nsglConfig->aaSamples > 0)
  {
    ADD_ATTR2(NSOpenGLPFASampleBuffers, 1);
    ADD_ATTR2(NSOpenGLPFASamples, nsglConfig->aaSamples);
  }

  // NOTE: All NSOpenGLPixelFormats on the relevant cards support sRGB
  // frambuffer, so there's no need (and no way) to request it

  ADD_ATTR(0);

#undef ADD_ATTR
#undef ADD_ATTR2

  window->nsGL.pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
  if (window->nsGL.pixelFormat == nil) {
    niError("OSXGL: Failed to create OpenGL pixel format");
    return eFalse;
  }

  NSOpenGLContext* share = NULL;

  if (nsglConfig->share)
    share = ((cOSXWindow*)nsglConfig->share)->nsGL.context;

  window->nsGL.context =
      [[NSOpenGLContext alloc] initWithFormat:window->nsGL.pixelFormat
       shareContext:share];
  if (window->nsGL.context == nil)
  {
    niError("OSXGL: Failed to create OpenGL context");
    return eFalse;
  }

  window->nsGL.useBestResolution = ni::GetProperty(_kPropertyOSXGLBestResolution, "1").Bool();
  if (window->nsGL.useBestResolution) {
    [window->nsView setWantsBestResolutionOpenGLSurface:YES];
  }
  niDebugFmt(("... OSXGL use best resolution: %d contentScale: %s",
              window->nsGL.useBestResolution,
              window->GetContentsScale()));

  [window->nsGL.context setView:window->nsView];

  window->nsGL.swapInterval = nsglConfig->swapInterval;
  CVDisplayLinkCreateWithActiveCGDisplays(&window->nsGL.displayLink);
  CVDisplayLinkSetOutputCallback(window->nsGL.displayLink,
                                 &_osxglDisplayLinkCallback,
                                 window);
  CVDisplayLinkStart(window->nsGL.displayLink);

  osxglUpdateDisplayLinkDisplay(window);
  return eTrue;
}

niExportFunc(void) osxglDestroyContext(iOSWindow* apWindow)
{
  if (!osxglHasContext(apWindow))
    return;

  OSXGL_WINDOW();

  if (apWindow == osxglGetCurrentContext()) {
    osxglMakeContextCurrent(NULL);
  }

  if (window->nsGL.displayLink) {
    if (CVDisplayLinkIsRunning(window->nsGL.displayLink))
      CVDisplayLinkStop(window->nsGL.displayLink);
    CVDisplayLinkRelease(window->nsGL.displayLink);
  }
  window->nsGL.pixelFormat = nil;
  window->nsGL.context = nil;
}

niExportFunc(tBool) osxglHasContext(iOSWindow* apWindow) {
  OSXGL_WINDOW();
  return window->nsGL.context != nil;
}

niExportFunc(void) osxglMakeContextCurrent(iOSWindow* apWindow)
{
  if (apWindow) {
    // niDebugFmt(("... osxglMakeContextCurrent, has window"));
    OSXGL_WINDOW();
    if (window->nsGL.context) {
      // niDebugFmt(("... osxglMakeContextCurrent, has an opengl context"));
      [window->nsGL.context makeCurrentContext];
      pthread_setspecific(_osxgl.current, window);
    }
    else {
      // niDebugFmt(("... osxglMakeContextCurrent, doesnt have an opengl context"));
    }
  }
  else {
    [NSOpenGLContext clearCurrentContext];
    pthread_setspecific(_osxgl.current, NULL);
  }
}

niExportFunc(iOSWindow*) osxglGetCurrentContext(void)
{
  return (iOSWindow*)pthread_getspecific(_osxgl.current);
}

niExportFunc(void) osxglSwapBuffers(iOSWindow* apWindow, tBool abDoNotWait)
{
  OSXGL_WINDOW();

  if (!abDoNotWait) {
    if (window->mfRefreshTimer < 0) {
      if (window->mbIsActive) {
        window->nsGL.drawEvent.WaitEx(_knEventDrivenWindowTimerMs_Active);
      }
      else if (window->GetIsMinimized()) {
        window->nsGL.drawEvent.WaitEx(_knEventDrivenWindowTimerMs_Minimized);
      }
      else {
        window->nsGL.drawEvent.WaitEx(_knEventDrivenWindowTimerMs_Background);
      }
    }

    if (window->nsGL.swapInterval > 0) {
      // Wait for 200ms max, in case something went wrong with the displaylink I guess?
      window->nsGL.vsyncEvent.WaitEx(1000/5);
    }
  }

  [window->nsGL.context flushBuffer];
}

niExportFunc(tPtr) osxglGetProcAddress(const char* procname)
{
  CFStringRef symbolName = CFStringCreateWithCString(
      kCFAllocatorDefault,procname,kCFStringEncodingASCII);

  tPtr symbol = (tPtr)CFBundleGetFunctionPointerForName(
      (CFBundleRef)_osxgl.framework, symbolName);
  CFRelease(symbolName);

  return symbol;
}

niExportFunc(tIntPtr) osxglGetNSOpenGLContext(iOSWindow* apWindow)
{
  OSXGL_WINDOW();
  niCAssert(sizeof(tIntPtr) == sizeof(id));
  return (tIntPtr)window->nsGL.context;
}

niExportFunc(void) osxglUpdateDisplayLinkDisplay(iOSWindow* apWindow)
{
  OSXGL_WINDOW();
  CGDirectDisplayID displayID = [[[window->nsWindow screen] deviceDescription][@"NSScreenNumber"] unsignedIntValue];
  if (!displayID)
    return;
  CVDisplayLinkSetCurrentCGDisplay(window->nsGL.displayLink, displayID);
}

///--------------------------------------------------------------------------------------------
///
/// NIWindowDelegate implementation
///
///--------------------------------------------------------------------------------------------

@implementation NIWindowDelegate
///////////////////////////////////////////////
-(void) listen:(cOSXWindow *)data {
  NSNotificationCenter *center;

  _wnd = data;
  NSView *view = [_wnd->nsWindow contentView];

#if !defined OSX_SWITCH_INOUT_ON_FOCUS
  _wnd->mbObservingVisible = YES;
  _wnd->mbWasVisible = [_wnd->nsWindow isVisible];
#endif

  center = [NSNotificationCenter defaultCenter];

  [_wnd->nsWindow setNextResponder:self];
  if ([_wnd->nsWindow delegate] != nil) {
    [center addObserver:self selector:@selector(windowDisExpose:) name:NSWindowDidExposeNotification object:_wnd->nsWindow];
    [center addObserver:self selector:@selector(windowDidMove:) name:NSWindowDidMoveNotification object:_wnd->nsWindow];
    [center addObserver:self selector:@selector(windowDidResize:) name:NSWindowDidResizeNotification object:_wnd->nsWindow];
    [center addObserver:self selector:@selector(windowDidMiniaturize:) name:NSWindowDidMiniaturizeNotification object:_wnd->nsWindow];
    [center addObserver:self selector:@selector(windowDidDeminiaturize:) name:NSWindowDidDeminiaturizeNotification object:_wnd->nsWindow];
    [center addObserver:self selector:@selector(windowDidBecomeKey:) name:NSWindowDidBecomeKeyNotification object:_wnd->nsWindow];
    [center addObserver:self selector:@selector(windowDidResignKey:) name:NSWindowDidResignKeyNotification object:_wnd->nsWindow];
    [center addObserver:self selector:@selector(windowWillEnterFullScreen:) name:NSWindowWillEnterFullScreenNotification object:_wnd->nsWindow];
    [center addObserver:self selector:@selector(windowDidEnterFullScreen:) name:NSWindowDidEnterFullScreenNotification object:_wnd->nsWindow];
    [center addObserver:self selector:@selector(windowWillExitFullScreen:) name:NSWindowWillExitFullScreenNotification object:_wnd->nsWindow];
    [center addObserver:self selector:@selector(windowDidExitFullScreen:) name:NSWindowDidExitFullScreenNotification object:_wnd->nsWindow];
    [center addObserver:self selector:@selector(windowDidChangeScreen:) name:NSWindowDidChangeScreenNotification object:_wnd->nsWindow];
  } else {
    [_wnd->nsWindow setDelegate:self];
  }

#if !defined OSX_SWITCH_INOUT_ON_FOCUS
  /* Haven't found a delegate / notification that triggers when the window is
   * ordered out (is not visible any more). You can be ordered out without
   * minimizing, so DidMiniaturize doesn't work. (e.g. -[NSWindow orderOut:])
   */
  [_wnd->nsWindow addObserver:self
   forKeyPath:@"visible"
   options:NSKeyValueObservingOptionNew
   context:NULL];
#endif

  [_wnd->nsWindow setNextResponder:self];
  [_wnd->nsWindow setAcceptsMouseMovedEvents:YES];

  [view setNextResponder:self];
#ifdef HANDLE_TOUCH_EVENTS
  [view setAcceptsTouchEvents:YES];
#endif
}

///////////////////////////////////////////////
-(void) close {
  NSNotificationCenter *center;

  center = [NSNotificationCenter defaultCenter];

  [_wnd->nsWindow setNextResponder:nil];
  if ([_wnd->nsWindow delegate] != self) {
    [center removeObserver:self name:NSWindowDidExposeNotification object:_wnd->nsWindow];
    [center removeObserver:self name:NSWindowDidMoveNotification object:_wnd->nsWindow];
    [center removeObserver:self name:NSWindowDidResizeNotification object:_wnd->nsWindow];
    [center removeObserver:self name:NSWindowDidMiniaturizeNotification object:_wnd->nsWindow];
    [center removeObserver:self name:NSWindowDidDeminiaturizeNotification object:_wnd->nsWindow];
    [center removeObserver:self name:NSWindowDidBecomeKeyNotification object:_wnd->nsWindow];
    [center removeObserver:self name:NSWindowDidResignKeyNotification object:_wnd->nsWindow];
  } else {
    [_wnd->nsWindow setDelegate:nil];
  }
  [center removeObserver:self name:NSApplicationDidHideNotification object:NSApp];
  [center removeObserver:self name:NSApplicationDidUnhideNotification object:NSApp];
}

///////////////////////////////////////////////
-(BOOL) windowShouldClose:(id) sender {
  TRACE_OSX(("... OSX: windowShouldClose"));
  if (!_wnd) return NO;
  if (_wnd->GetRequestedClose()) {
    // already requested close...
  }
  else if (!_wnd->mptrMT.IsOK() || _wnd->mptrMT->empty()) {
    _wnd->SetRequestedClose(eTrue);
    return YES;
  }
  else {
    _wnd->_SendMessage(eOSWindowMessage_Close);
  }
  return NO;
}

///////////////////////////////////////////////
-(void) windowDidMove:(NSNotification *) aNotification {
  TRACE_OSX(("... OSX: windowDidMove"));
  if (!_wnd) return;
  if (_wnd->nsGL.context) {
    [_wnd->nsGL.context update];
  }
  _wnd->nsViewRect = [_wnd->nsView frame];
  _wnd->_SendMessage(eOSWindowMessage_Move);
}

///////////////////////////////////////////////
-(void) windowDidResize:(NSNotification *) aNotification {
  TRACE_OSX(("... OSX: windowDidResize"));
  if (!_wnd) return;
  if (_wnd->nsGL.context) {
    [_wnd->nsGL.context update];
  }
  _wnd->nsViewRect = [_wnd->nsView frame];
  _wnd->_SendMessage(eOSWindowMessage_Size);
}

///////////////////////////////////////////////
-(void) windowDidMiniaturize:(NSNotification *) aNotification {
  TRACE_OSX(("... OSX: windowDidMiniaturize"));
  if (!_wnd) return;
  _wnd->_SendMessage(eOSWindowMessage_SwitchOut, eOSWindowSwitchReason_Minimized);
}

///////////////////////////////////////////////
-(void) windowDidDeminiaturize:(NSNotification *) aNotification {
  TRACE_OSX(("... OSX: windowDidDeminiaturize"));
  if (!_wnd) return;
  //     _wnd->_SendMessage(eOSWindowMessage_SwitchIn);
}

///////////////////////////////////////////////
-(void) windowDidBecomeKey:(NSNotification *) aNotification {
  TRACE_OSX(("... OSX: windowDidBecomeKey"));
  if (!_wnd) return;
  _wnd->SwitchIn(eOSWindowSwitchReason_SetFocus);
  _wnd->_SendMessage(eOSWindowMessage_SetFocus);
  _wnd->_UpdateCursor(_wnd->mCursor);
}

///////////////////////////////////////////////
-(void) windowDidResignKey:(NSNotification *) aNotification {
  TRACE_OSX(("... OSX: windowDidResignKey"));
  if (!_wnd) return;
  _wnd->_UpdateCursor(eOSCursor_Arrow);
  _wnd->_SendMessage(eOSWindowMessage_LostFocus);
  _wnd->SwitchOut(eOSWindowSwitchReason_LostFocus);
}

///////////////////////////////////////////////
-(void) windowDidExpose:(NSNotification *) aNotification {
  TRACE_OSX(("... OSX: windowDidExpose"));
  if (!_wnd) return;
}


///////////////////////////////////////////////
-(void) windowDidChangeBackingProperties:(NSNotification *) aNotification {

  NSWindow *theWindow = (NSWindow *)[aNotification object];
  // NSLog(@"windowDidChangeBackingProperties: window=%@", theWindow);

  CGFloat newBackingScaleFactor = [theWindow backingScaleFactor];
  CGFloat oldBackingScaleFactor = [[[aNotification userInfo]
                                    objectForKey:@"NSBackingPropertyOldScaleFactorKey"]
                                   doubleValue];
  if (newBackingScaleFactor != oldBackingScaleFactor) {
    TRACE_OSX(("... OSX: The backing scale factor changed from %.1f -> %.1f",
               oldBackingScaleFactor, newBackingScaleFactor));
  }

  /*
  NSColorSpace *newColorSpace = [theWindow colorSpace];
  NSColorSpace *oldColorSpace = [[aNotification userInfo]
                                 objectForKey:@"NSBackingPropertyOldColorSpaceKey"];
  if (![newColorSpace isEqual:oldColorSpace]) {
    NSLog(@"\tThe color space changed from %@ -> %@", oldColorSpace, newColorSpace);
  }
  */
}

///////////////////////////////////////////////
-(void) windowWillEnterFullScreen:(NSNotification *) aNotification {
  TRACE_OSX(("... OSX: windowWillEnterFullScreen"));
  if (!_wnd) return;
}
-(void) windowDidEnterFullScreen:(NSNotification *) aNotification {
  TRACE_OSX(("... OSX: windowDidEnterFullScreen"));
  if (!_wnd) return;
  _wnd->nsIsFullscreen = eTrue;
}
-(void) windowWillExitFullScreen:(NSNotification *) aNotification {
  TRACE_OSX(("... OSX: windowWillExitFullScreen"));
  if (!_wnd) return;
}
-(void) windowDidExitFullScreen:(NSNotification *) aNotification {
  TRACE_OSX(("... OSX: windowDidExitFullScreen"));
  if (!_wnd) return;
  _wnd->nsIsFullscreen = eFalse;
}

///////////////////////////////////////////////
-(void) windowDidChangeScreen:(NSNotification *) aNotification {
  TRACE_OSX(("... OSX: windowDidChangeScreen"));
  if (!_wnd) return;
  osxglUpdateDisplayLinkDisplay(_wnd);
}

@end

//--------------------------------------------------------------------------------------------
//
//  sOSXSystem implementation
//
//--------------------------------------------------------------------------------------------
struct sOSXSystem : public cMemImpl {
  astl::vector<cOSXWindow*> mvWindows;
  struct sMonitor {
    tIntPtr   mHandle;
    cString   mstrName;
    sRecti    mRect;
    sRecti    mBounds;
    tOSMonitorFlags mFlags;
    tU32      mVendorID;
    tU32      mProductID;
    ni::Ptr<tVec2iCVec> _resolutions;

    tU32 EnumerateResolutions(astl::vector<sVec2i>& aResolutions) {
      CFArrayRef displayModes = CGDisplayCopyAllDisplayModes(mHandle, nil);
      niDefer { CFRelease(displayModes); };
      astl::set<sVec2i> enumeratedResolutions;
      for(CFIndex i = 0; i < CFArrayGetCount(displayModes); ++i)
      {
        CGDisplayModeRef currentDisplayMode = (CGDisplayModeRef)CFArrayGetValueAtIndex(displayModes, i);
        sVec2i displayResolution = Vec2i(
          (uint32_t)CGDisplayModeGetWidth(currentDisplayMode),
          (uint32_t)CGDisplayModeGetHeight(currentDisplayMode)
        );
        enumeratedResolutions.insert(displayResolution);
      }
      aResolutions.reserve(enumeratedResolutions.size());
      aResolutions.assign(enumeratedResolutions.begin(),enumeratedResolutions.end());
      return enumeratedResolutions.size();
    }
  };
  astl::vector<sMonitor>  mvMonitors;

  sOSXSystem() {
    // Detect displays
    CGDisplayCount displayCount;
    CGDisplayErr r = CGGetOnlineDisplayList(0,NULL,&displayCount);
    if (r == kCGErrorSuccess) {
      astl::vector<CGDirectDisplayID> displays;
      displays.resize(displayCount);
      r = CGGetOnlineDisplayList(displayCount,displays.data(),&displayCount);
      if (r == kCGErrorSuccess) {
        niLoop(i,displayCount) {
          _AddMonitor(displays[i]);
        }
      }
    }

    // If no monitors add on default one...
    if (mvMonitors.empty()) {
      _AddMonitor(CGMainDisplayID());
    }
  }

  ///////////////////////////////////////////////
  NSString* screenNameForDisplay(CGDirectDisplayID displayID)
  {
    if (@available(macOS 10.15, *)) {
      NSArray *screens = [NSScreen screens];

      for (NSScreen *screen in screens) {
        CGDirectDisplayID screenDisplayID = [[[screen deviceDescription] objectForKey:@"NSScreenNumber"] intValue];
        if (screenDisplayID == displayID) {
          NSString* screenName = screen.localizedName;
          return screenName;
        }
      }
      return nil;
    }
    else {
      NSString *screenName = nil;
      NSDictionary *deviceInfo = (__bridge NSDictionary *)IODisplayCreateInfoDictionary(CGDisplayIOServicePort(displayID), kIODisplayOnlyPreferredName);
      NSDictionary *localizedNames = [deviceInfo objectForKey:[NSString stringWithUTF8String:kDisplayProductName]];
      if ([localizedNames count] > 0) {
        screenName = [localizedNames objectForKey:[[localizedNames allKeys] objectAtIndex:0]];
      }
      return screenName;
    }
  }

  void _AddMonitor(CGDirectDisplayID displayID) {
    if (CGDisplayIsInMirrorSet(displayID))
      return;

    CFDictionaryRef moderef = CGDisplayCurrentMode(displayID);
    if (!moderef)
      return;

    sMonitor m;
    m.mHandle = displayID;
    NSString* screenName = screenNameForDisplay(displayID);
    if (screenName == nil) {
      m.mstrName.Format(_A("Unnamed%d"),displayID);
    }
    else {
      m.mstrName = [screenName UTF8String];
    }
    m.mRect = ni::sRecti(0,0,
                         CGDisplayPixelsWide(m.mHandle),
                         CGDisplayPixelsHigh(m.mHandle));

    CGRect displayBounds = CGDisplayBounds(displayID);
    m.mBounds = Recti((int32_t)displayBounds.origin.x,
                      (int32_t)displayBounds.origin.y,
                      (int32_t)displayBounds.size.width,
                      (int32_t)displayBounds.size.height);
    m.mFlags = 0;
    if (CGDisplayIsMain(m.mHandle)) {
      m.mFlags |= eOSMonitorFlags_Primary;
    }
    m.mVendorID = CGDisplayVendorNumber(displayID);
    m.mProductID = CGDisplayModelNumber(displayID);

    cString strResolutions;
#if 0
    astl::vector<sVec2i> resolutions;
    m.EnumerateResolutions(resolutions);
    strResolutions << "\nResolutions: " << (tU32)resolutions.size() << AEOL;
    for (auto it = resolutions.begin(); it != resolutions.end(); ++it) {
      strResolutions << "- " << it->x << "x" << it->y << AEOL;
    }
#endif

    mvMonitors.push_back(m);
    niLog(Info, niFmt(
      "... OSX: Monitor %d: ID:%X name:'%s' rect:%s bounds:%s flags:%d vendorID:0x%X productID:0x%X%s\n",
      mvMonitors.size()-1,
      m.mHandle, m.mstrName.Chars(),
      m.mRect, m.mBounds,
      m.mFlags,
      m.mVendorID, m.mProductID,
      strResolutions));
  }
};

static sOSXSystem* _GetOSXSystem() {
  static sOSXSystem* _system = niNew sOSXSystem();
  return _system;
}

///////////////////////////////////////////////
static void _RegisterWindow(cOSXWindow* apWindow) {
  sOSXSystem* osx = _GetOSXSystem();
  astl::push_back_once(osx->mvWindows,apWindow);
  TRACE_OSX(("... OSX: _RegisterWindow: %p, %d window opened.",
             (tIntPtr)apWindow, osx->mvWindows.size()));
}
static void _UnregisterWindow(cOSXWindow* apWindow) {
  sOSXSystem* osx = _GetOSXSystem();
  astl::find_erase(osx->mvWindows,apWindow);
  TRACE_OSX(("... OSX: _UnregisterWindow: %p, %d window opened.",
             (tIntPtr)apWindow, osx->mvWindows.size()));
}

static void _TerminateApp() {
  _RegisterApp();
  sOSXSystem* osx = _GetOSXSystem();
  TRACE_OSX(("... OSX: Requested App Exit: %d window opened.", osx->mvWindows.size()));
  while (!osx->mvWindows.empty()) {
    osx->mvWindows.back()->Invalidate();
  }
}

void cLang::FatalError(const achar* aszMsg) {
  sOSXSystem* osx = _GetOSXSystem();

  // Log message
  cString logMessage = niFmt(_A("[FATAL ERROR]\n%s\n"), aszMsg);
  niLog(Error,logMessage);

  if (!osx->mvWindows.empty() && !ni_debug_get_print_asserts()) {
    // Dialog message
    cString dialogMessage = logMessage;

    astl::vector<cString> logs;
    ni_get_last_logs(&logs,20);
    if (!logs.empty()) {
      dialogMessage += "--- Last logs ---\n";
      niLoop(i,logs.size()) {
        dialogMessage += logs[i];
      }
    }

    ni::GetLang()->MessageBox(NULL, "Fatal Error", dialogMessage.Chars(), eOSMessageBoxFlags_Ok|eOSMessageBoxFlags_IconError);
  }

  _PlatformExit(0xDEADBEEF);
}

//--------------------------------------------------------------------------------------------
//
//  cLang implementation
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
void cLang::_PlatformStartup() {
}

///////////////////////////////////////////////
tU32 cLang::GetNumMonitors() const {
  sOSXSystem* osx = _GetOSXSystem();
  return (tU32)osx->mvMonitors.size();
}
tU32 cLang::GetMonitorIndex(tIntPtr aHandle) const {
  sOSXSystem* osx = _GetOSXSystem();
  niLoop(i,osx->mvMonitors.size()) {
    if (osx->mvMonitors[i].mHandle == aHandle)
      return (tU32)i;
  }
  return eInvalidHandle;
}
tIntPtr cLang::GetMonitorHandle(tU32 anIndex) const {
  sOSXSystem* osx = _GetOSXSystem();
  niCheckSilent(anIndex < osx->mvMonitors.size(),eInvalidHandle);
  return osx->mvMonitors[anIndex].mHandle;
}
const achar* cLang::GetMonitorName(tU32 anIndex) const {
  sOSXSystem* osx = _GetOSXSystem();
  niCheckSilent(anIndex < osx->mvMonitors.size(),NULL);
  return osx->mvMonitors[anIndex].mstrName.Chars();
}
sRecti cLang::GetMonitorRect(tU32 anIndex) const {
  sOSXSystem* osx = _GetOSXSystem();
  niCheckSilent(anIndex < osx->mvMonitors.size(),sRecti::Null());
  return osx->mvMonitors[anIndex].mRect;
}
tOSMonitorFlags cLang::GetMonitorFlags(tU32 anIndex) const {
  sOSXSystem* osx = _GetOSXSystem();
  niCheckSilent(anIndex < osx->mvMonitors.size(),0);
  return osx->mvMonitors[anIndex].mFlags;
}

///////////////////////////////////////////////
iOSWindow* __stdcall cLang::CreateWindow(iOSWindow* apParent, const achar* aaszTitle, const sRecti& aRect, tOSWindowCreateFlags aCreate, tOSWindowStyleFlags aStyle) {
  if (!_RegisterApp()) {
    niError(_A("Can't register application !"));
    return NULL;
  }

  niCheck(aRect.GetWidth() > 0,NULL);
  niCheck(aRect.GetWidth() < 0xFFFF,NULL);
  niCheck(aRect.GetHeight() > 0,NULL);
  niCheck(aRect.GetHeight() < 0xFFFF,NULL);

  NSRect rect = _ToNSRect(NULL,aRect);
  unsigned int style = _ToNSWindowStyle(aStyle);
  NSWindow* nsWindow = [[NIWindow alloc] initWithContentRect:rect styleMask:style backing:NSBackingStoreBuffered defer:FALSE];

  // Add the OSX Lion fullscreen button if the window is not fixed size
  if (!niFlagIs(aStyle,eOSWindowStyleFlags_FixedSize)) {
    if ([nsWindow respondsToSelector: @selector(toggleFullScreen:)]) {
      [nsWindow setCollectionBehavior:
       [nsWindow collectionBehavior] | NSWindowCollectionBehaviorFullScreenPrimary];
    }
  }

  // Disable native macOS sierra tabs
  if (NSAppKitVersionNumber > 1500) {
    [nsWindow setValue:[NSNumber numberWithInt:2] forKey:@"tabbingMode"];
  }

  Ptr<cOSXWindow> wnd = niNew cOSXWindow(apParent);

  NSView* nsView = [[NIContentView alloc] initWithOSXWindow:wnd.ptr()];
  [nsWindow setContentView: nsView];

  if (!wnd->_RetainWindow(nsWindow,nsView,eTrue,eTrue)) {
    niError(_A("Can't retain window !"));
    wnd = NULL;
  }
  else {
    wnd->SetTitle(aaszTitle);
  }

  return wnd.GetRawAndSetNull();
}
iOSWindow* __stdcall cLang::CreateWindowEx(tIntPtr aOSWindowHandle, tOSWindowCreateFlags aCreate) {
  niError("Not implemented !");
  return NULL;
}

///////////////////////////////////////////////
eOSMessageBoxReturn __stdcall cLang::MessageBox(iOSWindow* apParent, const achar* aaszTitle, const achar* aaszText, tOSMessageBoxFlags aFlags)
{
  _RegisterApp();
  cString strTitle = niIsStringOK(aaszTitle)?aaszTitle:"Message";
  cString strText = niIsStringOK(aaszText)?aaszText:_A("");
  if (!_bAppHasBeenRegistered) {
    niPrintln(niFmt("--- %s ---\n%s\n",
                            strTitle.Chars(),
                            strText.Chars()));
    return eOSMessageBoxReturn_Yes;
  }
  else {
    NSString* title = _ToNSString(strTitle.Chars());
    NSString* msg = _ToNSString(strText.Chars());
    eOSMessageBoxReturn r = eOSMessageBoxReturn_Error;

    NSAlert* alert = [[NSAlert alloc] init];
    [alert setMessageText:title];

#if 1
    NSSize msgSize = [msg sizeWithAttributes:@{NSFontAttributeName: [NSFont systemFontOfSize:[NSFont systemFontSize]]}];
    alert.accessoryView = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, ni::Min(msgSize.width,750), 0)];
    [alert setInformativeText:msg];
#else
    // This is kept as reference, we could in the future use NSTextView or
    // NSTextField to make a clean left aligned "report" section. I think this
    // would also need a special message box for this style of message box,
    // that style would be useful to display error logs more cleanly.
    NSTextField *textField;
    textField = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 200, 100)];
    [textField setStringValue:msg];
    [textField setBezeled:NO];
    [textField setDrawsBackground:NO];
    [textField setEditable:NO];
    [textField setSelectable:NO];
    alert.accessoryView = textField;
#endif

    if (niFlagIs(aFlags,eOSMessageBoxFlags_IconWarning) ||
        niFlagIs(aFlags,eOSMessageBoxFlags_IconError)) {
      [alert setAlertStyle:NSAlertStyleCritical];
    }
    else if (niFlagIs(aFlags,eOSMessageBoxFlags_IconInfo)) {
      [alert setAlertStyle:NSAlertStyleInformational];
    }

    if (niFlagIs(aFlags,eOSMessageBoxFlags_YesNo)) {
      [alert addButtonWithTitle:@"Yes"];
      [alert addButtonWithTitle:@"No"];
    }
    else if (niFlagIs(aFlags,eOSMessageBoxFlags_OkCancel)) {
      [alert addButtonWithTitle:@"Ok"];
      [alert addButtonWithTitle:@"Cancel"];
    }
    else {
      [alert addButtonWithTitle:@"Ok"];
    }

    [[NSRunningApplication currentApplication] activateWithOptions:NSApplicationActivateIgnoringOtherApps];

    NSInteger i = [alert runModal];
    if (i == NSAlertFirstButtonReturn) {
      r = eOSMessageBoxReturn_Yes;
    }
    else {
      r = eOSMessageBoxReturn_No;
    }
    return r;
  }
}

// source of code modal logic: http://stackoverflow.com/questions/604768/wait-for-nsalert-beginsheetmodalforwindow
// note: whether extensions are actually shown depends on a user setting in Finder; we can't control it here
static void _setupSavePanel(NSSavePanel *s, const achar* aTitle, const achar* aFilter, const achar* aInitDir) {
  [s setCanCreateDirectories:YES];
  [s setShowsHiddenFiles:YES];
  [s setExtensionHidden:NO];
  [s setCanSelectHiddenExtension:NO];
  [s setTreatsFilePackagesAsDirectories:YES];
  if (niStringIsOK(aTitle)) {
    // niDebugFmt(("... _setupSavePanel: setTitle: '%s'", aTitle));
    [s setTitle:_ToNSString(aTitle)];
  }
  if (niStringIsOK(aInitDir)) {
    // niDebugFmt(("... _setupSavePanel: setDirectoryURL: '%s'", aInitDir));
    [s setDirectoryURL:[NSURL fileURLWithPath:_ToNSString(niFmt("file://%s",aInitDir))]];
  }
  if (niStringIsOK(aFilter)) {
    tBool allowAllExts = eFalse;
    astl::vector<cString> exts;
    StringSplit(_ASTR(aFilter), ";", &exts);
    NSMutableArray* filters = [[NSMutableArray alloc] init];
    niLoop(i, exts.size()) {
      if (exts[i].IEq("*")) {
        allowAllExts = eTrue;
        continue;
      }
      [filters addObject:_ToNSString(exts[i].Chars())];
    }
    [s setAllowedFileTypes:filters];
    if (allowAllExts) {
      [s setAllowsOtherFileTypes:TRUE];
    }
  }
}

static cString _runSavePanel(NSWindow *parent, NSSavePanel *s) {
  _RegisterApp();
  [s beginSheetModalForWindow:parent completionHandler:^(NSInteger result) {
      [NSApp stopModalWithCode:result];
    }];
  const tBool isOK = ([NSApp runModalForWindow:s] == NSFileHandlingPanelOKButton);
  if (parent) {
    // Make the parent window active. Without this the file dialog stays
    // dangling when we switch back if we switch to another window right away
    // ; that happens when we open a file in a new application or window as
    // the result of a file selection for example.
    [parent makeKeyAndOrderFront:nil];
  }

  if (isOK) {
    return _ASTR([[[s URL] path] UTF8String]);
  }
  else {
    return "";
  }
}

///////////////////////////////////////////////
cString __stdcall cLang::OpenFileDialog(iOSWindow* aParent, const achar* aTitle, const achar* aFilter, const achar* aInitDir) {
  NSWindow* nsParent = nil;
  if (niIsOK(aParent)) {
    nsParent = ((cOSXWindow*)aParent)->nsWindow;
  }

  NSOpenPanel* o = [NSOpenPanel openPanel];
  if (!o) {
    niError("Can't open NSOpenPanel.");
    return "";
  }
  [o setCanChooseFiles:YES];
  [o setCanChooseDirectories:NO];
  [o setResolvesAliases:NO];
  [o setAllowsMultipleSelection:NO];
  _setupSavePanel(o, aTitle, aFilter, aInitDir);
  // panel is autoreleased
  return _runSavePanel(nsParent, o);
}

///////////////////////////////////////////////
cString __stdcall cLang::SaveFileDialog(iOSWindow* aParent, const achar* aTitle, const achar* aFilter, const achar* aInitDir) {
  NSWindow* nsParent = nil;
  if (niIsOK(aParent)) {
    nsParent = ((cOSXWindow*)aParent)->nsWindow;
  }

  NSSavePanel* s = [NSSavePanel savePanel];
  if (!s) {
    niError("Can't open NSSavePanel.");
    return "";
  }
  _setupSavePanel(s, aTitle, aFilter, aInitDir);
  // panel is autoreleased
  return _runSavePanel(nsParent, s);
}

///////////////////////////////////////////////
cString __stdcall cLang::PickDirectoryDialog(iOSWindow* aParent, const achar* aTitle, const achar* aInitDir) {
  NSWindow* nsParent = nil;
  if (niIsOK(aParent)) {
    nsParent = ((cOSXWindow*)aParent)->nsWindow;
  }

  NSOpenPanel* o = [NSOpenPanel openPanel];
  if (!o) {
    niError("Can't open NSOpenPanel.");
    return "";
  }
  [o setCanChooseFiles:NO];
  [o setCanChooseDirectories:YES];
  [o setResolvesAliases:NO];
  [o setAllowsMultipleSelection:NO];
  _setupSavePanel(o, aTitle, NULL, aInitDir);
  // panel is autoreleased
  return _runSavePanel(nsParent, o);
}

///////////////////////////////////////////////
tU32 __stdcall cLang::GetNumGameCtrls() const {
  return 0;
}
iGameCtrl* __stdcall cLang::GetGameCtrl(tU32 aulIdx) const {
  return NULL;
}

#endif // niOSX
