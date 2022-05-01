#include <niLang.h>
#include <niLang/Utils/StrBreakIt.h>
#include "cleanup_pp.h"

namespace ni {

#define TRACE_CLEANUPPP
// #define TRACE_CLEANUPPP niDebugFmt

// Preprocessor to cleanup generated GLSL, this is necessary because the IE11
// preprocessor is buggy and will crash on a "recursive" define (such as
// "#define vs_v1 unpackColor4(vs_v1)")
tBool CleanupPP(cString& aOutput, const achar* aChars, const tU32 anLen) {
  StrCharIt charIt(aChars, anLen);
  StrBreakLineIt lineIt(charIt);
  astl::map<cString,cString> defines;
  bool skipCode = false;
  int inIf = -1;

  while (!lineIt.is_end()) {
    const achar* lineChars = lineIt.current().start();

    if (ni::StrStartsWith(lineChars,"//")) {
      // just skip this line
    }
    else if (ni::StrStartsWith(lineChars,"#if 0")) {
      if (inIf >= 0 || skipCode) {
        niError("Unexpected #if 0 encountered.");
        return eFalse;
      }
      inIf = 0;
      skipCode = true;
    }
    else if (ni::StrStartsWith(lineChars,"#if 1")) {
      if (inIf >= 1 || skipCode) {
        niError("Unexpected #if 1 encountered.");
        return eFalse;
      }
      inIf = 1;
      skipCode = false;
    }
    else if (ni::StrStartsWith(lineChars,"#if")) {
      niError("Unexpected #if encountered.");
      return eFalse;
    }
    else if (ni::StrStartsWith(lineChars,"#else")) {
      if (inIf == 0) {
        skipCode = false;
      }
      else if (inIf == 1) {
        skipCode = true;
      }
      else {
        niError("Unexpected #else encountered.");
        return eFalse;
      }
    }
    else if (ni::StrStartsWith(lineChars,"#endif")) {
      if (inIf == 0 || inIf == 1) {
        skipCode = false;
        inIf = -1;
      }
      else {
        niError("Unexpected #endif encountered.");
        return eFalse;
      }
    }
    else if (ni::StrStartsWith(lineChars,"#define")) {
      if (!skipCode) {
        StrBreakSpaceIt wordIt(lineIt.current());
        wordIt.next(); // skip #define
        cString defName(wordIt.current().start(),wordIt.current().end());
        cString defValue(wordIt.current().end()+1,lineIt.current().end());
        TRACE_CLEANUPPP(("... DEFINE: %s = %s", defName, defValue));
        astl::upsert(defines,defName,defValue);
      }
      else {
        TRACE_CLEANUPPP(("... SKIPDEF: %s", cString(lineIt.current().start(),lineIt.current().end())));
      }
    }
    else {
      if (skipCode) {
        TRACE_CLEANUPPP(("... SKIP: %s", cString(lineIt.current().start(),lineIt.current().end())));
      }
      else {
        if (*lineChars == '#') {
          if (ni::StrStartsWith(lineChars,"#extension") ||
              ni::StrStartsWith(lineChars,"#version"))
          {
            TRACE_CLEANUPPP(("... PP: %s", cString(lineIt.current().start(),lineIt.current().end())));
            aOutput << cString(lineIt.current().start(),lineIt.current().end()) << AEOL;
          }
          else {
            niError(niFmt("Unknown preprocessor directive: %s.",
                          cString(lineIt.current().start(),lineIt.current().end())));
            return eFalse;
          }
        }
        else {
          TRACE_CLEANUPPP(("... LINE: %s", cString(lineIt.current().start(),lineIt.current().end())));
          const char* prevEnd = NULL;
          StrBreakIdentifierIt wordIt(lineIt.current());
          while (!wordIt.is_end()) {
            if (prevEnd) {
              TRACE_CLEANUPPP(("...... SEP: %s", cString(prevEnd, wordIt.current().start())));
              aOutput << cString(prevEnd, wordIt.current().start());
            }
            prevEnd = wordIt.current().end();

            cString identifier(wordIt.current().start(),wordIt.current().end());
            astl::map<cString,cString>::const_iterator itDef = defines.find(identifier);
            if (itDef != defines.end()) {
              TRACE_CLEANUPPP(("...... RE: %s -> %s", identifier, itDef->second));
              aOutput << itDef->second;
            }
            else {
              TRACE_CLEANUPPP(("...... ID: %s", identifier));
              aOutput << identifier;
            }
            wordIt.next();
          }
          if (prevEnd) {
            TRACE_CLEANUPPP(("...... END: %s", cString(prevEnd, wordIt.current().start())));
            aOutput << cString(prevEnd, wordIt.current().start());
          }
          aOutput << AEOL;
        }
      }
    }

    lineIt.next();
  }

  return eTrue;
}

}
