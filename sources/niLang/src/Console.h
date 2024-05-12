#ifndef __CONSOLE_22888240_H__
#define __CONSOLE_22888240_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Utils/Sync.h>
#include <niLang/STL/deque.h>
#include <niLang/Utils/UnknownImpl.h>
#include "API/niLang/IConsole.h"
#include "API/niLang/Utils/StringTokenizerImpl.h"

class cCommandSink_ListNamespaces;
class cCommandSink_ListCommands;
class cCommandSink_ListVariables;
class cCommandSink_Help;

namespace ni {

//////////////////////////////////////////////////////////////////////////////////////////////
class cConsole : public ImplRC<iConsole>
{
 public:
  // Namespace
  typedef astl::map<cString,Ptr<iCommandSink> > tCommandMap;
  typedef tCommandMap::iterator     tCommandMapIt;
  typedef tCommandMap::const_iterator   tCommandMapCIt;

  typedef astl::map<cString,cString> tStringMap;

  struct sNamespace
  {
    tStringMap      mapVars;
    tCommandMap     mapCommands;
    tU32        ulRef;
    sNamespace();
    ~sNamespace();
    void Clear();
  };
  typedef astl::map<cString,sNamespace> tNamespaceMap;
  typedef tNamespaceMap::iterator     tNamespaceMapIt;
  typedef tNamespaceMap::const_iterator tNamespaceMapCIt;

  sNamespace* GetNamespace(const achar* aszNamespace);
  sNamespace* GetNamespaceAndName(const achar* aszName, cString& strNamespace, cString& strName);

  void RegisterCommands();

 public:
  cConsole();
  ~cConsole();

  void ZeroMembers();

  //// iUnknown /////////////////////////////////
  tBool __stdcall IsOK() const;
  //// iUnknown /////////////////////////////////

  //// iConsole /////////////////////////////////
  //! Get the console sink list.
  tConsoleSinkLst* __stdcall GetSinkList() const;
  void __stdcall AddNamespace(const achar* aszNamespace);
  tBool __stdcall RemoveNamespace(const achar* aszNamespace);
  tBool __stdcall AddVariable(const achar* aszName, const achar* aszValue = NULL);
  tBool __stdcall RemoveVariable(const achar* aszName);
  tBool __stdcall SetVariable(const achar* aszName, const achar* aszValue);
  cString __stdcall GetVariable(const achar* aszName);

  tBool __stdcall CommandExists(const achar* aszName);
  tBool __stdcall AddCommand(iCommandSink* pCmd);
  tBool __stdcall RemoveCommand(const achar* aszName);
  cString __stdcall GetCommandDescription(const achar* aszCommand) const;
  tBool __stdcall RunCommand(const achar* aszCommand);
  cString __stdcall CompleteCommandLine(const achar* aaszCmd, tBool abNext);

  tU32 __stdcall GetNumCommands() const;
  tBool __stdcall PushCommand(const achar* aszCommand);
  cString __stdcall PopCommand();
  tU32 __stdcall PopAndRunAllCommands();
  //// iConsole /////////////////////////////////

 private:
  friend class ::cCommandSink_ListNamespaces;
  friend class ::cCommandSink_ListCommands;
  friend class ::cCommandSink_ListVariables;
  friend class ::cCommandSink_Help;

  __sync_mutex();

  tNamespaceMap mmapNamespaces;
  sNamespace    mGlobalNamespace;

  tU32 mulCompleteCmdLineCount;
  cString mstrLastCompletedCommand;

  Nonnull<tConsoleSinkLst> msinkList;

  typedef astl::deque<cString> tCmdQueue;
  tCmdQueue mCmdQueue;
};

} // end of namespace ni
/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __CONSOLE_22888240_H__
