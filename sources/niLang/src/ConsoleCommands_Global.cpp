// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Utils/ThreadImpl.h"
#include "API/niLang/Utils/MemBlock.h"
#include "API/niLang/ILang.h"
#include "Console.h"

using namespace ni;

///////////////////////////////////////////////
niGlobalCommand(Mem, _A("Show the memory usage."))
{
  cString o;
  ni::sVec4i ms;
  ni_mem_get_stats(&ms);
  o.Format(
    "--- MemStats ---\n"
    "- current frame : %d\n"
    "- all/obj allocs: %d / %d\n"
    "- all/obj free: %d / %d\n"
    "- all/obj alloced: %d / %d\n",
    ni::GetLang()->GetFrameNumber(),
    ms.x,ms.z,
    ms.y,ms.w,
    ms.x-ms.y,ms.z-ms.w);
  niLog(Info,o.Chars());
  return eTrue;
}

///////////////////////////////////////////////
niGlobalCommand(ListNamespaces, _A("Show the list of the namespaces."))
{
  ni::AutoThreadLock syncLock(static_cast<cConsole*>(apConsole)->mmutexMutex);

  tSize ulNumNS = static_cast<cConsole*>(apConsole)->mmapNamespaces.size();

  niLog(Info,niFmt(_A("Number of namespace: %d"), ulNumNS));
  for(cConsole::tNamespaceMapIt it =  static_cast<cConsole*>(apConsole)->mmapNamespaces.begin();
      it != static_cast<cConsole*>(apConsole)->mmapNamespaces.end();
      ++it)
  {
    niLog(Info,niFmt(_A("  %s"), it->first.Chars()));
  }
  niLog(Info,_A(""));

  return eTrue;
}

///////////////////////////////////////////////
niGlobalCommand(ListCommands,_A("Show the list of the commands."))
{
  ni::AutoThreadLock syncLock(static_cast<cConsole*>(apConsole)->mmutexMutex);

  if (niCommandNumArgs() >= 2)
  {
    cConsole::sNamespace* pNS = static_cast<cConsole*>(apConsole)->GetNamespace(niCommandArg(1).Chars());
    if (!pNS) {
      niLog(Info,niFmt(_A("Can't find namespace '%s'"), niCommandArg(1).Chars()));
    }
    else {
      niLog(Info,niFmt(_A("%s:"), niCommandArg(1).Chars()));
      for(cConsole::tCommandMapIt itCmd = pNS->mapCommands.begin(); itCmd != pNS->mapCommands.end(); ++itCmd)
      {
        iCommandSink* pSink = itCmd->second;
        niLog(Info,niFmt(_A("  %s - %s"), pSink->GetName(), pSink->GetDescription()));
      }
    }
  }
  else
  {
    cConsole::sNamespace* pNS = &static_cast<cConsole*>(apConsole)->mGlobalNamespace;
    niLog(Info,_A("Global:"));
    cConsole::tCommandMapIt itCmd;
    for(itCmd = pNS->mapCommands.begin(); itCmd != pNS->mapCommands.end(); ++itCmd)
    {
      iCommandSink* pSink = itCmd->second;
      niLog(Info,niFmt(_A("  %s - %s"), pSink->GetName(), pSink->GetDescription()));
    }

    for(cConsole::tNamespaceMapIt it = static_cast<cConsole*>(apConsole)->mmapNamespaces.begin(); it != static_cast<cConsole*>(apConsole)->mmapNamespaces.end(); ++it)
    {
      pNS = &it->second;
      niLog(Info,niFmt(_A("%s:"), it->first.Chars()));
      for(itCmd = pNS->mapCommands.begin(); itCmd != pNS->mapCommands.end(); ++itCmd)
      {
        iCommandSink* pSink = itCmd->second;
        niLog(Info,niFmt(_A("  %s - %s"), pSink->GetName(), pSink->GetDescription()));
      }
    }
  }

  niLog(Info,_A(""));

  return eTrue;
}

///////////////////////////////////////////////
niGlobalCommand(ListVariables,_A("Show the list of the variables."))
{
  ni::AutoThreadLock syncLock(static_cast<cConsole*>(apConsole)->mmutexMutex);

  if (niCommandNumArgs() >= 2)
  {
    cConsole::sNamespace* pNS = static_cast<cConsole*>(apConsole)->GetNamespace(niCommandArg(1).Chars());
    if (!pNS)
    {
      niLog(Info,niFmt(_A("Can't find namespace '%s'"), niCommandArg(1).Chars()));
    }
    else
    {
      niLog(Info,niFmt(_A("%s:"), niCommandArg(1).Chars()));
      for(cConsole::tStringMap::iterator itVar = pNS->mapVars.begin(); itVar != pNS->mapVars.end(); ++itVar) {
        niLog(Info,niFmt(_A("  %s = %s"), itVar->first.Chars(), itVar->second.Chars()));
      }
    }
  }
  else
  {
    cConsole::sNamespace* pNS = &static_cast<cConsole*>(apConsole)->mGlobalNamespace;
    niLog(Info,_A("Global:"));
    cConsole::tStringMap::iterator itVar;
    for (itVar = pNS->mapVars.begin(); itVar != pNS->mapVars.end(); ++itVar) {
      niLog(Info,niFmt(_A("  %s"), itVar->first.Chars()));
    }

    for(cConsole::tNamespaceMapIt it = static_cast<cConsole*>(apConsole)->mmapNamespaces.begin(); it != static_cast<cConsole*>(apConsole)->mmapNamespaces.end(); ++it)
    {
      pNS = &it->second;
      niLog(Info,niFmt(_A("%s:"), it->first.Chars()));
      for(itVar = pNS->mapVars.begin(); itVar != pNS->mapVars.end(); ++itVar)
      {
        niLog(Info,niFmt(_A("  %s = %s"), itVar->first.Chars(), itVar->second.Chars()));
      }
    }
  }
  niLog(Info,_A(""));

  return eTrue;
}

#if niMinFeatures(15)
///////////////////////////////////////////////
niGlobalCommand(ListScriptingHosts,_A("Show the list of the registered Scripting Hosts."))
{
  niLog(Info,_A("Registered Scripting Hosts:"));
  niLoop(i,ni::GetLang()->GetNumScriptingHosts()) {
    tHStringPtr hspName = ni::GetLang()->GetScriptingHostName(i);
    iScriptingHost* pHost = ni::GetLang()->GetScriptingHost(i);
    if (pHost) {
      niLog(Info,niFmt(_A("  %s = 0x%x"), niHStr(hspName), pHost));
      Nonnull<tUUIDCVec> uuidLst(tUUIDCVec::Create());
      pHost->ListInterfaces(uuidLst,0);
      tBool first = eTrue;
      cString itfs = _A("  - interfaces: ");
      niLoop(i,uuidLst->size()) {
        const tUUID& IID = uuidLst->at(i);
        iHString* hspIntfName = ni::GetLang()->GetInterfaceName(IID);
        if (!first) itfs << _A(",");
        else        first = eFalse;
        itfs << niFmt(_A("%s{%s}"),niHStr(hspIntfName),_ASZ(IID));
      }
      niLog(Info,itfs.Chars());
    }
  }

  niLog(Info,_A(""));
  return eTrue;
}
#endif

///////////////////////////////////////////////
niGlobalCommand(Quit,_A("Try to quit the application."))
{
  QPtr<iOSWindow> ptrWindow = ni::GetLang()->GetGlobalInstance("iOSWindow");
  if (ptrWindow.IsOK()) {
    ptrWindow->TryClose();
  }
  return eTrue;
}

///////////////////////////////////////////////
niGlobalCommand(Set,_A("Set the value of a variable."))
{
  if(niCommandNumArgs() < 3)
  {
    niLog(Info,_A(""));
    niLog(Info,_A("syntax:"));
    niLog(Info,niFmt(_A("    %s variable_name variable_value"), niCommandArg(0).Chars()));
    niLog(Info,_A(""));
  }
  else
  {
    if (!apConsole->SetVariable(niCommandArg(1).Chars(), niCommandArg(2).GetWithoutBEQuote().Chars()))
      niLog(Info,niFmt(_A("Can't set variable '%s'."), niCommandArg(1).Chars()));
  }

  return eTrue;
}

///////////////////////////////////////////////
niGlobalCommand(Help,_A("List commands and output command descriptions."))
{
  if(niCommandNumArgs() > 1) {
    cString strDesc = apConsole->GetCommandDescription(niCommandArg(1).Chars());
    niLog(Info,strDesc.Chars());
  }
  else
  {
    ni::AutoThreadLock syncLock(static_cast<cConsole*>(apConsole)->mmutexMutex);

    cConsole::sNamespace* pNS = &static_cast<cConsole*>(apConsole)->mGlobalNamespace;
    niLog(Info,_A("Global:"));
    cConsole::tCommandMapIt itCmd;
    for(itCmd = pNS->mapCommands.begin(); itCmd != pNS->mapCommands.end(); ++itCmd)
    {
      iCommandSink* pSink = itCmd->second;
      niLog(Info,niFmt(_A("  %s - %s"), pSink->GetName(), pSink->GetDescription()));
    }

    for(cConsole::tNamespaceMapIt it = static_cast<cConsole*>(apConsole)->mmapNamespaces.begin(); it != static_cast<cConsole*>(apConsole)->mmapNamespaces.end(); ++it)
    {
      pNS = &it->second;
      niLog(Info,niFmt(_A("%s:"), it->first.Chars()));
      for(itCmd = pNS->mapCommands.begin(); itCmd != pNS->mapCommands.end(); ++itCmd)
      {
        iCommandSink* pSink = itCmd->second;
        niLog(Info,niFmt(_A("  %s - %s"), pSink->GetName(), pSink->GetDescription()));
      }
    }
  }

  return eTrue;
}

///////////////////////////////////////////////
niGlobalCommand(CommandDescription,_A("Show the description of a command."))
{
  if (niCommandNumArgs() < 2) {
    niLog(Info,niFmt(_A("syntax:\n    %s [Command]"), niCommandArg(0).Chars()));
    return eTrue;
  }

  cString strDesc = apConsole->GetCommandDescription(niCommandArg(1).Chars());
  niLog(Info,strDesc.Chars());

  return eTrue;
}

///////////////////////////////////////////////
void RegisterConsoleCommands_Global(iConsole* apConsole)
{
  // Register the Result and Error variables.
  apConsole->AddVariable(_A("result"));
  apConsole->AddVariable(_A("error"));

  // Register commands
  apConsole->AddCommand(niCreateGlobalCommandInstance(Mem,apConsole));
  apConsole->AddCommand(niCreateGlobalCommandInstance(CommandDescription,apConsole));
  apConsole->AddCommand(niCreateGlobalCommandInstance(Help,apConsole));
  apConsole->AddCommand(niCreateGlobalCommandInstance(ListCommands,apConsole));
  apConsole->AddCommand(niCreateGlobalCommandInstance(ListNamespaces,apConsole));
  apConsole->AddCommand(niCreateGlobalCommandInstance(ListVariables,apConsole));
#if niMinFeatures(15)
  apConsole->AddCommand(niCreateGlobalCommandInstance(ListScriptingHosts,apConsole));
#endif
  apConsole->AddCommand(niCreateGlobalCommandInstance(Quit,apConsole));
  apConsole->AddCommand(niCreateGlobalCommandInstance(Set,apConsole));
}
