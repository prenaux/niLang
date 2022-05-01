#pragma once
#ifndef __BMPIO_ABM_H_E7C09B2B_9A84_4224_BCCD_E4DA31DD81E4__
#define __BMPIO_ABM_H_E7C09B2B_9A84_4224_BCCD_E4DA31DD81E4__

const ni::tU32 _kfccABM4 = niFourCC('A','B','M','4');
const ni::tU32 _kfccABM5 = niFourCC('A','B','M','5');
const ni::tU32 _kfccBMPC = niFourCC('B','M','P','C'); // bitmap container
const ni::tU32 _kfccBMP0 = niFourCC('B','M','P','0'); // bitmap type 0, unpacked
// const ni::tU32 _kfccBMP1 = niFourCC('B','M','P','1');  // bitmap type 1, LZO packed // NOT SUPPORTED
const ni::tU32 _kfccBMPZ = niFourCC('B','M','P','2'); // bitmap type 2, Zip packed
const ni::tU32 _kfccBMPJ = niFourCC('B','M','P','J'); // bitmap type J, Jpeg packed
const ni::tU32 _knABMCompressBufferSize = 1024*1024;

#endif // __BMPIO_ABM_H_E7C09B2B_9A84_4224_BCCD_E4DA31DD81E4__
