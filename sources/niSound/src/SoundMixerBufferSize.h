#ifndef __SOUNDMIXERBUFFERSIZE_42744444_H__
#define __SOUNDMIXERBUFFERSIZE_42744444_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

const tU32 kMixerDefaultBlockSizeMs = 25;

struct sMixerBufferSize {
  sMixerBufferSize() {
    mnBufferSize = 0;
    mnBufferSizeMs = 0;
    mnBlockSize = 0;
    mnBlockSizeMs = 0;
  }

  sMixerBufferSize(const tU32 anLenMs, const tU32 anMixerRate, tBool abStereo) {
    mnBufferSize = 0;
    mnBufferSizeMs = 0;
    mnBlockSize = 0;
    mnBlockSizeMs = 0;
    Initialize(anLenMs,anMixerRate,abStereo?2:1);
  }

  ///////////////////////////////////////////////
  // anLenMs: Smaller buffer reduce latency and memory usage but are more unstable. Default to 50ms for
  //          a good hardware driver, and 200ms for emulated drivers (waveout, dsound buff, ...)
  void Initialize(tU32 anLenMs, const tU32 anMixerRate, tU32 anChannels) {

    // Needs to be at least the sizeof the latency
    if (anLenMs < kMixerDefaultBlockSizeMs) {
      anLenMs = kMixerDefaultBlockSizeMs;
    }
    mnBufferSizeMs = (anLenMs / kMixerDefaultBlockSizeMs) * kMixerDefaultBlockSizeMs;

    mnBlockSize = anMixerRate * kMixerDefaultBlockSizeMs / 1000;

#ifdef niWindowsCE
#define SOFTWARE_GRANULARITY  1024
#else
#define SOFTWARE_GRANULARITY  256
#endif

    mnBlockSize &= ~(SOFTWARE_GRANULARITY-1);
    if (mnBlockSize < SOFTWARE_GRANULARITY) {
      mnBlockSize = SOFTWARE_GRANULARITY;
    }

    // Recalculate the actual block size in ms now that the buffersize has been rounded off.
    // Store as 24:8
    mnBlockSizeMs = ((mnBlockSize * 1000) << 8) / anMixerRate;
    mnBufferSize = anMixerRate * mnBufferSizeMs / 1000;

    // Now make the mixer's buffersize divisible by the block size.
    mnBufferSize /= mnBlockSize;
    mnBufferSize *= mnBlockSize;

    // Make sure it is at least 2x bigger than the block size
    if (mnBufferSize < (mnBlockSize * 2)) {
      mnBufferSize = mnBlockSize * 2;
    }

    // Finally! recalculate the buffer size in milliseconds based on the new calculated buffersize in samples.
    mnBufferSizeMs = mnBufferSize * 1000 / anMixerRate;
  }

  tU32 mnBufferSizeMs;
  tU32 mnBufferSize;
  tU32 mnBlockSize;
  tU32 mnBlockSizeMs;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __SOUNDMIXERBUFFERSIZE_42744444_H__
