#ifndef __STATESUTILS_6134573_H__
#define __STATESUTILS_6134573_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */
/** \addtogroup niUI_Utils
 * @{
 */

struct sGraphicsAllStates {
  iGraphicsContext* mpContext;
  sRecti mViewport;
  sRecti mScissor;
  sMatrixf mmtxCamView;
  sMatrixf mmtxCamProj;
  sMatrixf mmtxView;
  sMatrixf mmtxProj;

  sGraphicsAllStates(iGraphicsContext* apContext) {
    mpContext = apContext;
    if (mpContext)
      _Save(mpContext);
  }
  ~sGraphicsAllStates() {
    if (mpContext)
      _Restore(mpContext);
  }

  void _Save(iGraphicsContext* apContext) {
    mViewport = apContext->GetViewport();
    mScissor = apContext->GetScissorRect();
    iFixedStates* fs = apContext->GetFixedStates();
    if (fs) {
      mmtxCamView = fs->GetCameraViewMatrix();
      mmtxCamProj = fs->GetCameraProjectionMatrix();
      mmtxView = fs->GetViewMatrix();
      mmtxProj = fs->GetProjectionMatrix();
    }
  }
  void _Restore(iGraphicsContext* apContext) {
    iFixedStates* fs = apContext->GetFixedStates();
    if (fs) {
      fs->SetOnlyCameraViewMatrix(mmtxCamView);
      fs->SetOnlyCameraProjectionMatrix(mmtxCamProj);
      fs->SetViewMatrix(mmtxView);
      fs->SetProjectionMatrix(mmtxProj);
    }
    apContext->SetViewport(mViewport);
    apContext->SetScissorRect(mScissor);
  }
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __STATESUTILS_6134573_H__
