#include "stdafx.h"

#ifdef niWindows

#include <niLang/STL/scope_guard.h>
#include "../../niUI/src/API/niUI/GraphicsEnum.h"
#include "../../niUI/src/GDRV_ShaderConstants.h"
#include "mojoshader/mojoshader.h"
#include "mojoshader/cleanup_pp.h"
#include <niLang/Utils/Trace.h>

using namespace ni;

#include "d3dx9.h"
#pragma comment(lib,"legacy_stdio_definitions.lib")
#pragma comment(lib,"d3dx9.lib")

#define OUTPUT_CONSTANTS 1
#define OUTPUT_D3D 1
#define OUTPUT_GLSL 1
#define OUTPUT_METAL 1

niDeclareModuleTrace_(nicgc,TraceShaderCompilerMacro);
#define _TraceShaderCompilerMacro(FMT) niModuleTrace_(nicgc,TraceShaderCompilerMacro,FMT);

static inline DWORD D3D9_GetD3DXCompilerFlags(tShaderCompilerFlags aFlags, tBool abAssember)
{
  DWORD flags = 0;
#if 0
  // This is for the DX2010 SDK HLSL compiler. The issue is that it creates
  // shaders that result in mojoshader generating black shaders. It could be a
  // problem with the varying parameters mapping, the Position is correct but
  // the color output is always black.
  flags |= D3DXSHADER_OPTIMIZATION_LEVEL3|D3DXSHADER_IEEE_STRICTNESS;
#endif
  if (abAssember) {
    if (niFlagIs(flags,eShaderCompilerFlags_Debug))
      flags |= D3DXSHADER_DEBUG;
    if (niFlagIs(flags,eShaderCompilerFlags_SkipValidation))
      flags |= D3DXSHADER_SKIPVALIDATION;
  }
  else {
    flags |= D3DXSHADER_PACKMATRIX_ROWMAJOR;
    if (niFlagIs(flags,eShaderCompilerFlags_Debug))
      flags |= D3DXSHADER_DEBUG;
    if (niFlagIs(flags,eShaderCompilerFlags_SkipValidation))
      flags |= D3DXSHADER_SKIPVALIDATION;
    if (!abAssember && niFlagIs(flags,eShaderCompilerFlags_SkipOptimization))
      flags |= D3DXSHADER_SKIPOPTIMIZATION;
    //if (!abAssember && niFlagIs(flags,eShaderCompilerFlags_PackMatrixRowMajor))
    //  flags |= D3DXSHADER_PACKMATRIX_ROWMAJOR;
    //if (!abAssember && niFlagIs(flags,eShaderCompilerFlags_PackMatrixColumnMajor))
    //  flags |= D3DXSHADER_PACKMATRIX_COLUMNMAJOR;
    if (!abAssember && niFlagIs(flags,eShaderCompilerFlags_PartialPrecision))
      flags |= D3DXSHADER_PARTIALPRECISION;
    if (!abAssember && niFlagIs(flags,eShaderCompilerFlags_AvoidFlowControl))
      flags |= D3DXSHADER_AVOID_FLOW_CONTROL;
    if (!abAssember && niFlagIs(flags,eShaderCompilerFlags_PreferFlowControl))
      flags |= D3DXSHADER_PREFER_FLOW_CONTROL;
    //if (niFlagIs(flags,eShaderCompilerFlags_Save))
    //flags |= ...;
  }
  return flags;
}

struct sD3DXIncludeImpl : public ID3DXInclude
{
  astl::vector<cString>& _vIncludes;
  sD3DXIncludeImpl(astl::vector<cString>& avIncludes) : _vIncludes(avIncludes) {
    // niDebugFmt(("... %d include dir", _vIncludes.size()));
  }

  HRESULT __stdcall Open(D3DXINCLUDE_TYPE IncludeType,
                         LPCSTR pFileName,
                         LPCVOID pParentData,
                         LPCVOID *ppData,
                         UINT *pBytes)
  {

    Ptr<iFile> ptrFile;

    // Look in the script directories
    {
      cPath path = cString(pFileName).Chars();
      ptrFile = ni::GetLang()->URLOpen(path.GetPath().Chars());
      if (!ptrFile.IsOK()) {
        niLoopr(ri, (ni::tI32)_vIncludes.size()) {
          path.PushDirectory();
          path.AddDirectoryFront(_vIncludes[ri].Chars());
          // niDebugFmt(("... Trying '%s'", path.GetPath()));
          ptrFile = ni::GetLang()->URLOpen(path.GetPath().Chars());
          path.PopDirectory();
          if (ptrFile.IsOK()) {
            break;
          }
        }
      }
      if (!ptrFile.IsOK()) {
        niError(niFmt(_A("Can't find header file '%s'."), pFileName));
        return D3DERR_NOTFOUND;
      }
    }

    *pBytes = (UINT)ptrFile->GetSize();
    *ppData = niMalloc(*pBytes);
    ptrFile->ReadRaw(const_cast<void*>(*ppData),*pBytes);
    ptrFile->Invalidate();
    return S_OK;
  }

  HRESULT __stdcall Close(LPCVOID apData)
  {
    niSafeFree(apData);
    return S_OK;
  }
};

static void InitD3DXMACROS(cString& strCompilerTrace, astl::vector<D3DXMACRO>* apOut, iDataTable* apMacros) {
  D3DXMACRO m;
  if (niIsOK(apMacros)) {
    for (tU32 i = 0; i < apMacros->GetNumProperties(); ++i) {
      m.Name = ni::StrDupModule(apMacros->GetPropertyName(i));
      m.Definition = ni::StrDupModule(apMacros->GetStringFromIndex(i).Chars());
      _TraceShaderCompilerMacro((
          _A("-- MACRO: %s = %s\n"),
          apMacros->GetPropertyName(i),
          apMacros->GetStringFromIndex(i)));
      apOut->push_back(m);
    }
  }
  m.Name = NULL;
  m.Definition = NULL;
  apOut->push_back(m);
}

static void FreeD3DXMACROS(astl::vector<D3DXMACRO>* apOut) {
  if (!apOut->empty()) {
    for (tU32 i = 0; i < apOut->size()-1; ++i) {
      niSafeFree((*apOut)[i].Name);
      niSafeFree((*apOut)[i].Definition);
    }
    apOut->clear();
  }
}

///////////////////////////////////////////////
static cString __stdcall ShaderConstants_ToString(const iShaderConstants* apConstants) {
  if (!niIsOK(apConstants)) {
    return "No constants.";
  }
  ni::cString code;
  code += niFmt("%d constant(s).\n", apConstants->GetNumConstants());
  niLoop(i, apConstants->GetNumConstants()) {
    code += niFmt("- name: %s, index: %d, size: %d, type: %d, hwindex: %d\n",
                  apConstants->GetConstantName(i),
                  i,
                  apConstants->GetConstantSize(i),
                  apConstants->GetConstantType(i),
                  apConstants->GetHwIndex(i));
  }
  return code;
}

///////////////////////////////////////////////
static cString __stdcall Shader_ToD3D(eShaderUnit aUnit, tPtr apDataPtr, tSize anDataSize) {
  const MOJOSHADER_parseData *shaderPD = MOJOSHADER_parse(MOJOSHADER_PROFILE_D3D,
                                                          apDataPtr,
                                                          anDataSize, NULL, 0,
                                                          NULL,
                                                          NULL,
                                                          NULL);
  niDefer { MOJOSHADER_freeParseData(shaderPD); };

  ni::cString code(shaderPD->output, shaderPD->output_len);
  return code;
}

///////////////////////////////////////////////
static cString __stdcall Shader_ToGLSL(eShaderUnit aUnit, tPtr apDataPtr, tSize anDataSize) {
  // Assume standard derivatives & texture lod.
  // TODO: These should be controled by a #ifdef that can be used when
  // compiling the OpenGL shader, that way we could safely load the OpenGL
  // code and only use the preprocessor.
  MOJOSHADER_GLSL_SetStandardDerivatives(true);
  MOJOSHADER_GLSL_SetTextureLod(eMOJOSHADERTextureLod_ARB);

  const MOJOSHADER_parseData *shaderPD = MOJOSHADER_parse(MOJOSHADER_PROFILE_GLSL,
                                                          apDataPtr,
                                                          anDataSize, NULL, 0,
                                                          NULL,
                                                          NULL,
                                                          NULL);
  niDefer { MOJOSHADER_freeParseData(shaderPD); };

  ni::cString preprocessed;
  if (!CleanupPP(preprocessed, shaderPD->output, shaderPD->output_len)) {
    niError(niFmt("Can't cleanup the shader code:\n %s.",shaderPD->output));
    return AZEROSTR;
  }

  return preprocessed;
}

///////////////////////////////////////////////
static cString __stdcall Shader_ToMetal(eShaderUnit aUnit, tPtr apDataPtr, tSize anDataSize) {
  const MOJOSHADER_parseData *shaderPD = MOJOSHADER_parse(MOJOSHADER_PROFILE_METAL,
                                                          apDataPtr,
                                                          anDataSize, NULL, 0,
                                                          NULL,
                                                          NULL,
                                                          NULL);
  niDefer { MOJOSHADER_freeParseData(shaderPD); };

  ni::cString code(shaderPD->output, shaderPD->output_len);
  return code;
}

///////////////////////////////////////////////
static tBool __stdcall Shader_Write(
    iFile* apFile,
    iHString* ahspProgName,
    eShaderUnit aUnit,
    iHString* ahspProfile,
    tPtr apDataPtr,
    tSize anDataSize,
    const iShaderConstants* apConstants,
    tBool abDebug)
{
  niCheckIsOK(apFile,eFalse);
  niCheck(apDataPtr != NULL,eFalse);
  niCheck(anDataSize > 0,eFalse);
  niCheck(aUnit == eShaderUnit_Pixel || aUnit == eShaderUnit_Vertex,eFalse);

  const tBool bHasConstants = niIsOK(apConstants);
  apFile->WriteLE32(kfccD3DShader);
  apFile->WriteLE32(niMakeVersion(1,0,0));
  apFile->BeginWriteBits();
  apFile->WriteBitsPackedU8(aUnit);   // unit
  apFile->WriteBitsString(niHStr(ahspProgName));    // program name
  apFile->WriteBitsString(niHStr(ahspProfile));   // profile
  apFile->WriteBitsPackedU32(anDataSize); // size
  apFile->WriteBit(bHasConstants);
  apFile->EndWriteBits();
  apFile->WriteRaw(apDataPtr,anDataSize);    // shader
  // constants
  if (bHasConstants) {
    ni::GetLang()->SerializeObject(
        apFile,
        const_cast<iShaderConstants*>(apConstants),
        eSerializeMode_WriteRaw,
        NULL);
  }

  if (abDebug) {
#if OUTPUT_CONSTANTS
    // dump the D3D
    apFile->WriteString("//--- CONSTANTS ---\n");
    apFile->WriteString(ShaderConstants_ToString(apConstants).Chars());
#endif

#if OUTPUT_D3D
    // dump the D3D
    apFile->WriteString("//--- D3D BEGIN ---\n");
    apFile->WriteString(Shader_ToD3D(aUnit, apDataPtr, anDataSize).Chars());
#endif

#if OUTPUT_GLSL
    // dump the GLSL
    apFile->WriteString("//--- GLSL BEGIN ---\n");
    apFile->WriteString(Shader_ToGLSL(aUnit, apDataPtr, anDataSize).Chars());
#endif

#if OUTPUT_METAL
    // dump the METAL
    apFile->WriteString("//--- METAL BEGIN ---\n");
    apFile->WriteString(Shader_ToMetal(aUnit, apDataPtr, anDataSize).Chars());
#endif
  }

  return eTrue;
}

///////////////////////////////////////////////
niExportFunc(tBool) Shader_CompileDX9(
    iFile* apInput,
    iFile* apOutput,
    iHString* ahspName,
    eShaderUnit aUnit,
    iHString* ahspProfileName,
    iDataTable* apMacros,
    tShaderCompilerFlags aFlags,
    astl::vector<cString>& avIncludes,
    tBool abDebug)
{
  niCheck(aUnit == eShaderUnit_Pixel || aUnit == eShaderUnit_Vertex, eFalse);
  niCheckIsOK(apInput, eFalse);
  niCheckIsOK(apOutput, eFalse);

  cString strCompilerTrace;
  cString strShader = apInput->ReadString();
  HRESULT hr = S_FALSE;
  ID3DXBuffer* pD3DBufferData = NULL;
  ID3DXConstantTable* pD3DConstTable = NULL;

  {
    ID3DXBuffer* pD3DBufferErrors = NULL;

    // Setup macros
    astl::vector<D3DXMACRO> vMacros;
    D3DXMACRO m;
    {
      switch (aUnit) {
        case eShaderUnit_Pixel: ni::StrDupModule("__EXEC_PIXEL__"); break;
        case eShaderUnit_Vertex: ni::StrDupModule("__EXEC_VERTEX__"); break;
      }
      m.Definition = ni::StrDupModule("1");
      m.Name = ni::StrDupModule(niHStr(ahspProfileName));
      m.Definition = ni::StrDupModule("1");
      vMacros.push_back(m);
      niInfo(niFmt(_A("#MACRO: %s = 1"),ahspProfileName));
    }

    sD3DXIncludeImpl includeHandler(avIncludes);

    // D3DXCompile/Assemble
    InitD3DXMACROS(strCompilerTrace,&vMacros,apMacros);
    if (StrStartsWith(niHStr(ahspProfileName),"asm_")) {
      hr = D3DXAssembleShader(
          strShader.Chars(),
          strShader.Len(),
          vMacros.empty()?NULL:&vMacros[0],
          &includeHandler,
          D3D9_GetD3DXCompilerFlags(aFlags,eTrue),
          &pD3DBufferData,
          &pD3DBufferErrors);
    }
    else if (StrStartsWith(niHStr(ahspProfileName),"hlsl_")) {
      hr = D3DXCompileShader(
          strShader.Chars(),
          strShader.Len(),
          vMacros.empty()?NULL:&vMacros[0],
          &includeHandler,
          "main",
          niHStr(ahspProfileName)+5,
          D3D9_GetD3DXCompilerFlags(aFlags,eFalse),
          &pD3DBufferData,
          &pD3DBufferErrors,
          &pD3DConstTable);
    }
    else {
      niAssertUnreachable("Unreachable code.");
    }
    FreeD3DXMACROS(&vMacros);

    // Check compilation result
    if (FAILED(hr)) {
      cString shaderError;
      shaderError += pD3DBufferErrors?((const cchar*)pD3DBufferErrors->GetBufferPointer()):("UNKNOWN");
      if (shaderError.StartsWith("(")) {
        shaderError = _ASTR(apInput->GetSourcePath()) + shaderError;
      }
      niLog(Raw,shaderError.Chars());
      niError(niFmt("Can't compile shader '%s' (profile:%s).", ahspName, ahspProfileName));
      niSafeRelease(pD3DBufferErrors);
      return eFalse;
    }
    else {
      niSafeRelease(pD3DBufferErrors);
    }
  }

  // Get constants
  Ptr<iShaderConstants> ptrConsts;
  if (pD3DConstTable) {
    ptrConsts = niNew cShaderConstants(4096);
    if (!ptrConsts.IsOK()) {
      niError("Can't create the shader constants.");
      return eFalse;
    }

    D3DXCONSTANTTABLE_DESC tableDesc;
    hr = pD3DConstTable->GetDesc(&tableDesc);
    niAssert(SUCCEEDED(hr));
    for (tU32 i = 0; i < tableDesc.Constants; ++i)
    {
      D3DXCONSTANT_DESC desc;
      D3DXHANDLE h = pD3DConstTable->GetConstant(NULL,i);
      UINT cn = 1;
      pD3DConstTable->GetConstantDesc(h,&desc,&cn);
      switch (desc.RegisterSet) {
        case D3DXRS_FLOAT4: {
          tHStringPtr hspName = _H(_ASZ(desc.Name));
          _TraceShaderCompilerMacro((
              _A("-- Const[%d] '%s' (%d registers): %s.\n"),
              desc.RegisterIndex,
              niHStr(hspName),
              desc.RegisterCount,
              desc.DefaultValue ?
              _ASZ(*(sVec4f*)desc.DefaultValue) :
              _A("NA")));
          tU32 cidx = ptrConsts->AddConstant(
              hspName,eShaderRegisterType_ConstFloat,
              desc.RegisterCount);
          ptrConsts->SetHwIndex(cidx,desc.RegisterIndex);
          if (desc.DefaultValue) {
            ptrConsts->SetFloatPointer(
                cidx,(sVec4f*)desc.DefaultValue,desc.RegisterCount);
          }
          break;
        }
        case D3DXRS_SAMPLER: {
          break;
        }
        case D3DXRS_INT4: {
          niError(niFmt(_A("Register %s: INT4 register not yet supported."),cString(desc.Name).Chars()));
          break;
        }
        case D3DXRS_BOOL: {
          niError(niFmt(_A("Register %s: BOOL register not yet supported."),cString(desc.Name).Chars()));
          break;
        }
      }
    }
    pD3DConstTable->Release();
  }

  if (!strCompilerTrace.empty()) {
    niDebugFmt(("Shader Compiler trace for %s from %s:\n%s\n",
                ahspName, apInput->GetSourcePath(), strCompilerTrace));
  }

  if (!Shader_Write(apOutput,ahspName,aUnit,ahspProfileName,
                    (tPtr)pD3DBufferData->GetBufferPointer(),
                    pD3DBufferData->GetBufferSize(),
                    ptrConsts,abDebug))
  {
    niError(niFmt("Can't write output binary shader '%s' (profile:%s)",
                  ahspName,
                  ahspProfileName));
    return eFalse;
  }

  return eTrue;
}

#endif
