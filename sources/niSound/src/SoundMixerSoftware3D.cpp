// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#define SPEED_OF_SOUND  340.0f    /* same as A3D */
#define Check3D(CHAN,RET) if (CHAN < mnFirst3DChannel || CHAN > mnLast3DChannel) return RET;

//////////////////////////////////////////////////////////////////////////////////////////////
class cSoundMixerSoftware3D : public ImplRC<iSoundMixer3D,eImplFlags_Default>
{
  niBeginClass(cSoundMixerSoftware3D);

  tU32          mnNumAudioChannels;
  tU32          mnLast3DChannel;
  tU32          mnFirst3DChannel;
  Ptr<iSoundMixer>  mptrBase;
  struct sChannel {
    eSoundMode  mode;
    sVec3f pos;
    sVec3f vel;
    tF32    min_dist;
    tF32    max_dist;
    tF32    cone_inner;
    tF32    cone_outer;
    sVec3f cone_dir;
    tU32    cone_outer_volume;
  } *chan;

 public:
  cSoundMixerSoftware3D() {
    ZeroMembers();
  }
  ~cSoundMixerSoftware3D() {
    Close();
  }

  void ZeroMembers() {
    mnFirst3DChannel = 0;
    chan = NULL;
  }

  tBool __stdcall IsOK() const {
    niClassIsOK(cSoundMixerSoftware3D);
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool Initialize(iSoundMixer* apBase, tU32 anNum3DChannels, tU32 anNumAudioChannels) {
    niCheckIsOK(apBase,eFalse);
    niCheck(anNum3DChannels > 0 && anNum3DChannels <= 1024,eFalse);
    niCheck(apBase->GetNumChannels() > anNum3DChannels,eFalse);
    mnNumAudioChannels = anNumAudioChannels;
    mptrBase = apBase;
    mnFirst3DChannel = apBase->GetNumChannels()-anNum3DChannels;
    mnLast3DChannel = apBase->GetNumChannels()-1;
    chan = niTMalloc(sChannel,anNum3DChannels);
    memset(chan,0,sizeof(sChannel)*anNum3DChannels);
    return eTrue;
  }

  ///////////////////////////////////////////////
  void Close() {
    mptrBase = NULL;
    niSafeFree(chan);
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetFirst3DChannel() const {
    return mnFirst3DChannel;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall GetIsChannel3D(tU32 nchan) const {
    Check3D(nchan,eFalse);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetChannelMode(tU32 nchan, eSoundMode aMode) {
    Check3D(nchan,;);
    nchan -= mnFirst3DChannel;
    chan[nchan].mode = aMode;
  }
  virtual eSoundMode __stdcall GetChannelMode(tU32 nchan) const {
    Check3D(nchan,eSoundMode_Normal2D);
    nchan -= mnFirst3DChannel;
    return chan[nchan].mode;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetChannelPosition(tU32 nchan, const sVec3f& avPosition) {
    Check3D(nchan,;);
    nchan -= mnFirst3DChannel;
    chan[nchan].pos = avPosition;
  }
  virtual sVec3f __stdcall GetChannelPosition(tU32 nchan) {
    Check3D(nchan,sVec3f::Zero());
    nchan -= mnFirst3DChannel;
    return chan[nchan].pos;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetChannelVelocity(tU32 nchan, const sVec3f& avVelocity) {
    Check3D(nchan,;);
    nchan -= mnFirst3DChannel;
    chan[nchan].vel = avVelocity;
  }
  virtual sVec3f __stdcall GetChannelVelocity(tU32 nchan) {
    Check3D(nchan,sVec3f::Zero());
    nchan -= mnFirst3DChannel;
    return chan[nchan].vel;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetChannelMinDistance(tU32 nchan, tF32 afMinDistance) {
    Check3D(nchan,;);
    nchan -= mnFirst3DChannel;
    chan[nchan].min_dist = afMinDistance;
  }
  virtual tF32 __stdcall GetChannelMinDistance(tU32 nchan) const {
    Check3D(nchan,0.0f);
    nchan -= mnFirst3DChannel;
    return chan[nchan].min_dist;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetChannelMaxDistance(tU32 nchan, tF32 afMaxDistance) {
    Check3D(nchan,;);
    nchan -= mnFirst3DChannel;
    chan[nchan].max_dist = afMaxDistance;
  }
  virtual tF32 __stdcall GetChannelMaxDistance(tU32 nchan) const {
    Check3D(nchan,0.0f);
    nchan -= mnFirst3DChannel;
    return chan[nchan].max_dist;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetChannelConeInner(tU32 nchan, tF32 afInsideAngle) {
    Check3D(nchan,;);
    nchan -= mnFirst3DChannel;
    chan[nchan].cone_inner = afInsideAngle;
  }
  virtual tF32 __stdcall GetChannelConeInner(tU32 nchan) const {
    Check3D(nchan,0.0f);
    nchan -= mnFirst3DChannel;
    return chan[nchan].cone_inner;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetChannelConeOuter(tU32 nchan, tF32 afOutsideAngle) {
    Check3D(nchan,;);
    nchan -= mnFirst3DChannel;
    chan[nchan].cone_outer = afOutsideAngle;
  }
  virtual tF32 __stdcall GetChannelConeOuter(tU32 nchan) const {
    Check3D(nchan,0.0f);
    nchan -= mnFirst3DChannel;
    return chan[nchan].cone_outer;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetChannelConeOuterVolume(tU32 nchan, tU32 anOuterVolume) {
    Check3D(nchan,;);
    nchan -= mnFirst3DChannel;
    chan[nchan].cone_outer_volume = anOuterVolume;
  }
  virtual tU32 __stdcall GetChannelConeOuterVolume(tU32 nchan) const {
    Check3D(nchan,0);
    nchan -= mnFirst3DChannel;
    return chan[nchan].cone_outer_volume;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetChannelConeDirection(tU32 nchan, const sVec3f& avDir) {
    Check3D(nchan,;);
    nchan -= mnFirst3DChannel;
    chan[nchan].cone_dir = avDir;
  }
  virtual sVec3f __stdcall GetChannelConeDirection(tU32 nchan) const {
    Check3D(nchan,sVec3f::ZAxis());
    nchan -= mnFirst3DChannel;
    return chan[nchan].cone_dir;
  }

  //--------------------------------------------------------------------------------------------
  //
  //  Listener
  //
  //--------------------------------------------------------------------------------------------
  struct sListener {
    sListener() {
      pos = sVec3f::Zero();
      vel = sVec3f::Zero();
      fwd = sVec3f::ZAxis();
      up = sVec3f::YAxis();
      right = sVec3f::XAxis();
      distance_scale = 1.0f;
      rolloff_scale = 1.0f;
      doppler_scale = 1.0f;
    }
    void Update() {
      VecNormalize(vel);
      VecNormalize(fwd);
      VecNormalize(up);
      VecCross(right,up,fwd);
    }
    sVec3f pos;
    sVec3f vel;
    sVec3f fwd;
    sVec3f up;
    sVec3f right;
    tF32    distance_scale;
    tF32    rolloff_scale;
    tF32    doppler_scale;
  };
  sListener mListener;

  ///////////////////////////////////////////////
  virtual void __stdcall SetListenerPosition(const sVec3f& aV) {
    mListener.pos = aV;
  }
  virtual sVec3f __stdcall GetListenerPosition() const {
    return mListener.pos;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetListenerVelocity(const sVec3f& aV) {
    mListener.vel = aV;
  }
  virtual sVec3f __stdcall GetListenerVelocity() const {
    return mListener.vel;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetListenerForward(const sVec3f& aV) {
    mListener.fwd = aV;
  }
  virtual sVec3f __stdcall GetListenerForward() const {
    return mListener.fwd;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetListenerUp(const sVec3f& aV) {
    mListener.up = aV;
  }
  virtual sVec3f __stdcall GetListenerUp() const {
    return mListener.up;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetListenerDistanceScale(tF32 afScale) {
    mListener.distance_scale = ni::Clamp(afScale,1/100000.0f,100000.0f);
  }
  virtual tF32 __stdcall GetListenerDistanceScale() const {
    return mListener.distance_scale;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetListenerRolloffScale(tF32 afScale) {
    mListener.rolloff_scale = afScale;
  }
  virtual tF32 __stdcall GetListenerRolloffScale() const {
    return mListener.rolloff_scale;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetListenerDopplerScale(tF32 afScale) {
    mListener.doppler_scale = afScale;
  }
  virtual tF32 __stdcall GetListenerDopplerScale() const {
    return mListener.doppler_scale;
  }

  //--------------------------------------------------------------------------------------------
  //
  //  3D Mixer
  //
  //--------------------------------------------------------------------------------------------

  void Compute3D(tI32& anOutVolume, // volume, is a scale of the current absolute volume
                 tI32& anOutPan,    // panning...
                 tF32& afOutPitch,  // pitch, scale of the frequency
                 const sChannel& c,
                 const sListener& l,
                 const tF32 afPanSeparation = 1.0f,  // 1 is full pan separation, 0 is mono
                 const tF32 afEpsilon = niEpsilon5)
  {
    tI32  currpan, vol3d;
    tF32  pitch3d;

    vol3d   = 0;
    pitch3d = 1.0f;
    currpan = 128;

    sVec3f currdiff;
    tF32        val, distance;

    sVec3f pos = c.pos;
    if (c.mode != eSoundMode_Relative3D) {
      pos -= l.pos;
    }

    // Distance between emitter and listener this frame
    currdiff = pos;
    distance = VecLength(currdiff);
    if (c.max_dist > 0.0f && distance >= c.max_dist) {
      distance = c.max_dist;
    }

    // DOPPLER EFFECT
    if (l.doppler_scale > 0)
    {
      sVec3f lastdiff, lastpos;
      tF32      lastdistance;

      lastpos = pos - c.vel;
      lastdiff = lastpos - l.pos;
      lastdistance = VecLength(lastdiff);

      pitch3d = SPEED_OF_SOUND / l.distance_scale;
      pitch3d -= (distance - lastdistance) * l.doppler_scale;
      pitch3d /= SPEED_OF_SOUND / l.distance_scale;
    }
    if (pitch3d < afEpsilon) {
      pitch3d = afEpsilon;
    }

    // VOLUME ATTENUATION / LOGARITHMIC ROLLOFF
    if (distance <= c.min_dist) {
      distance = c.min_dist;
      val = 1.0f;
    }
    else {
      if (distance > c.min_dist) {
        distance -= c.min_dist;
        distance = distance * l.rolloff_scale;
        distance += c.min_dist;
      }

      // Rolloffscale is 0 safe but mindistance may be 0
      if (distance < afEpsilon) {
        distance = afEpsilon;
      }

      // Get the reciprocal.
      val = c.min_dist / distance;
      if (val < 0) {
        val = 0;
      }
      if (val > 1) {
        val = 1;
      }
    }

    vol3d = (int)(255.0f * val);

    // 3D PAN
    {
      // Normalize difference vector
      distance = VecLength(currdiff);
      if (distance <= 0) {
        // Set up a unit vector down y
        currdiff.x = 0;
        currdiff.y = 1;
        currdiff.z = 0;
      }
      else {
        // Normalize
        currdiff.x = currdiff.x / distance;
        currdiff.y = currdiff.y / distance;
        currdiff.z = currdiff.z / distance;
      }

      val = VecDot(currdiff,l.right);

      currpan = 128 + (int)(128.0f * val);
      if (currpan < 0) {
        currpan = 0;
      }
      if (currpan > 255) {
        currpan = 255;
      }

      currpan = (int)(((currpan - 128.0f) * afPanSeparation) + 128.0f);
    }

    anOutVolume = vol3d;
    anOutPan = currpan;
    afOutPitch = pitch3d;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall UpdateMixer3D() {
    mListener.Update();
    for (tU32 i = mnFirst3DChannel; i <= mnLast3DChannel; ++i) {
      sChannel c = chan[i-mnFirst3DChannel];
      if (c.mode != eSoundMode_Normal2D && mptrBase->GetIsChannelBusy(i)) {
        tI32 vol, pan;
        tF32 pitch;
        Compute3D(  vol,
                    pan,
                    pitch,
                    c,
                    mListener,
                    mnNumAudioChannels==2?1.0f:0.0f);
        mptrBase->SetChannelVolume(i,vol);
        mptrBase->SetChannelPan(i,pan);
      }
    }
    return eTrue;
  }

  niEndClass(cSoundMixerSoftware3D);
};

//////////////////////////////////////////////////////////////////////////////////////////////
iSoundMixer3D* __stdcall New_SoundMixerSoftware3D(iSoundMixer* apBase, tU32 anNum3DChannels, tU32 anNumAudioChannels) {
  Ptr<cSoundMixerSoftware3D> ptrSM = niNew cSoundMixerSoftware3D();
  if (!ptrSM->Initialize(apBase,anNum3DChannels,anNumAudioChannels)) {
    niError(_A("Can't create the software 3d sound mixer."));
    return NULL;
  }
  return ptrSM.GetRawAndSetNull();
}
