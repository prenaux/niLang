#ifndef __TEXTENCODING_H_5872CAAC_CE2E_46F5_BC1D_E02E8648E9CE__
#define __TEXTENCODING_H_5872CAAC_CE2E_46F5_BC1D_E02E8648E9CE__

ni::eTextEncodingFormat __stdcall TextEncodingFormatGet(ni::iFile* apFile, ni::tBool abSkipHeader, ni::eTextEncodingFormat aDefault);
ni::tBool __stdcall TextEncodingFormatWriteHeader(ni::iFile* apFile, ni::eTextEncodingFormat aFormat);
ni::tU32 __stdcall TextEncodingFormatReadChar(ni::iFile* apFile, ni::eTextEncodingFormat aFormat);
ni::tSize __stdcall TextEncodingFormatWriteChar(ni::iFile* apFile, ni::eTextEncodingFormat aFormat, ni::tU32 ch);
ni::tSize __stdcall TextEncodingFormatReadString(ni::iFile* apFile, ni::eTextEncodingFormat aFormat, ni::cString& strOut);
ni::tSize __stdcall TextEncodingFormatReadStringLine(ni::iFile* apFile, ni::eTextEncodingFormat aFormat, ni::cString& strOut);
ni::tSize __stdcall TextEncodingFormatWriteString(ni::iFile* apFile, ni::eTextEncodingFormat aFormat, const ni::achar* aaszStr, ni::tBool abWriteEndZero);

#endif // __TEXTENCODING_H_5872CAAC_CE2E_46F5_BC1D_E02E8648E9CE__
