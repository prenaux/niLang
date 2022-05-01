#ifndef __DATATABLE_9927046_H__
#define __DATATABLE_9927046_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

namespace ni {

static const tU32 kfccBinaryDT = niFourCC('D','T','B','L');
static const tU32 knDataTableSerializeVersionRaw = niMakeVersion(2,0,1);
static const tU32 knDataTableSerializeVersionRawZip = niMakeVersion(2,1,1);
static const tU32 knDataTableSerializeVersionBitsLzma = niMakeVersion(2,2,0);
static const tU32 knDataTableSerializeVersionRawLzma = niMakeVersion(2,2,1);

//const ni::tU32 _knDataTableObjectSerializeVersion = knDataTableSerializeVersionRaw; // dtb
static const ni::tU32 _knDataTableObjectSerializeVersion = knDataTableSerializeVersionRawZip; // dtz

} // end of namespace ni

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __DATATABLE_9927046_H__
