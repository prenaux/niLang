#ifndef __WIDGETCOMMAND_15926215_H__
#define __WIDGETCOMMAND_15926215_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetCommand declaration.
class cWidgetCommand : public ni::cIUnknownImpl<ni::iWidgetCommand,ni::eIUnknownImplFlags_Default>
{
  niBeginClass(cWidgetCommand);

 public:
  //! Constructor.
  cWidgetCommand();
  //! Destructor.
  ~cWidgetCommand();

  //! Zeros all the class members.
  void ZeroMembers();
  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  //// ni::iWidgetCommand //////////////////////
  tBool __stdcall Copy(const ni::iWidgetCommand *apSrc);
  ni::iWidgetCommand * __stdcall Clone() const;
  tBool __stdcall SetSender(ni::iWidget *apSender);
  ni::iWidget * __stdcall GetSender() const;
  void __stdcall SetID(tU32 anID);
  tU32 __stdcall GetID() const;
  void __stdcall SetExtra1(const Var &aVar);
  const Var & __stdcall GetExtra1() const;
  void __stdcall SetExtra2(const Var &aVar);
  const Var & __stdcall GetExtra2() const;
  //// ni::iWidgetCommand //////////////////////

 private:
  Ptr<iWidget>  mptrSender;
  tU32        mnID;
  Var         mvarExtra1;
  Var         mvarExtra2;
  niEndClass(cWidgetCommand);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __WIDGETCOMMAND_15926215_H__
