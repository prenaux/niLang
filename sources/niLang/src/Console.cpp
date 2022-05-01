// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include <niLang/ILang.h>
#include "Console.h"
#include "API/niLang/ILang.h"
#include "API/niLang/Utils/StringTokenizerImpl.h"

using namespace ni;

void RegisterConsoleCommands_Global(iConsole* apConsole);

static const tU32 _knMaxQueueSize = 64;
_HDecl(ni);
_HDecl(js);
_HDecl(command);

//////////////////////////////////////////////////////////////////////////////////////////////
// cConsole::sNamespace

///////////////////////////////////////////////
cConsole::sNamespace::sNamespace()
{
  ulRef = 1;
}

///////////////////////////////////////////////
cConsole::sNamespace::~sNamespace()
{
  Clear();
}

///////////////////////////////////////////////
void cConsole::sNamespace::Clear()
{
  mapCommands.clear();
  mapVars.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
cConsole::cConsole()
    : msinkList(tConsoleSinkLst::Create())
{
  ZeroMembers();
  RegisterConsoleCommands_Global(this);
}

///////////////////////////////////////////////
cConsole::~cConsole()
{
  mGlobalNamespace.Clear();
  mmapNamespaces.clear();
}

///////////////////////////////////////////////
void cConsole::ZeroMembers()
{
  mulCompleteCmdLineCount = 0;
}

///////////////////////////////////////////////
tBool __stdcall cConsole::IsOK() const
{
  return eTrue;
}

///////////////////////////////////////////////
tConsoleSinkLst* __stdcall cConsole::GetSinkList() const
{
  return msinkList;
}

///////////////////////////////////////////////
cConsole::sNamespace* cConsole::GetNamespaceAndName(const achar* aszName, cString& strNamespace, cString& strName)
{
  tBool bHasNamespace = eFalse;
  cString strA;
  cString strB;
  const achar* p = aszName;
  while (StrIsSpace(*p)) { StrGetNextX(&p); }
  while (*p && !ni::StrIsSpace(*p))
  {
    const tU32 ch = StrGetNext(p);
    if (ch == '.') {
      bHasNamespace = eTrue;
    }
    else {
      if (!bHasNamespace)
      { // strA will contain the namespace if there is one, else the name
        strA.appendChar(StrToLower(ch));
      }
      else
      { // strB will contain the name if there is a namespace
        strB.appendChar(StrToLower(ch));
      }
    }
    StrGetNextX(&p);
  }

  if (bHasNamespace) {
    strNamespace = strA;
    strName = strB;
  }
  else {
    strNamespace = _A("global");
    strName = strA;
  }

  sNamespace* pNS = GetNamespace(bHasNamespace?strNamespace.Chars():NULL);
  if (!pNS) {
    niWarning(niFmt(_A("Can't find '%s' namespace"),strNamespace.Chars()));
    return NULL;
  }

  return pNS;
}

///////////////////////////////////////////////
cConsole::sNamespace* cConsole::GetNamespace(const achar* aszNamespace)
{
  cString strNamespace = aszNamespace;
  strNamespace.ToLower();

  sNamespace* pNS = NULL;
  if (strNamespace.IsNotEmpty() && !strNamespace.IEq(_A("global"))) {
    __sync_lock();
    tNamespaceMapIt it = mmapNamespaces.find(strNamespace);
    if (it == mmapNamespaces.end())
      return NULL;
    pNS = &it->second;
  }
  else {
    pNS = &mGlobalNamespace;
  }

  return pNS;
}

///////////////////////////////////////////////
//! Add a namespace.
void cConsole::AddNamespace(const achar* aszNamespace)
{
  cString strNamespace = aszNamespace;
  strNamespace.ToLower();

  __sync_lock();
  tNamespaceMapIt it = mmapNamespaces.find(strNamespace);
  if (it != mmapNamespaces.end()) {
    it->second.ulRef++;
    return;
  }

  astl::upsert(mmapNamespaces, strNamespace, sNamespace());
}

///////////////////////////////////////////////
//! Remove a namespace.
tBool cConsole::RemoveNamespace(const achar* aszNamespace)
{
  cString strNamespace = aszNamespace;
  strNamespace.ToLower();

  __sync_lock();
  tNamespaceMapIt it = mmapNamespaces.find(strNamespace);
  if (it == mmapNamespaces.end())
    return eFalse;

  if (--it->second.ulRef <= 0)
    mmapNamespaces.erase(it);

  return eTrue;
}

///////////////////////////////////////////////
//! Add a new string variable.
tBool cConsole::AddVariable(const achar* aszName, const achar* aszValue)
{
  cString strName, strNS;
  sNamespace* pNS = GetNamespaceAndName(aszName, strNS, strName);
  if (!pNS) {
    niWarning(niFmt(_A("Can't get namespace of '%s'."), aszName));
    return eFalse;
  }

  {
    __sync_lock();

    tStringMap::iterator itVar = pNS->mapVars.find(strName);
    if (itVar != pNS->mapVars.end()) {
      niWarning(niFmt(_A("Variable '%s' already exists in namespace '%s'"), strName.Chars(), strNS.Chars()));
      return eFalse;
    }

    astl::upsert(pNS->mapVars, strName, aszValue?aszValue:_A(""));
  }
  return eTrue;
}

///////////////////////////////////////////////
//! Remove a variable.
tBool cConsole::RemoveVariable(const achar* aszName)
{
  cString strName, strNS;
  sNamespace* pNS = GetNamespaceAndName(aszName, strNS, strName);
  if (!pNS) {
    niWarning(niFmt(_A("Can't get namespace of '%s'."), aszName));
    return eFalse;
  }

  {
    __sync_lock();

    tStringMap::iterator itVar = pNS->mapVars.find(strName);
    if (itVar == pNS->mapVars.end()) {
      niWarning(niFmt(_A("Variable '%s' don't exists in namespace '%s'"), strName.Chars(), strNS.Chars()));
      return eFalse;
    }

    pNS->mapVars.erase(itVar);
  }
  return eTrue;
}

///////////////////////////////////////////////
//! Set the string value of a variable.
tBool cConsole::SetVariable(const achar* aszName, const achar* aszValue)
{
  cString strName, strNS;
  sNamespace* pNS = GetNamespaceAndName(aszName, strNS, strName);
  if (!pNS) {
    niWarning(niFmt(_A("Can't get namespace of '%s'."), aszName));
    return eFalse;
  }

  {
    __sync_lock();
    tStringMap::iterator itVar = pNS->mapVars.find(strName);
    if (itVar == pNS->mapVars.end()) {
      niWarning(niFmt(_A("Variable '%s' don't exists in namespace '%s'"), strName.Chars(), strNS.Chars()));
      return eFalse;
    }

    itVar->second = aszValue;

    niCallSinkVoid_(ConsoleSink,msinkList,NamespaceVariableChanged,(itVar->first.Chars(), itVar->second.Chars()));
  }
  return eTrue;
}

///////////////////////////////////////////////
//! Get the string value of a variable.
cString cConsole::GetVariable(const achar* aszName)
{
  cString strName, strNS;
  sNamespace* pNS = GetNamespaceAndName(aszName, strNS, strName);
  if (!pNS) {
    niWarning(niFmt(_A("Can't get namespace of '%s'."), aszName));
    return AZEROSTR;
  }

  {
    __sync_lock();

    tStringMap::iterator itVar = pNS->mapVars.find(strName);
    if (itVar == pNS->mapVars.end()) {
      niWarning(niFmt(_A("Variable '%s' don't exists in namespace '%s'"), strName.Chars(), strNS.Chars()));
      return AZEROSTR;
    }

    return itVar->second.Chars();
  }
}

///////////////////////////////////////////////
tU32 __stdcall cConsole::GetNumCommands() const {
  __sync_lock();
  return (tU32)mCmdQueue.size();
}
tBool __stdcall cConsole::PushCommand(const achar* aszCommand) {
  if (!niStringIsOK(aszCommand))
    return eFalse;

  __sync_lock();
  if (mCmdQueue.size() >= _knMaxQueueSize) {
    niError("Too many commands in the queue.");
    return eFalse;
  }
  mCmdQueue.push_back(aszCommand);
  return eTrue;
}
cString __stdcall cConsole::PopCommand() {
  __sync_lock();
  if (mCmdQueue.empty())
    return "";
  cString r = mCmdQueue.front();
  mCmdQueue.pop_front();
  return r;
}
tU32 __stdcall cConsole::PopAndRunAllCommands() {
  {
    __sync_lock();
    if (mCmdQueue.empty())
      return 0;
  }
  cString cmd;
  tU32 c = 0;
  while (c < _knMaxQueueSize) {
    {
      __sync_lock();
      cmd = mCmdQueue.front();
      mCmdQueue.pop_front();
    }
    this->RunCommand(cmd.Chars());
    {
      __sync_lock();
      if (mCmdQueue.empty())
        break;
      ++c;
    }
  }
  return c;
}

///////////////////////////////////////////////
//! Return eTrue if the given command exists.
tBool cConsole::CommandExists(const achar* aszName)
{
  if (!niIsStringOK(aszName))
    return eFalse;

  cString strName, strNS;
  sNamespace* pNS = GetNamespaceAndName(aszName,strNS,strName);
  if (!pNS)
    return eFalse;

  {
    __sync_lock();
    tCommandMapIt itCmd = pNS->mapCommands.find(strName);
    return !(itCmd == pNS->mapCommands.end());
  }
}

///////////////////////////////////////////////
//! Add a command in the console.
tBool cConsole::AddCommand(iCommandSink* apCmd)
{
  Ptr<iCommandSink> pCmd = apCmd;
  if (!niIsOK(pCmd)) {
    niError(_A("Invalid command interface given."));
    return eFalse;
  }

  sNamespace* pNS = GetNamespace(pCmd->GetNamespace());
  if (!pNS) {
    niError(niFmt(_A("Can't get namespace of '%s'."), pCmd->GetName()));
    return eFalse;
  }

  cString strName = pCmd->GetName();
  strName.ToLower();

  {
    __sync_lock();
    tCommandMapIt itCmd = pNS->mapCommands.find(strName);
    if (itCmd != pNS->mapCommands.end()) {
      niError(niFmt(_A("Command '%s' already exists."), pCmd->GetName()));
      return eFalse;
    }
    astl::upsert(pNS->mapCommands, strName, pCmd);
    return eTrue;
  }
}

///////////////////////////////////////////////
//! Remove a command of the console.
tBool cConsole::RemoveCommand(const achar* aszName)
{
  cString strName, strNS;
  sNamespace* pNS = GetNamespaceAndName(aszName, strNS, strName);
  if (!pNS) {
    niError(niFmt(_A("Can't get namespace of '%s'."), aszName));
    return eFalse;
  }

  {
    __sync_lock();
    tCommandMapIt itCmd = pNS->mapCommands.find(strName);
    if (itCmd == pNS->mapCommands.end()) {
      niError(niFmt(_A("Command '%s' don't exists."), strName.Chars()));
      return eFalse;
    }
    pNS->mapCommands.erase(itCmd);
    return eTrue;
  }
}

///////////////////////////////////////////////
//! Get the description of a command.
cString __stdcall cConsole::GetCommandDescription(const achar* aszCommand) const
{
  cString strName, strNS;
  sNamespace* pNS = const_cast<cConsole*>(this)->GetNamespaceAndName(aszCommand, strNS, strName);
  if (!pNS) {
    niError(niFmt(_A("Can't get namespace of '%s'."), aszCommand));
    return AZEROSTR;
  }

  {
    __sync_lock();
    tCommandMapIt itCmd = pNS->mapCommands.find(strName);
    if (itCmd == pNS->mapCommands.end()) {
      niError(niFmt(_A("Command '%s' don't exists."), strName.Chars()));
      return AZEROSTR;
    }
    return itCmd->second->GetDescription();
  }
}

///////////////////////////////////////////////
//! Run a command.
tBool cConsole::RunCommand(const achar* aszCommand)
{
  tBool sinkRet = eTrue;
  const achar* p = aszCommand;
  while (*p && ni::StrIsSpace(*p)) {
    ++p;
  }

  // Empty command...
  if (!*p)
    return eFalse;

  const tBool isContextOrAq = (*p == ':' || *p == '[');
  if (isContextOrAq) {
    // [SCRIPTINGHOST,CONTEXT]
    tHStringPtr hspHostName, hspContext;
    {
      ++p; // skip the '[' or ':'
      if (*p == ':') {
        ++p; // skip the ':'
        hspHostName = _HC(ni);
        hspContext = _HC(command);
      }
      else {
        cString strHostName, strContext;
        while (*p && *p != ']') {
          strHostName.appendChar(StrGetNextX(&p));
        }
        if (!*p) {
          niError(_A("ScriptingHost command syntax error: ']' expected."));
          return eFalse;
        }
        ++p; // skip the ']'
        if (strHostName.contains(_A(","))) {
          strContext = strHostName.After(_A(","));
          strHostName = strHostName.Before(_A(","));
        }
        if (strHostName.IsEmpty()) {
          hspHostName = _HC(js);
        }
        else {
          hspHostName = _H(strHostName);
        }
        if (strContext.IsEmpty()) {
          hspContext = _HC(command);
        }
        else {
          hspContext = _H(strContext);
        }
      }
    }

#if niMinFeatures(15)
    Ptr<iScriptingHost> ptrHost = ni::GetLang()->GetScriptingHostFromName(hspHostName);
    if (!ptrHost.IsOK()) {
      niError(niFmt(_A("ScriptingHost[%s]: not registered."), hspHostName));
      return eFalse;
    }

    niCallSinkRetTest_(ConsoleSink,msinkList,BeforeRunScript,(ptrHost,p),sinkRet,sinkRet==eTrue);
    if (sinkRet) {
      SetVariable(_A("Result"), NULL);
      SetVariable(_A("Error"), NULL);
      tBool r = ptrHost->EvalString(hspContext,p);
      if (!r) {
        niError(niFmt(_A("ScriptingHost[%s,%s]: command '%s' error."),hspHostName,hspContext,p));
        return eFalse;
      }

      niCallSinkVoid_(ConsoleSink,msinkList,AfterRunScript,(ptrHost,p));
      return eTrue;
    }
    else {
      return eFalse;
    }
#else
    niError("Scripting Host not supported in minimal build.");
    return eFalse;
#endif
  }
  else
  {
    niCallSinkRetTest_(ConsoleSink,msinkList,BeforeRunCommand,(aszCommand),sinkRet,sinkRet==eTrue);
    if (sinkRet) {

      cString strName, strNS;
      sNamespace* pNS = GetNamespaceAndName(aszCommand,strNS,strName);
      if (!pNS) {
        niError(niFmt(_A("Can't get namespace of '%s'."), aszCommand));
        return eFalse;
      }

      Ptr<iCommandSink> cmdSink;
      ni::Ptr<tStringCVec> vArgs = tStringCVec::Create();
      {
        __sync_lock();
        tCommandMapIt itCmd = pNS->mapCommands.find(strName);
        if (itCmd == pNS->mapCommands.end()) {
          niError(niFmt(_A("Command '%s' don't exists."), strName.Chars()));
          return eFalse;
        }

        cString strCmd(aszCommand);
        cCommandLineStringTokenizer tokCmdLine;
        StringTokenize(strCmd,*vArgs,&tokCmdLine);

        // Look in the argument list if there is no variable
        // and if yes replace them by their string value.
        for (tStringCVec::iterator itArg = vArgs->begin(); itArg != vArgs->end(); ++itArg)
        {
          const achar *parg = itArg->c_str();
          if (*(parg++) == '$') {
            if (*parg == '$')
            { // $$ print a $
              *itArg = parg;
            }
            else {
              *itArg = GetVariable(parg);
            }
          }
          else {
            // trim quotes
            *itArg = itArg->TrimEx(_A("\"'"));
          }
        }

        cmdSink = itCmd->second;
        if (!niIsOK(cmdSink)) {
          niError(niFmt(_A("Command '%s' has an invalid sink."), strName.Chars()));
          return eFalse;
        }
      }

      SetVariable(_A("Result"), NULL);
      SetVariable(_A("Error"), NULL);
      tBool ret = cmdSink->OnRun(vArgs,this);
      niCallSinkVoid_(ConsoleSink,msinkList,AfterRunCommand,(p));
      return ret;
    }
    else {
      return eFalse;
    }
  }
}

///////////////////////////////////////////////
//! Complete the given command line.
cString __stdcall cConsole::CompleteCommandLine(const achar* aaszCmd, tBool abNext)
{
  __sync_lock();

  tBool abDollar = eFalse;

  cString strCmdLine = aaszCmd;

  if (!(strCmdLine.IEq(mstrLastCompletedCommand)))
    mulCompleteCmdLineCount = 0;

  mstrLastCompletedCommand = strCmdLine;
  if (strCmdLine.empty())
    return cString();


  astl::vector<cString> vToks;
  cDefaultStringTokenizer tokDefault;
  StringTokenize(strCmdLine, vToks, &tokDefault);

  tU32 count = 0;
  cString strResult;

  if (vToks.size() == 1)
  {
    tSize cmplen = strCmdLine.length();

    for (tCommandMapCIt itCmd = mGlobalNamespace.mapCommands.begin(); itCmd != mGlobalNamespace.mapCommands.end(); ++itCmd)
    {
      if (itCmd->first.nicmp(strCmdLine.Chars(),cmplen) == 0) {
        if (mulCompleteCmdLineCount == count++)
        {
          strResult = itCmd->first;
        }
      }
    }

    for (tNamespaceMapCIt itNS = mmapNamespaces.begin(); itNS != mmapNamespaces.end(); ++itNS)
    {
      if (itNS->first.nicmp(strCmdLine.Chars(),ni::Min(itNS->first.length(),cmplen)) != 0)
        continue;

      for (tCommandMapCIt itCmd = itNS->second.mapCommands.begin(); itCmd != itNS->second.mapCommands.end(); ++itCmd)
      {
        cString str;
        str << itNS->first << _A(".") << itCmd->first;

        if (str.nicmp(strCmdLine.Chars(),cmplen) == 0)
        {
          if (mulCompleteCmdLineCount == count++)
          {
            strResult = str;
          }
        }
      }
    }
  }
  else
  {
    if (vToks.back()[0] == _A('$'))
    {
      strCmdLine = vToks.back().substr(1);
      abDollar = eTrue;
    }
    else
    {
      strCmdLine = vToks.back();
    }
    tSize cmplen = strCmdLine.length();

    for (tStringMap::const_iterator itVar = mGlobalNamespace.mapVars.begin();
         itVar != mGlobalNamespace.mapVars.end(); ++itVar)
    {
      if (itVar->first.nicmp(strCmdLine.Chars(),cmplen) == 0)
      {
        if (mulCompleteCmdLineCount == count++)
        {
          strResult = itVar->first;
        }
      }
    }

    for (tNamespaceMapCIt itNS = mmapNamespaces.begin(); itNS != mmapNamespaces.end(); ++itNS)
    {
      if (itNS->first.nicmp(strCmdLine.Chars(), ni::Min(itNS->first.length(),cmplen)) != 0)
        continue;

      for (tStringMap::const_iterator itVar = itNS->second.mapVars.begin();
           itVar != itNS->second.mapVars.end(); ++itVar)
      {
        cString str;
        str << itNS->first << _A(".") << itVar->first;

        if (str.nicmp(strCmdLine.Chars(), cmplen) == 0)
        {
          if (mulCompleteCmdLineCount == count++)
          {
            strResult = str;
          }
        }
      }
    }
  }

  // A completion has been found
  if (strResult.IsNotEmpty())
  {
    if (vToks.size() == 1)
    {
      strCmdLine = strResult;
    }
    else
    {
      strCmdLine.Clear();
      for (astl::vector<cString>::iterator itT = vToks.begin(); itT+1 != vToks.end(); ++itT) {
        strCmdLine << *itT;
        strCmdLine << _A(" ");
      }
      strCmdLine << (achar*)(abDollar?_A("$"):_A("")) <<  strResult;
    }

    //if (abNext)
    {
      if (++mulCompleteCmdLineCount >= count)
        mulCompleteCmdLineCount = 0;
    }
    //    else
    //    {
    //      if (mulCompleteCmdLineCount == 0)
    //        mulCompleteCmdLineCount = count-1;
    //      else
    //        --mulCompleteCmdLineCount;
    //    }
  }
  else
  {
    return cString();
  }

  return strCmdLine;
}

///////////////////////////////////////////////
namespace ni {

static Ptr<iConsole> _pConsole;

niExportFunc(iConsole*) GetConsole() {
  if (!_pConsole.IsOK()) {
    _pConsole = niNew cConsole();
  }
  return _pConsole.ptr();
}

niExportFunc(iUnknown*) New_niLang_Console(const Var&, const Var&) {
  return GetConsole();
}

}
