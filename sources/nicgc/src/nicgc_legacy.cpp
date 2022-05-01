#include "stdafx.h"
#include <niLang/Utils/CmdLine.h>

using namespace ni;

#include "../../niUI/src/API/niUI/GraphicsEnum.h"
#include "../../niUI/src/GDRV_ShaderConstants.h"

#ifdef niWindows
niExportFunc(tBool) Shader_CompileDX9(
    iFile* apProgram,
    iFile* apOutput,
    iHString* ahspName,
    eShaderUnit aUnit,
    iHString* ahspProfileName,
    iDataTable* apMacros,
    tShaderCompilerFlags aFlags,
    astl::vector<cString>& avIncludes,
    tBool abDebug);
#endif

static const achar* _aszVersion = _A("5.0");

Ptr<iFile> __stdcall RemoteCompileShader(const achar* aServerUrl, const achar* aaszProfile, const achar* aaszName, const achar* aaszPPPath);

struct sLegacyOptions {
  tBool _bDebugInfos;
  tHStringPtr _hspShaderProfile;
  tHStringPtr _hspShaderName;
  eShaderUnit _shaderUnit;
  Ptr<iFile> _fpInput;
  cString    _strOutput;
  cString    _strServerUrl;
  cString    _strCacheDir;
  astl::vector<cString> _vIncludes;
  Ptr<iDataTable> _ptrDefines;

  sLegacyOptions() {
    _bDebugInfos = eFalse;
    _ptrDefines = ni::CreateDataTable("Defines");
  }
  ~sLegacyOptions() {
    Invalidate();
  }
  void Invalidate() {
    _vIncludes.clear();
    _ptrDefines = ni::CreateDataTable("Defines");
  }
};

cString GetVersionString() {
  return niFmt("Ni Shader Compiler Version %s\n"
               "Build %s, built on %s at %s\n"
               "%s\n",
               _aszVersion,
               niBuildType,
               __DATE__,
               __TIME__,
               niCopyright);
}

cString GetHelpString() {
  cString strHelp =
      _A("usage:\n")
      _A("    nicgc [ option... ] shaderProfile shaderName input output\n")
      _A("\n")
      _A("Available options are:\n")
      _A("   -V             \t displays the version number alone\n")
      _A("   -v             \t displays the version infos\n")
      _A("   -h             \t prints help\n")
      _A("   -d             \t generates debug infos\n")
      _A("   -I directory   \t add an include directory for the preprocessor\n")
      _A("   -D pname=value \t add a predefined value for the preprocessor\n")
      _A("   -C directory   \t compiled shaders cache directory\n")
      _A("   -S serverUrl   \t shader compiler server url\n")
      _A("\n")
      ;
  return strHelp;
}

void InfoExit(const achar* aszMsg) {
  cString str;
  str += aszMsg;
  niPrintln(str.Chars());
  ni::GetLang()->Exit(0);
}

void ErrorHelp(const achar* aszMsg) {
  cString str;
  str += GetVersionString();
  str += _A("\n");
  str += _A("E/");
  str += aszMsg;
  str += _A("\n");
  str += GetHelpString();
  str += _A("\n");
  niPrintln(str.Chars());
  ni::GetLang()->Exit(0x99);
}

void ErrorExit(const achar* aszMsg) {
  cString str;
  str = _A("E/");
  str += aszMsg;
  str += _A("\n");
  niPrintln(str.Chars());
  ni::GetLang()->Exit(0x99);
}

void Warning(const achar* aszMsg) {
  cString str;
  str = _A("W/");
  str += aszMsg;
  str += _A("\n");
  niPrintln(str.Chars());
}

tBool parseCommandLine(sLegacyOptions& aOptions, const achar* aaszCmdLine) {
  TRACE_CMD_LINE(("D/parseCommandLine: %s", aaszCmdLine));

  cString strCmdLine = aaszCmdLine;
  tI32 parametersPos = ni::CmdLineGetParametersPos(strCmdLine);
  TRACE_CMD_LINE(("D/parametersPos: %d", parametersPos));
  if (parametersPos <= 0) {
    ErrorHelp("No parameters.");
  }

  StrCharIt it = strCmdLine.charZIt(parametersPos);
  TRACE_CMD_LINE(("D/parseCommandLine:params: %s", it.current()));

  // Read the VM arguments
  tU32 prevChar = 0;
  while (!it.is_end()) {
    const tU32 c = it.next();
    if (prevChar == '-') {
      switch (c) {
        case 'v': {
          cString msg;
          msg << GetVersionString();
          InfoExit(msg.Chars());
          break;
        }
        case 'V': {
          InfoExit(_aszVersion);
          break;
        }
        case '?':
        case 'h': {
          cString msg;
          msg << GetVersionString();
          msg << "\n";
          msg << GetHelpString();
          InfoExit(msg.Chars());
          break;
        }
        case 'd':
          TRACE_CMD_LINE(("D/debug infos"));
          aOptions._bDebugInfos = ni::eTrue;
          break;
        case '-': // -- is a synonym for -D
        case 'D': {
          cString pname = ni::CmdLineStrCharItReadFile(it,'=');
          cString pvalue = ni::CmdLineStrCharItReadFile(it);
          TRACE_CMD_LINE(("D/add define: %s = %s",pname,pvalue));
          aOptions._ptrDefines->SetString(pname.Chars(),pvalue.Chars());
          break;
        }
        case 'I': {
          cString incl = ni::CmdLineStrCharItReadFile(it,0);
          TRACE_CMD_LINE(("D/add include: %s",incl));
          aOptions._vIncludes.push_back(incl);
          break;
        }
        case 'S': {
          aOptions._strServerUrl = ni::CmdLineStrCharItReadFile(it,0);
          break;
        }
        case 'C': {
          aOptions._strCacheDir = ni::CmdLineStrCharItReadFile(it,0);
          break;
        }
        default: {
          ErrorHelp(niFmt("Unknown option -%c",c));
        }
      }
      prevChar = 0;
    }
    else if (StrIsSpace(c)) {
      continue;
    }
    else  {
      prevChar = c;
      if (prevChar != '-') {
        it.prior();
        break;
      }
    }
  }

  // read the shader profile
  {
    aOptions._hspShaderProfile = _H(ni::CmdLineStrCharItReadFile(it));
    if (HStringIsEmpty(aOptions._hspShaderProfile)) {
      ErrorHelp("No shader profile specified.");
    }
    if (StrEq(niHStr(aOptions._hspShaderProfile), "hlsl_ps_3_0")) {
      aOptions._shaderUnit = eShaderUnit_Pixel;
    }
    else if (StrEq(niHStr(aOptions._hspShaderProfile), "hlsl_vs_3_0")) {
      aOptions._shaderUnit = eShaderUnit_Vertex;
    }
    else {
      ErrorHelp(niFmt("Unknown shader profile '%s'.", aOptions._hspShaderProfile));
    }
    aOptions._ptrDefines->SetString(niHStr(aOptions._hspShaderProfile), "1");
    TRACE_CMD_LINE(("D/_hspShaderProfile: %s", aOptions._hspShaderProfile));
  }

  {
    // read the shader name
    {
      aOptions._hspShaderName = _H(ni::CmdLineStrCharItReadFile(it));
      if (HStringIsEmpty(aOptions._hspShaderName)) {
        ErrorHelp("No shader name specified.");
      }
      TRACE_CMD_LINE(("D/_hspShaderName: %s", aOptions._hspShaderName));
    }

    // read the input shader
    {
      cString input = ni::CmdLineStrCharItReadFile(it);
      if (input.empty()) {
        ErrorHelp("No input shader file specified.");
      }
      aOptions._fpInput = ni::GetRootFS()->FileOpen(input.Chars(),eFileOpenMode_Read);
      if (!aOptions._fpInput.IsOK()) {
        ErrorHelp(niFmt("Can't open input file '%s'.", input));
      }
      TRACE_CMD_LINE(("D/input: %s", input));
    }

    // read the output file
    {
      cString output = ni::CmdLineStrCharItReadFile(it);
      if (output.empty()) {
        ErrorHelp("No output shader file specified.");
      }
      aOptions._strOutput = output;
      TRACE_CMD_LINE(("D/output: %s", output));
    }
  }

  return eTrue;
}

int legacy_main(const achar* aCmdLine) {
  // Init the options
  sLegacyOptions options;
  if (!parseCommandLine(options, aCmdLine)) {
    ErrorHelp("Invalid Command Line.");
  }

  if (options._strServerUrl.IsNotEmpty()) {
    Ptr<iFile> fpCompiled = RemoteCompileShader(options._strServerUrl.Chars(),
                                                niHStr(options._hspShaderProfile),
                                                niHStr(options._hspShaderName),
                                                options._fpInput->GetSourcePath());
    if (!fpCompiled.IsOK()) {
      niError(niFmt("Can't remote compile shader '%s'.", options._hspShaderName));
      return 1;
    }

    Ptr<iFile> fpOutput = ni::GetRootFS()->FileOpen(options._strOutput.Chars(),eFileOpenMode_Write);
    if (!fpOutput.IsOK()) {
      ErrorHelp(niFmt("Can't open output file '%s'.", options._strOutput));
    }
    fpCompiled->SeekSet(0);
    fpOutput->WriteFile(fpCompiled->GetFileBase());
    niLog(Info,niFmt("Remote compiled shader '%s' to '%s'", options._hspShaderName, options._strOutput));
  }
  else {
#ifdef niWindows
    Ptr<iFile> fpOutputMem = ni::CreateFileDynamicMemory(0,"");
    if (!Shader_CompileDX9(
          options._fpInput,
          fpOutputMem,
          options._hspShaderName,
          options._shaderUnit,
          options._hspShaderProfile,
          options._ptrDefines,
          0,
          options._vIncludes,
          options._bDebugInfos))
    {
      niError("Shader compilation failed.");
      return 1;
    }

    fpOutputMem->SeekSet(0);
    Ptr<iFile> fpOutput = ni::GetRootFS()->FileOpen(options._strOutput.Chars(),eFileOpenMode_Write);
    if (!fpOutput.IsOK()) {
      ErrorHelp(niFmt("Can't open output file '%s'.", options._strOutput));
    }
    fpOutput->WriteFile(fpOutputMem->GetFileBase());
    niLog(Info,niFmt("Locally compiled shader '%s' to '%s'", options._hspShaderName, options._strOutput));
#else
    niError("No local shader compiler found.");
    return 1;
#endif
  }

  return 0;
}
