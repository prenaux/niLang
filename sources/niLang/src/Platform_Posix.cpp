#include "API/niLang/Types.h"

#if defined niPosix

#include "API/niLang/Platforms/Unix/UnixFindFirst.h"
#include "API/niLang/StringDef.h"
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include "FileFd.h"
#include "API/niLang/Utils/FileEnum.h"
#include <niLang/Utils/Trace.h>

#define TRACE_UNIX_FF(X) //niDebugFmt(X)
#define NAMELEN(dirent) (strlen((dirent)->d_name))

#include "API/niLang/IRegex.h"

#if defined niOSX || defined niLinux || defined niJSCC
#define EMULATE_CIPATH
#endif

#if defined EMULATE_CIPATH

niDeclareModuleProperty(niLang,TraceUnixCiPath,ni::eFalse);
#define _TraceUnixCiPath(FMT) niModuleTrace_(niLang,TraceUnixCiPath,FMT);

// r must have strlen(path) + 2 bytes
static ni::tBool _find_ci_path(ni::achar *r, const ni::achar* path, const ni::achar* context)
{
  // return ni::eFalse;
  static int _count = 0;
  _TraceUnixCiPath(("Looking for case insensitive path %d (%s) '%s'.", _count++, context, path));

  const int l = ni::StrSize(path);
  char *p = (char*)alloca(l + 1);
  strcpy(p, path);
  size_t rl = 0;

  DIR *d;
  if (p[0] == '/')
  {
    d = opendir("/");
    p = p + 1;
    if (p[0] == '/') {
      // Handle '//' paths
      p = p + 1;
    }
  }
  else
  {
    d = opendir(".");
    r[0] = '.';
    r[1] = 0;
    rl = 1;
  }

  int last = 0;
  char *c = strsep(&p, "/");
  while (c)
  {
    if (!d) {
      return ni::eFalse;
    }
    if (last) {
      closedir(d);
      return ni::eFalse;
    }

    r[rl] = '/';
    rl += 1;
    r[rl] = 0;

    struct dirent *e = readdir(d);
    while (e)
    {
      if (strcasecmp(c, e->d_name) == 0)
      {
        strcpy(r + rl, e->d_name);
        rl += strlen(e->d_name);
        closedir(d);
        d = opendir(r);
        // niDebugFmt(("... opendir: %s - %p", r, (ni::tIntPtr)d));
        break;
      }

      e = readdir(d);
    }

    if (!e)
    {
      strcpy(r + rl, c);
      rl += strlen(c);
      last = 1;
    }

    c = strsep(&p, "/");
  }

  if (d)
    closedir(d);

  _TraceUnixCiPath(("Found case insensitive path (%s) '%s'.", context, r));
  return ni::eTrue;
}

struct sCIPath {
  __sync_mutex();
  astl::map<ni::cString,ni::cString> _cache;

  ~sCIPath() {
  }

  ni::tBool ResolveDir(ni::achar* r, const ni::achar* path) {
    __sync_lock();
    astl::map<ni::cString,ni::cString>::iterator it = _cache.find(path);
    if (it == _cache.end()) {
      if (!_find_ci_path(r,path,"cache"))
        *r = 0;
      it = astl::upsert(_cache,path,r);
      // niDebugFmt(("... ResolveDir: Add to cache: %s -> %s", path, r));
    }
    else {
      // niDebugFmt(("... ResolveDir: from cache: %s -> %s", path, it->second.Chars()));
      niUnused(it);
    }
    *r = 0;
    ni::StrCpy(r,it->second.Chars());
    return niStringIsOK(r);
  }

  ni::tBool ResolvePath(ni::achar* r, const ni::achar* path, const ni::achar* context) {
    // niDebugFmt(("--- ResolvePath: path: %s, context: %s", path, context));

    DIR* d = NULL;
    const size_t l = ni::StrSize(path);
    const int dirSlash = ni::StrZRFindChar(path,l,'/');
    const char* fileName = NULL;
    ni::cString dir;
    if (dirSlash >= 0) {
      dir.Set(path, path+dirSlash);
      fileName = path+dirSlash+1;
    }
    else {
      fileName = path;
    }
    // niDebugFmt(("... ResolvePath: dir: %s, file: %s", dir, fileName));

    if (dir.IsNotEmpty()) {
      if (!ResolveDir(r,dir.Chars())) {
        // niDebugFmt(("... ResolvePath: not a dir on disk, dir: %s, file: %s", dir, fileName));
        return ni::eFalse;
      }
      if (!*fileName) {
        // niDebugFmt(("... ResolvePath: we found the folder we were looking for"));
        return ni::eTrue;
      }
      // niDebugFmt(("... ResolvePath: got dir %s", r));
      d = opendir(r);
    }
    else {
      // niDebugFmt(("... ResolvePath: current dir"));
      ni::StrCpy(r,".");
      d = opendir(".");
    }

    if (!d) {
      // niDebugFmt(("... ResolvePath: couldnt open dir"));
      return ni::eFalse;
    }

    // niDebugFmt(("... ResolvePath: looking for file: %s in %s", fileName, r));
    struct dirent *e = readdir(d);
    while (e) {
      if (ni::StrIEq(fileName, e->d_name)) {
        ni::StrCat(r, "/");
        ni::StrCat(r, e->d_name);
        // niDebugFmt(("... ResolvePath: found the file: %s in %s", e->d_name, r));
        closedir(d);
        return ni::eTrue;
      }
      e = readdir(d);
    }

    closedir(d);
    return ni::eFalse;
  }
};

static sCIPath& _GetCIPath() {
  static sCIPath _ciPath;
  return _ciPath;
}
#endif

// Retourne le nom du fichier a partir d'un chemin complet
niExportFunc(ni::achar*) unix_ff_get_filename(ni::achar *path)
{
  ni::achar *p = path + ni::StrSize(path);
  while ((p > path) && (*(p - 1) != '/'))
    p--;
  return p;
}

// Si le dernier caractere d'un nom de fichier n'est pas '/', cette
// fonction l'ajoute
niExportFunc(void) unix_ff_put_backslash(ni::achar *filename, int size)
{
  int len = ni::StrSize(filename);
  if ((len > 0) && (len < (size - 1)) && (filename[len - 1] != '/')) {
    filename[len] = '/';
    filename[len + 1] = 0;
  }
}

// Trouve le premier fichier contenant le pattern passe, et qui n'a pas
// d'attribut qui n'est pas present dans attrib
niExportFunc(int) unix_findfirst(const ni::achar *pattern, sUnixFFBlk* aFFBlk)
{
  int ret;
  sUnixFFInfo& ffinfo = aFFBlk->info;
  memset(&ffinfo, 0, sizeof(ffinfo));

  ni::StrZCpy(ffinfo.dirname, niCountOf(ffinfo.dirname)-1, pattern);
  *unix_ff_get_filename(ffinfo.dirname) = 0;
  if (ffinfo.dirname[0] == 0)
    ni::StrCpy(ffinfo.dirname, _A("./"));
  else
    ni::StrPutPathSep(ffinfo.dirname);

  ni::StrZCpy(ffinfo.pattern,
              niCountOf(ffinfo.pattern)-1,
              unix_ff_get_filename(niConstCast(ni::achar*,pattern)));

  if (ni::StrCmp(ffinfo.pattern, _A("*.*")) == 0)
    ni::StrCpy(ffinfo.pattern, _A("*"));

  TRACE_UNIX_FF(("... Try opendir: '%s'", ffinfo.dirname));
  ffinfo.dir = opendir(ffinfo.dirname);
  if (ffinfo.dir == 0) {
#ifdef EMULATE_CIPATH
    if (_GetCIPath().ResolvePath(ffinfo.dirname,ffinfo.dirname, "unix_findfirst")) {
      TRACE_UNIX_FF(("... Try opendir after unix_casepath: '%s'", ffinfo.dirname));
      ffinfo.dir = opendir(ffinfo.dirname);
    }
#endif
    if (ffinfo.dir == 0) {
      return ((errno == 0) ? ENOENT : errno);
    }
    ni::StrPutPathSep(ffinfo.dirname);
  }

  ret = unix_findnext(aFFBlk);
  if(ret) {
    if (ffinfo.dir != 0) {
      closedir(ffinfo.dir);
      ffinfo.dir = 0;
    }
  }

  return ret;
}

// Termine une operation de recherche
niExportFunc(void) unix_findclose(sUnixFFBlk* aFFBlk)
{
  if (aFFBlk->info.dir != NULL) {
    closedir(aFFBlk->info.dir);
    aFFBlk->info.dir = NULL;
  }
}

#if defined ni64 && (!defined niOSX && !defined niIOS && !defined niAndroid)
#define USE_READDIR64
#endif

niExportFunc(ni::tBool) unix_filetime(const char* filename, time_t* apATime, time_t* apMTime, time_t* apCTime)
{
#ifdef USE_READDIR64
  struct stat64 fileStat;
  if (stat64(filename, &fileStat))
    return ni::eFalse;
#else
  struct stat fileStat;
  if (stat(filename, &fileStat))
    return ni::eFalse;
#endif

  if (apATime)
    *apATime = fileStat.st_atime;
  if (apMTime)
    *apMTime = fileStat.st_mtime;
  if (apCTime)
    *apCTime = fileStat.st_ctime;

  return ni::eTrue;
}

// Cherche le fichier suivant qui a un pattern correspondant
niExportFunc(int) unix_findnext(sUnixFFBlk* aFFBlk)
{
  sUnixFFInfo& ffinfo = aFFBlk->info;
  ni::cString tempName;
#ifdef USE_READDIR64
  struct dirent64* entry;
  struct stat64 fileStat;
#else
  struct dirent* entry;
  struct stat fileStat;
#endif // ni64

  while(1)
  {
#ifdef USE_READDIR64
    entry = readdir64(ffinfo.dir);
#else
    entry = readdir(ffinfo.dir);
#endif
    if (entry == NULL) {
      return ((errno == 0) ? ENOENT : errno);
    }

    tempName = entry->d_name;
    if (!unix_ff_match(tempName.Chars(), ffinfo.pattern)) {
      continue;
    }

    ni::StrCpy(ffinfo.filename, ffinfo.dirname);
    unix_ff_put_backslash(ffinfo.filename, niCountOf(ffinfo.filename));

    ni::StrZCat(ffinfo.filename,
                niCountOf(ffinfo.filename)-
                ni::StrSize(ffinfo.filename)-1,
                tempName.Chars());


#ifdef USE_READDIR64
    if (stat64(ffinfo.filename, &fileStat))
      continue;
#else
    if (stat(ffinfo.filename, &fileStat))
      continue;
#endif

    aFFBlk->attrib = 0;
    if ((fileStat.st_mode & S_IRUSR) == 0)
      aFFBlk->attrib |= _A_RDONLY;
    if (S_ISDIR(fileStat.st_mode))
      aFFBlk->attrib |= _A_SUBDIR;
    if ((tempName[0] == '.') &&
        ((tempName[1] != '.') || (tempName[2] != 0)))
      aFFBlk->attrib |= _A_HIDDEN;

    aFFBlk->time_write = fileStat.st_mtime;
    aFFBlk->size = fileStat.st_size;
    ni::StrZCpy(aFFBlk->name, niCountOf(aFFBlk->name)-1, tempName.Chars());

    return 0;
  }
}

niExportFunc(int) unix_ff_match(const ni::achar *fn, const ni::achar *pattern) {
  return ni::afilepattern_match(pattern,fn);
}

niExportFunc(FILE*) afopen(const ni::achar* file, const ni::achar* mode, ni::cString* apPathOnDisk) {
  FILE* r = fopen(file,mode);
  if (!r) {
#ifdef EMULATE_CIPATH
    ni::achar cipath[AMAX_PATH] = {0};
    if (_GetCIPath().ResolvePath(cipath, file, "afopen")) {
      r = fopen(cipath,mode);
      if (r && apPathOnDisk) {
        *apPathOnDisk = cipath;
      }
    }
#endif
  }
  else if (apPathOnDisk) {
    *apPathOnDisk = file;
  }
  return r;
}

niExportFunc(int) amkdir(const ni::achar* dir) {
#ifdef __MINGW32__
  return mkdir(dir);
#else
  return mkdir(dir,S_IRWXU);
#endif
}

niExportFunc(int) armdir(const ni::achar* dir) {
  int r = rmdir(dir);
#ifdef EMULATE_CIPATH
  if (r < 0) {
    ni::achar cipath[AMAX_PATH] = {0};
    if (_GetCIPath().ResolvePath(cipath, dir, "armdir")) {
      return rmdir(cipath);
    }
  }
#endif
  return r;
}

niExportFunc(int) aunlink(const ni::achar* path) {
  int r = unlink(path);
#ifdef EMULATE_CIPATH
  if (r < 0) {
    ni::achar cipath[AMAX_PATH] = {0};
    if (_GetCIPath().ResolvePath(cipath,path,"aunlink")) {
      return unlink(cipath);
    }
  }
#endif
  return r;
}

niExportFuncCPP(ni::cString) agetenv(const ni::achar* env) {
  return getenv(env);
}

niExportFunc(int) aputenv(const ni::achar* envString) {
  return putenv((char*)envString);
}

niExportFunc(int) FdOpen(const ni::achar* path, ni::cString* apPathOnDisk, int mode, int pmode) {
  int r = open(path,mode,pmode);
  if (r < 0) {
#ifdef EMULATE_CIPATH
    ni::achar cipath[AMAX_PATH] = {0};
    if (_GetCIPath().ResolvePath(cipath,path,"fdopen")) {
      r = open(cipath,mode,pmode);
      if (r >= 0 && apPathOnDisk) {
        *apPathOnDisk = cipath;
      }
    }
#endif
  }
  else if (apPathOnDisk) {
    *apPathOnDisk = path;
  }
  return r;
}

#ifndef niNoDLL
#include <dlfcn.h>
niExportFunc(ni::tIntPtr) ni_dll_load(const char* aName) {
  return (ni::tIntPtr)::dlopen(aName,RTLD_LAZY);
}
niExportFunc(void) ni_dll_free(ni::tIntPtr aModule) {
  ::dlclose((void*)aModule);
}
niExportFunc(ni::tPtr) ni_dll_get_proc(ni::tIntPtr aModule, const char* aProcName) {
  return (ni::tPtr)::dlsym((void*)aModule, aProcName);
}
#endif

#endif // niPosix
