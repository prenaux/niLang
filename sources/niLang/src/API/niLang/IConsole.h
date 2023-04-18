#pragma once
#ifndef __ICONSOLE_61424165_H__
#define __ICONSOLE_61424165_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/StringDef.h>
#include <niLang/Utils/SinkList.h>
#include <niLang/Utils/CollectionImpl.h>
#include <niLang/IScriptingHost.h>

namespace ni {

struct iConsole;

/** \addtogroup niLang
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! Command sink.
//! {DispatchWrapper}
struct iCommandSink : public iUnknown
{
  niDeclareInterfaceUUID(iCommandSink,0x4c99779b,0xaedf,0x404c,0x95,0x43,0x70,0xc3,0x78,0x6e,0x6f,0xac)

  //! Name of the command.
  virtual const achar* __stdcall GetName() const = 0;
  //! Namespace of the command.
  virtual const achar* __stdcall GetNamespace() const = 0;
  //! Description of the command
  virtual const achar* __stdcall GetDescription() const = 0;
  //! Run the command.
  //! \param  avArgs are the arguments.
  //! \param  apConsole is the console instance the command is run from.
  //! \return the result of the command execution.
  virtual tBool __stdcall OnRun(const tStringCVec* avArgs, iConsole* apConsole) = 0;
};

//! Macro to help to declare a command.
#define niDeclareCommand(ParentType, Namespace, Name, Desc)             \
  class cCommandSink_##Namespace##Name : public cIUnknownImpl<ni::iCommandSink> \
  {                                                                     \
 public:                                                                \
    cCommandSink_##Namespace##Name(ParentType* pParent) { mpParent = pParent; } \
    const ni::achar* __stdcall GetName() const { return _A(#Name); }    \
    const ni::achar* __stdcall GetNamespace() const { return _A(#Namespace); } \
    const ni::achar* __stdcall GetDescription() const { return Desc; }  \
    ni::tBool __stdcall OnRun(const ni::tStringCVec* avArgs, ni::iConsole* apConsole); \
    ParentType*     __stdcall GetParent()   { return mpParent; }        \
    const ParentType* __stdcall GetParent() const { return mpParent; }  \
 private:                                                               \
    ParentType* mpParent;                                               \
  };

//! Macro to help to declare a command.
#define niDeclareGlobalCommand(Name, Desc)                              \
  class cCommandSink_##Name : public cIUnknownImpl<ni::iCommandSink>    \
  {                                                                     \
 public:                                                                \
    cCommandSink_##Name(ni::iConsole* pParent)  { mpParent = pParent; } \
    const ni::achar* __stdcall GetName() const { return _A(#Name); }    \
    const ni::achar* __stdcall GetNamespace() const { return NULL; }    \
    const ni::achar* __stdcall GetDescription() const { return Desc; }  \
    ni::tBool __stdcall OnRun(const ni::tStringCVec* avArgs, ni::iConsole* apConsole); \
    ni::iConsole*     __stdcall GetParent()   { return mpParent; }      \
    const ni::iConsole* __stdcall GetParent() const { return mpParent; } \
 private:                                                               \
    ni::iConsole* mpParent;                                             \
  };

//! Macro that help to declare the run command.
#define niImplementCommand(Namespace,Name)  ni::tBool __stdcall cCommandSink_##Namespace##Name::OnRun(const ni::tStringCVec* avArgs, ni::iConsole* apConsole)

//! Macro that help to declare the run command.
#define niImplementGlobalCommand(Name)    ni::tBool __stdcall cCommandSink_##Name::OnRun(const ni::tStringCVec* avArgs, ni::iConsole* apConsole)

//! Macro that declare and implement the run command.
#define niCommand(ParentType, Namespace, Name, Desc)  \
  niDeclareCommand(ParentType,Namespace,Name,Desc)    \
  niImplementCommand(Namespace,Name)

//! Macro that declare and implement the run command.
#define niGlobalCommand(Name, Desc)             \
  niDeclareGlobalCommand(Name,Desc)             \
  niImplementGlobalCommand(Name)

//! Macro to create a new instance of a command.
#define niCreateCommandInstance(Namespace,Name,ParentPtr) niNew cCommandSink_##Namespace##Name(ParentPtr)

//! Macro to create a new instance of a command.
#define niCreateGlobalCommandInstance(Name,ConsolePtr)    niNew cCommandSink_##Name(ConsolePtr)

//! Macro to get the number of arguments passed to the command.
#define niCommandNumArgs()  (avArgs?(ni::tU32)avArgs->size():0)
//! Macro to get the argument of a command, returns an empty string if the argument index is invalid.
#define niCommandArg(IDX) ((avArgs!=NULL && IDX<avArgs->size())?(*avArgs)[IDX]:ni::cString())

//! Console sink.
//! {DispatchWrapper}
struct iConsoleSink : public iUnknown
{
  niDeclareInterfaceUUID(iConsoleSink,0x415f0fc4,0x3e03,0x49ea,0xb2,0x1b,0x68,0xa5,0x10,0x82,0x1e,0x19)
  //! Called when a variable value changed
  virtual void __stdcall OnConsoleSink_NamespaceVariableChanged(const achar* aNamespace, const achar* aaszName, const achar* aaszValue) = 0;
  //! Called before a console command is ran.
  //! \return false to disallow the command run
  virtual tBool __stdcall OnConsoleSink_BeforeRunCommand(const achar* aaszCmd) = 0;
  //! Called after a console command is ran.
  virtual void __stdcall OnConsoleSink_AfterRunCommand(const achar* aaszCmd) = 0;
  //! Called before a script command is ran.
  //! \return false to disallow the script run
  virtual tBool __stdcall OnConsoleSink_BeforeRunScript(iScriptingHost* apHost, const achar* aaszCmd) = 0;
  //! Called after a script command is ran.
  virtual void __stdcall OnConsoleSink_AfterRunScript(iScriptingHost* apHost, const achar* aaszCmd) = 0;
};

//! Console sink list.
typedef SinkList<iConsoleSink> tConsoleSinkLst;

//////////////////////////////////////////////////////////////////////////////////////////////
//! Console interface.
struct iConsole : public iUnknown
{
  niDeclareInterfaceUUID(iConsole,0x064c5f0a,0x32bb,0x4a7c,0x9a,0x8d,0xf8,0x8c,0xbd,0xc5,0x13,0x50)

  //! Get the console sink list.
  //! {Property}
  virtual tConsoleSinkLst* __stdcall GetSinkList() const = 0;

  //########################################################################################
  //! \name Namespaces
  //########################################################################################
  //! @{

  //! Add a namespace.
  //! \remark if the namespace already exists add a reference to it.
  virtual void __stdcall AddNamespace(const achar* aszNamespace) = 0;
  //! Remove a namespace. Delete all sinks, commands and variables in the namespace.
  //! \return eFalse if the namespace don't exists, else return eTrue.
  virtual tBool __stdcall RemoveNamespace(const achar* aszNamespace) = 0;
  //! @}

  //########################################################################################
  //! \name Variables
  //########################################################################################
  //! @{

  //! Add a new string variable.
  //! \param  aszName name of the variable to create.
  //! \param  aszValue is the string value to give to the variable.
  //! \return eFalse if the variable already exists or that the namespace is not registered,
  //!     else eTrue.
  virtual tBool __stdcall AddVariable(const achar* aszName, const achar* aszValue = NULL) = 0;
  //! Remove a variable.
  //! \return eFalse if the variable can't be found and so can't be removed, else eTrue.
  virtual tBool __stdcall RemoveVariable(const achar* aszName) = 0;
  //! Set the string value of a variable.
  //! \param  aszName name of the variable to set.
  //! \param  aszValue is the string value to give to the variable.
  //! \return eFalse if the variable don't exists and so can't be set, else eTrue.
  virtual tBool __stdcall SetVariable(const achar* aszName, const achar* aszValue) = 0;
  //! Get the string value of a variable.
  //! \param  aszName name of the variable to get.
  //! \return the value of the variable or an empty string if it doesn't exist
  virtual cString __stdcall GetVariable(const achar* aszName) = 0;
  //! @}

  //########################################################################################
  //! \name Commands
  //########################################################################################
  //! @{

  //! Return eTrue if the given command exists.
  virtual tBool __stdcall CommandExists(const achar* aszName) = 0;
  //! Add a command in the console.
  //! \param  pCmd is the command sink that will be called when the command is runned.
  //! \return eFalse if the command already exists or that the namespace don't exists, else eTrue.
  virtual tBool __stdcall AddCommand(iCommandSink* pCmd) = 0;
  //! Remove a command of the console.
  //! \param  aszName is the name of the command to remove.
  //! \return eFalse if the command can't be found and so can't be removed, else eTrue.
  virtual tBool __stdcall RemoveCommand(const achar* aszName) = 0;
  //! Get the description of a command.
  virtual cString __stdcall GetCommandDescription(const achar* aszCommand) const = 0;
  //! Run a command.
  //! \param  aszCommand is the raw command to parse.
  //! \return the value returned by the Run() method of iCommandSink or
  //!     eFalse if the command don't exists.
  virtual tBool __stdcall RunCommand(const achar* aszCommand) = 0;
  //! Complete the given command line.
  //! \param  aaszCmd is the command line to complete.
  //! \param  abNext if eTrue returns the next match, else the previous one.
  //! \return The completed command line, if no match is found returns an empty string.
  virtual cString __stdcall CompleteCommandLine(const achar* aaszCmd, tBool abNext = eTrue) = 0;
  //! Get the number of commands in the queue.
  virtual tU32 __stdcall GetNumCommands() const = 0;
  //! Queue a command to be run later on.
  //! \param aszCommand is the raw command to parse.
  //! \return eFalse if the command can't be queue, else eTrue.
  virtual tBool __stdcall PushCommand(const achar* aszCommand) = 0;
  //! Pop a command from the queue.
  virtual cString __stdcall PopCommand() = 0;
  //! Pop and run all commands in the queue.
  virtual tU32 __stdcall PopAndRunAllCommands() = 0;
  //! @}
};

niExportFunc(iConsole*) GetConsole();
niExportFunc(iUnknown*) New_niLang_Console(const Var&, const Var&);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ICONSOLE_61424165_H__
