// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"
#include "API/niUI/IWidgetText.h"
#include <niLang/Utils/StrBreakIt.h>
#include <niUI/Utils/WidgetSinkImpl.h>
#include "API/niUI_ModuleDef.h"
#include "niUI_HString.h"
#include "Graphics.h"
#include "TextMetric.h"
#include <niLang/Utils/UnitSnap.h>

#define TRACE_TEXT_OBJECT(X) //niDebugFmt(X)

#define DIRTY_LISTWORDS niBit(0)
#define DIRTY_WRAPTEXT  niBit(1)

//! Text alignment
enum eTextAlignment
{
  //! Text will be drawn left aligned.
  eTextAlignment_Left = 0,
  //! Text will be drawn right aligned.
  eTextAlignment_Right = 1,
  //! Text will be drawn centered.
  eTextAlignment_Center = 2,
  //! Text will be drawn justified, except the last line.
  eTextAlignment_Justify = 3,
  //! Text will be drawn justified.
  eTextAlignment_JustifyAll = 4,
  //! \internal
  eTextAlignment_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

#define TA_BUILD(ALIGN,COL) ((ALIGN&0xFF)|((COL)<<8))
#define TA_ALIGN(TA) ((TA)&0xFF)
#define TA_COL_ID(TA) ((TA)>>8)

//////////////////////////////////////////////////////////////////////////////////////////////
class sTextOccluder : public ImplRC<iTextOccluder>
{
  niBeginClass(sTextOccluder);

 public:
  ///////////////////////////////////////////////
  sTextOccluder(iTextObject* apParent) {
    mRect = sRectf::Zero();
    mpwParent = apParent;
  }

  ///////////////////////////////////////////////
  virtual Ptr<iTextObject> __stdcall GetTextObject() const {
    QPtr<iTextObject> textObject = mpwParent;
    return textObject.ptr();
  }

  ///////////////////////////////////////////////
  void __stdcall SetRect(const sRectf& aRect) {
    mRect = aRect;
    _UpdateParent();
  }
  sRectf __stdcall GetRect() const {
    return mRect;
  }

  ///////////////////////////////////////////////
  void __stdcall SetUserData(const Var& aUserData) {
    mUserData = aUserData;
  }
  Var __stdcall GetUserData() const {
    return mUserData;
  }

 private:
  inline void _UpdateParent() {
    QPtr<iTextObject> textObject = mpwParent;
    if (textObject.IsOK()) {
      textObject->Update();
    }
  }

  WeakPtr<iTextObject> mpwParent;
  sRectf               mRect;
  Var                  mUserData;
};

class cTextObject : public ImplRC<iTextObject>
{
  niBeginClass(cTextObject);
  struct sLine;
  struct sWord;

 public:
  ///////////////////////////////////////////////
  cTextObject(iGraphics* apGraphics, const sVec2f& avSize, const tF32 afContentsScale) {
    mptrGraphics = apGraphics;
    mfContentsScale = afContentsScale;
    mvSize = avSize;
    mbTrimLeadingSpaces = eFalse;
    mnTruncationStyle = eTextTruncation_None;
    mstrTrunc = _A("...");
    mnDirty = 0;
    mnLastWordIndex = 0;
    mfCurrentTotalHeight = 0;
    mbAddingText = eFalse;
    mbKerning = eTrue;
    mvSelectionColor = Vec4f(0.2f,0.4f,0.55f,0.3f);
    mptrDefaultFont = mptrGraphics->LoadFont(_H("Default"));

    tU32 nColID = 0;
    auto registerColumns = [&](const tU32 aNumCols) {
      const tF32 step = (1.0f / (tF32)aNumCols);
      niLoop(i,aNumCols) {
        RegisterColumn(++nColID, niFmt("c%d_%d",i+1,aNumCols), step * (tF32)(i+1));
      }
    };

    registerColumns(12);
    registerColumns(4);
    RegisterColumn(++nColID, "rl", 0.0f);
    RegisterColumn(++nColID, "rc", 0.5f);
    RegisterColumn(++nColID, "rr", 1.0f);
  }

  ///////////////////////////////////////////////
  ~cTextObject() {
  }

  ///////////////////////////////////////////////
  tBool __stdcall IsOK() const {
    niClassIsOK(cTextObject);
    return mptrGraphics.IsOK() && mptrDefaultFont.IsOK();
  }

  ///////////////////////////////////////////////
  virtual iGraphics* __stdcall GetGraphics() const {
    return mptrGraphics;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetContentsScale(const tF32 afContentsScale) {
    mfContentsScale = afContentsScale;
  }
  virtual tF32 __stdcall GetContentsScale() const {
    return mfContentsScale;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetDefaultFont(iFont* apFont) {
    if (mptrDefaultFont == apFont)
      return;
    if (!niIsOK(apFont)) {
      mptrDefaultFont = mptrGraphics->LoadFont(_H("Default"));
    }
    else {
      mptrDefaultFont = apFont;
    }
  }
  virtual iFont* __stdcall GetDefaultFont() const {
    return mptrDefaultFont;
  }


  ///////////////////////////////////////////////
  virtual void __stdcall SetLoadFontCallback(iCallback* apLoadFontCallback) {
    mptrLoadFontCallback = niGetIfOK(apLoadFontCallback);
  }
  virtual iCallback* __stdcall GetLoadFontCallback() const {
    return mptrLoadFontCallback;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetSize(const sVec2f& avSize) {
    if (avSize == mvSize)
      return;
    mvSize = avSize;
    _NotifyWrapText();
  }
  virtual sVec2f __stdcall GetSize() const {
    return mvSize;
  }

  ///////////////////////////////////////////////
  //! Widget's update.
  inline void __stdcall _NotifyListWords() { mnDirty |= DIRTY_LISTWORDS; }
  inline void __stdcall _NotifyWrapText() { mnDirty |= DIRTY_WRAPTEXT; }
  inline void __stdcall _DoUpdate() {
    if (!mnDirty || !IsOK())
      return;

    if (mnDirty & DIRTY_LISTWORDS) {
      mlstWords.clear();
      _ListWords(mstrText.Chars());
    }

    if (mnDirty & DIRTY_WRAPTEXT) {
      // niDebugFmt(("... DIRTY_WRAPTEXT"));
      mnLastWordIndex = 0;
      mfCurrentTotalHeight = 0.0f;
      tU32 nWordIndex = 0;
      tU32 nNumLinesAdded = 0;
      do {
        nWordIndex = _DoWrapText(nNumLinesAdded, mfCurrentTotalHeight);
        if (!mlstLines.empty())
          mfCurrentTotalHeight += mlstLines.back().height;
        nNumLinesAdded++;
      } while (nWordIndex != eInvalidHandle);
      _DoAlignText(0);
    }

    // Update the text size
    {
      mTextSize.Set(0,0);
      const tF32 defaultHeight = TextLineMetric_ComputeLineHeight(mptrDefaultFont);
      niLoop(j,mlstLines.size()) {
        const sLine& pLine = mlstLines[j];
        mTextSize.x = ni::Max(mTextSize.x,pLine.width);
        mTextSize.y += ni::Max(defaultHeight,pLine.height);
      }
    }

    mnDirty = 0;
  }

  ///////////////////////////////////////////////
  void __stdcall Update() {
    if (!IsOK())
      return;
    if (mlstWords.empty()) {
      _NotifyListWords();
    }
    _NotifyWrapText();
  }

  ///////////////////////////////////////////////
  void __stdcall SetTrimLeadingSpaces(tBool abTrimLeadingSpaces) {
    mbTrimLeadingSpaces = abTrimLeadingSpaces;
    _NotifyListWords();
    _NotifyWrapText();
  }
  tBool __stdcall GetTrimLeadingSpaces() const {
    return mbTrimLeadingSpaces;
  }

  ///////////////////////////////////////////////
  void __stdcall SetExpressionContext(iExpressionContext* apContext) {
    mptrExpressionContext = apContext;
    _NotifyListWords();
    _NotifyWrapText();
  }
  iExpressionContext* __stdcall GetExpressionContext() const {
    return mptrExpressionContext;
  }

  ///////////////////////////////////////////////
  void __stdcall SetKerning(tBool abKerning) {
    if (mbKerning == abKerning)
      return;
    mbKerning = abKerning;
    _NotifyWrapText();
  }
  tBool __stdcall GetKerning() const {
    return mbKerning;
  }

  ///////////////////////////////////////////////
  void __stdcall SetTruncation(eTextTruncation aType) {
    mnTruncationStyle = aType;
    _NotifyWrapText();
  }
  eTextTruncation __stdcall GetTruncation() const {
    return mnTruncationStyle;
  }
  void __stdcall SetTruncationText(const achar* aaszString) {
    mstrTrunc = aaszString;
    _NotifyWrapText();
  }
  const achar* __stdcall GetTruncationText() const {
    return mstrTrunc.Chars();
  }

  ///////////////////////////////////////////////
  virtual sVec2f __stdcall GetTextSize() const {
    niThis(cTextObject)->_DoUpdate();
    return mTextSize;
  }

  ///////////////////////////////////////////////
  //! Word Wrap process
  void __stdcall _ListWords(const achar* aaszStr) {
    niProfileBlock(_ListWords);
    if (!niStringIsOK(aaszStr))
      return;

    tBool bUpdate = !mlstWords.empty();

    // tag-free string list
    tRawStrings mapStrings;

    ////////////////////////////////////////////////////////
    // Preprocessing the tags and fonts                   //
    ////////////////////////////////////////////////////////
    // don't start clearing fonts unnecesserally
    if (!bUpdate) {
      mFontManager.Init(this);
    }
    else {
      mFontManager.Continue();
    }

    {
      Ptr<sParseTagsXmlParserSink> xmlParser = niNew sParseTagsXmlParserSink(this,mapStrings);
      cString str;
      str << "<Text>";
      str << aaszStr;
      str << "</Text>";
      GetLang()->XmlParseString(str.Chars(),xmlParser);
    }

    ////////////////////////////////////////////////////////
    // Tokenizing the text                                //
    ////////////////////////////////////////////////////////
    tBool removeFirstEmptyLine = eTrue;
    cString firstIndent;

    tU32 lineNumWords = 0;
    tU32 lineFirstIndex = 0;
    tU32 lastWordIndex = 0;
    tBool lastWordIsSeparator = eFalse;

    auto addLineBreak = [&](iFont* apFont) {
      sRawString s(eTextAlignment_Left);
      s.font = apFont;
      s.string = _A("\n");
      s.size = sVec2f::Zero();
      mlstWords.push_back(s);
      lineNumWords = 0;
    };

    auto addWord = [&](iFont* apFont, sRawString& s, const tBool abIsSeparator) {
      if (mbTrimLeadingSpaces) {
        if (mlstWords.empty() && abIsSeparator) {
          firstIndent = s.string;
          // niDebugFmt(("... trimToFirstIndent: %s (%d)", firstIndent, firstIndent.size()));
        }
        if (lineNumWords == 0 && abIsSeparator) {
          if (s.string == firstIndent) {
            // niDebugFmt(("... trimToFirstIndent: skipped same indent"));
            return;
          }
          if (s.string.size() < firstIndent.size()) {
            // niDebugFmt(("... trimToFirstIndent: skipped shorter indent"));
            return;
          }
          // const tI32 prevLen = s.string.size();
          s.string = s.string.After(firstIndent);
          // niDebugFmt(("... trimToFirstIndent: trimed %d -> %d", prevLen, s.string.size()));
        }
      }

      s.font = apFont;
      s.size = Vec2f(TextLineMetric_ComputeRect(apFont, s.string, mbKerning).GetWidth(),
                     TextLineMetric_ComputeLineHeight(apFont));

      lastWordIndex = (tU32)mlstWords.size();
      lastWordIsSeparator = abIsSeparator;
      if (lineNumWords == 0) {
        lineFirstIndex = lastWordIndex;
      }
      ++lineNumWords;
      mlstWords.push_back(s);
    };

    niLoop(k,mapStrings.size()) {
      const sRawString& pRawStr = mapStrings[k];
      const Ptr<iFont> ptrFont = pRawStr.font;

      // const cString& strChunk = pRawStr.string;
      // niDebugFmt(("CHUNK: '%s'",strChunk));
      StrBreakLineIt lineIt(pRawStr.string.charIt());
      while (!lineIt.is_end()) {
        // niDebugFmt(("LINE: '%s'",_ASTR(lineIt.current())));

        // Iterate through the words
        {
          StrBreakSpaceIt wordIt(lineIt.current());
          while (!wordIt.is_end()) {
            const ni::StrCharIt& theWord = wordIt.current();
            if (theWord.length() > 0) {
              // niDebugFmt(("- WORD: '%s' (%d)",_ASTR(theWord),mlstWords.size()));
              sRawString s(pRawStr.alignment);
              s.string.Set(theWord);
              s.expr = pRawStr.expr;
              addWord(ptrFont,s,eFalse);
            }
            const ni::StrCharIt& theSep = wordIt.breaker().lastSeparator;
            if (theSep.length() > 0) {
              sRawString s(pRawStr.alignment);
              // niDebugFmt(("- SEP: '%s' (%d)",_ASTR(theSep),lineNumWords));
              s.string.Set(theSep);
              addWord(ptrFont,s,eTrue);
            }
            wordIt.next();
          }
        }

        // Add a new line if its a hard line break...
        {
          tBool hasLineBreak = eFalse;
          {
            // niDebugFmt(("- SEP-NL: '%s'",_ASTR(lineIt.breaker().lastSeparator)));
            StrCharIt itSep(lineIt.breaker().lastSeparator);
            while (!itSep.is_end()) {
              tU32 c = itSep.next();
              if (c == '\n' || c == '\r') {
                hasLineBreak = eTrue;
                break;
              }
            }
          }
          if (hasLineBreak) {
            if (mlstWords.empty() && removeFirstEmptyLine) {
              // niDebugFmt(("- FIRST NL SEPARATOR SKIPPED"));
              removeFirstEmptyLine = eFalse;
            }
            else {
              // niDebugFmt(("- NL SEPARATOR"));
              addLineBreak(ptrFont);
            }
          }
        }

        lineIt.next();
      }
    }
  }

  typedef astl::vector<sVec4f> tSegments;
  void __stdcall _ComputeOccluderLineSegments(tF32 afYPosition, tF32 afLineSize, tSegments* aSegments, iTextOccluder* aOccluder) {
    // this covers the area higher than any occluder
    tF32 fBottomLine = afYPosition + afLineSize;
    niProfileBlock(WidgetText_ComputeSegments);
    if (aSegments->empty()) {
      sVec4f rect;
      rect.x = 0;
      rect.y = afYPosition;
      rect.z = mvSize.x;
      rect.w = afLineSize;
      aSegments->push_back(rect);
    }

    if (aOccluder) {
      // get the top of the highest positioned occluder,
      // same for the bottom.
      const sRectf rectOccluder = aOccluder->GetRect();
      const tF32 top = rectOccluder.y;
      const tF32 bottom = top + rectOccluder.GetHeight();

      if (top < fBottomLine && bottom > afYPosition) {
        const tF32& fOccPos  = rectOccluder.x;
        const tF32& fOccSize = rectOccluder.GetWidth();
        sVec4f vRect;
        {
          sVec4f& vFrontSegment = aSegments->front();
          sVec4f vBackSegment = aSegments->back();

          // left side
          vRect.x = vFrontSegment.x;
          vRect.y = vFrontSegment.y;
          vRect.z = (vFrontSegment.x + vFrontSegment.z > fOccPos) ? fOccPos - vFrontSegment.x : vFrontSegment.z;
          vRect.w = vFrontSegment.w;
          if (aSegments->size() == 1) {
            aSegments->clear();
            aSegments->push_back(vRect);
          }
          else {
            aSegments->erase(aSegments->begin());
            aSegments->insert(aSegments->begin(),vRect);
          }

          // right side
          sVec4f vBackRect = sVec4f::Zero();
          vBackRect.x = fOccPos + fOccSize;
          vBackRect.y = vBackSegment.y;
          // vBackSegment.x being the 'offset' created by a possible occluder docked left
          vBackRect.z = vBackSegment.z - vBackRect.x + vBackSegment.x;
          vBackRect.w = vBackSegment.w;
          if (aSegments->size() > 1)
            aSegments->pop_back();
          aSegments->push_back(vBackRect);
        }
      }
    }
  }
  tU32 __stdcall _DoWrapText(tU32 anLine, tF32 aCurH) {
    if (mlstWords.empty())
      return eInvalidHandle;
    if (mvSize.x <= 0 || mvSize.y <= 0)
      return eInvalidHandle;
    niProfileBlock(WidgetText_DoWrapText);

    tU32 nStartingWordIndex = mnLastWordIndex;
    if (!mlstLines.empty() && anLine == 0) {
      mlstLines.clear();
      mlstLines.reserve(ni::Max(50,mlstWords.size()/64));
      mlstLineWords.clear();
      mlstLineWords.reserve(mlstWords.size());
    }

    const tF32 fWdgWidth = mvSize.x;
    tU32 nWordIndex niMaybeUnused = 0;
    sLine line;

    tU32 nCurrentSegment = 0;
    tF32 fLastSegmentSize = 0.0f;

    for (tU32 i = nStartingWordIndex; i < mlstWords.size(); i++) {
      mnLastWordIndex = i;
      sRawString& rawStr = mlstWords[i];
      {
        TextLineMetric_Clear(&rawStr.textMetric);
        rawStr.UpdateExpression(mbKerning);
      }

      const iFont* pFont = rawStr.font;
      const tF32 fontLineHeight = TextLineMetric_ComputeLineHeight(pFont);
      const tF32 fSpaceWidth = pFont->GetAdvance(' ');
      tF32 fWordWidth = rawStr.size.x;

      const cString& w = rawStr.string;
      if (w == _A("\n")) {
        // new line that we hardcoded
        if (line.height < fontLineHeight) {
          line.height = fontLineHeight;
          line.ascent = pFont->GetAscent();
        }
        line.y_pos = aCurH;
        mlstLines.push_back(line);
        if (!line.words_empty()) {
          sWord& pLastWord = mlstLineWords[line[line.words_size()-1]];
          const sRawString& rRaw = mlstWords[pLastWord.index];
          if (rRaw.string.EndsWith(" ")) {
            pLastWord.width -= fSpaceWidth;
            line.width -= fSpaceWidth;
          }
        }
        mnLastWordIndex += 1; // i + 1
        return i+1;
      }
      else if (i == nStartingWordIndex && (w.empty() || w == _A(" "))) {
        nStartingWordIndex++;
        // if the first word of the line is a space character
        // or is empty... skip it.
        continue;
      }

      // we will compute the space available
      // on target line to render the text.
      if (line.segments.empty()) {
        niProfileBlock(WidgetText_DoWrapText_CreateSegments);
        niLoop(i,mvOccluders.size()) {
          Ptr<iTextOccluder> occ = mvOccluders[i];
          _ComputeOccluderLineSegments(aCurH, fontLineHeight, &line.segments, occ);
        }
        if (line.segments.empty()) {
          _ComputeOccluderLineSegments(aCurH, fontLineHeight, &line.segments, NULL);
        }
      }
      const tF32 fSegSize = line.segments[nCurrentSegment].z;

      // if the word is bigger but we're only on the first segment when there
      // are actually more than one then we just need to process it as if the
      // "line" was bigger than the segment... No need to truncate.
      const tBool bWordBiggerThanWidget = (fWordWidth > fWdgWidth);

      // check if we need to go to the next line
      const tBool bLineBiggerThanSegment = bWordBiggerThanWidget ||
          ((line.width - fLastSegmentSize) + fWordWidth > fSegSize);

      ///////////////////
      // Truncate Word //
      ///////////////////
      if (bWordBiggerThanWidget && (mnTruncationStyle != eTextTruncation_None)) {
        tF32 fTruncStringSize = TextLineMetric_ComputeRect(pFont,mstrTrunc,mbKerning).GetWidth();
        if (fTruncStringSize > fWdgWidth) {
          // if even the (...) is bigger than the widget
          mlstLines.clear();
          return eInvalidHandle;
        }
        else {
          cString strTmpWord = w;

          // this is processing quite a lot when dealing with huge walls of text
          while (fWordWidth+fTruncStringSize > fSegSize) {
            // remove that char from the original string
            strTmpWord.Remove();
            if (strTmpWord.empty()) {
              break;
            }
            // recompute the size
            fWordWidth = TextLineMetric_ComputeRect(pFont,strTmpWord,mbKerning).GetWidth();
          }
          fWordWidth += fTruncStringSize;

          switch(mnTruncationStyle) {
            case eTextTruncation_Left:
              strTmpWord = mstrTrunc + strTmpWord;
              break;
            case eTextTruncation_Right:
              strTmpWord.append(mstrTrunc.Chars());
              break;
            case eTextTruncation_None:
              // nothing to do
              break;
          }

          rawStr.truncatedString = strTmpWord;
        }
      }
      else if (!rawStr.truncatedString.empty()) {
        rawStr.truncatedString.clear();
      }

      if (bLineBiggerThanSegment) {
        niProfileBlock(WidgetText_DoWrapText_TestSize);
        // set default settings in case the line is empty
        if (line.words_empty() && nCurrentSegment == line.segments.size()-1) {
          line.height = TextLineMetric_ComputeLineHeight(mptrDefaultFont);
          line.ascent = mptrDefaultFont->GetAscent();
        }

        if (nCurrentSegment < line.segments.size()-1) {
          nCurrentSegment++;
          i--;
          fLastSegmentSize = line.width;
          continue;
        }
        else {
          if (!line.words_empty()) {
            sWord& pLastWord = mlstLineWords[line[line.words_size()-1]];
            const sRawString& rRaw = mlstWords[pLastWord.index];
            if (rRaw.string.EndsWith(_A(" "))) {
              pLastWord.width -= fSpaceWidth;
              line.width -= fSpaceWidth;
            }
            // add line
            mlstLines.push_back(line);
            return i;
          }
        }
      }

      // adding word to line, setting current size
      {
        niProfileBlock(WidgetText_DoWrapText_AddingWord);
        sWord word;
        word.index = i;
        if (rawStr.selected) {
          niProfileBlock(WidgetText_DoWrapText_AddingWord_UpdateSelection);
          const tU32 nID = rawStr.selectionID;
          if (nID != eInvalidHandle) {
            sSelection& rSel = mlstSelections[nID];
            rSel.line = (tU32)mlstLines.size();
            rSel.word = line.words_size();
          }
        }
        word.x_pos = line.width;
        word.width = fWordWidth;
        word.segment = nCurrentSegment;
        word.font = pFont;
        word.alignment = rawStr.alignment;
        // add the word
        line.words_push_back(word,mlstLineWords);
      }
      line.width += fWordWidth;

      if (line.height < fontLineHeight) {
        niProfileBlock(WidgetText_DoWrapText_UpdateSegments);
        line.height = fontLineHeight;
        line.ascent = pFont->GetAscent();
        niLoop(j,line.segments.size()) {
          sVec4f& vSegment = line.segments[j];
          vSegment.y = aCurH;
          vSegment.w = line.height; //fFontSize;
        }
      }
      nWordIndex++;
    }
    mlstLines.push_back(line);
    return eInvalidHandle;
  }

  tF32 __stdcall _GetColPos(const tU32 aColID, const tF32 afFullWidth) {
    tColPosMap::const_iterator itColPos = mmapColPos.find(aColID);
    if (itColPos != mmapColPos.end()) {
      tF32 colPos = itColPos->second;
      if (colPos <= 1.0f && colPos >= -1.0f) {
        colPos = (colPos * afFullWidth);
      }
      if (colPos < 0) {
        colPos = afFullWidth - colPos;
      }
      colPos = ni::UnitSnapf(colPos);
      return colPos;
    }
    return niMaxF32;
  }

  tF32 _GetStartPos(const sLine& aLine, const sWord& aWord, const tF32 aNextX) {
    const sVec4f& vSegment = aLine.segments[aWord.segment];
    const tU32 colID = TA_COL_ID(aWord.alignment);
    if (colID > 0) {
      tF32 colPos = _GetColPos(colID, vSegment.z);
      if (colPos != niMaxF32) {
        TRACE_TEXT_OBJECT(("... Align Col: %d, %.2f", colID, colPos));
        return colPos;
      }
      else {
        niWarning(niFmt("Couldn't find column id '%d'.", colID));
      }
    }
    return (aNextX == niMaxF32) ? vSegment.x : aNextX;
  };

  tU32 _Measure(
      sVec3f& aOut, // left edge, right edge, len, next start x
      tF32& aOutNextXPos,
      const sLine& aLine,
      const tU32 k,
      const sWord& firstWord,
      const tF32 xStart)
  {
    const tU32 segmentID = firstWord.segment;
    const sVec4f& vSegment = aLine.segments[segmentID];
    const tU32 align = TA_ALIGN(firstWord.alignment);
    const tU32 numLineWords = aLine.words_size();
    tU32 lastWordIndex = 0;
    tF32 xLen = 0.0f;
    tF32 xLeft = xStart;
    tF32 xRight = 0.0f;
    {
      // find the length and right edge of the current segment
      tF32 rx = xStart;
      tU32 rk = k;
      for ( ; rk < numLineWords; ++rk) {
        const sWord& rword = mlstLineWords[aLine[rk]];
        if (firstWord.segment != rword.segment) {
          aOutNextXPos = niMaxF32;
          lastWordIndex = rk-1;
          xRight = _GetStartPos(aLine, rword, niMaxF32);
          break;
        }
        if (firstWord.alignment != rword.alignment) {
          aOutNextXPos = rx;
          lastWordIndex = rk-1;
          xRight = _GetStartPos(aLine, rword, rx);
          if (TA_ALIGN(firstWord.alignment) == eTextAlignment_Center &&
              TA_ALIGN(rword.alignment) == eTextAlignment_Right)
          {
            sVec3f m; tF32 nx;
            _Measure(m, nx, aLine, rk, rword, xRight);
            xRight = m.x;
          }
          break;
        }
        xLen += rword.width;
        rx += rword.width;
      }
      if (rk == numLineWords) {
        xRight = vSegment.x + vSegment.z;
        lastWordIndex = numLineWords-1;
      }
      TRACE_TEXT_OBJECT((
          "... measure[%d,%d]: %d, xLen: %g, xLeft: %g, xRight: %g, xStart: %g, lastWord: %d, nextX: %g",
          segmentID, k,
          (tU32)align, xLen, xLeft, xRight, xStart, lastWordIndex, aOutNextXPos));
    }

    switch (align) {
      case eTextAlignment_Right: {
        if (TA_COL_ID(firstWord.alignment) > 0) {
          // column position is the 'pivot'
          xLeft = xStart - xLen;
        }
        else {
          xLeft = xRight - xLen;
        }
        break;
      }
      case eTextAlignment_Center: {
        if (TA_COL_ID(firstWord.alignment) > 0) {
          // column position is the 'pivot'
          xLeft = xStart + (((xRight-xStart)/2) - (xLen/2));
        }
        else {
          xLeft = xLeft + (((xRight-xStart)/2) - (xLen/2));
        }
        break;
      }
    }

    aOut.x = xLeft;
    aOut.y = xRight;
    aOut.z = xLen;
    return lastWordIndex;
  };

  void __stdcall _DoAlignText(tU32 anStartingLine) {
    niProfileBlock(WidgetText_DoAlignText);
    const tF64 startLayout = ni::TimerInSeconds();

    TRACE_TEXT_OBJECT(("... _DoAlignText: %s", GetSize()));

    // process the words position now that
    // every word has been processed.
    niLoop(j,mlstLines.size()) {
      sLine& pLine = mlstLines[j];
      if (pLine.segments.empty()) {
        continue;
      }

      tF32 nextXPos = niMaxF32;
      const tU32 numLineWords = pLine.words_size();
      for (tU32 k = 0; k < numLineWords; ) {
        // const tU32 firstWordIndex = k;
        const sWord& firstWord = mlstLineWords[pLine[k]];
        const tU32 segmentID = firstWord.segment;
        const sVec4f& vSegment = pLine.segments[segmentID];
        const tU32 align = TA_ALIGN(firstWord.alignment);

        sVec3f m;
        tU32 lastWordIndex = _Measure(
            m, nextXPos, pLine, k, firstWord,
            _GetStartPos(pLine, firstWord, nextXPos));

        // Layout left to right
        for (tF32 x = m.x; k <= lastWordIndex; ++k) {
          sWord& word = mlstLineWords[pLine[k]];
          word.x_pos = x;
          pLine.y_pos = vSegment.y;
          x += word.width;
        }
      }
    }

    const tF64 endLayout = ni::TimerInSeconds();
    TRACE_TEXT_OBJECT(("... Layout in %gms.", (endLayout-startLayout)*1000.0f));
  }

  ///////////////////////////////////////////////
  tBool __stdcall Draw(iCanvas* apCanvas, const sRectf& aClippingRect) {
    return DrawAt(apCanvas, aClippingRect, sVec3f::Zero());
  }

  ///////////////////////////////////////////////
  tBool __stdcall DrawAt(iCanvas* apCanvas, const sRectf& aClippingRect, const sVec3f& avPos) {
    if (!IsOK() || !niIsOK(apCanvas))
      return eFalse;

    _DoUpdate();

    // Draw selection: this draws the rectangle using the top left of the
    // first word of a line and the bottom right of its last one.
    {
      const tU32 selColor = ULColorBuild(mvSelectionColor);
      sVec2f tl = sVec2f::Zero();
      tU32 nLastLine = eInvalidHandle;
      tF32 fLastWordPos = 0.0f;
      tF32 fLastWordWidth = 0.0f;
      tF32 fLastLineHeight = 0.0f;
      niLoop(i,mlstSelections.size()) {
        const sSelection& rSel = mlstSelections[i];
        if (rSel.line >= mlstLines.size())
          continue;
        const sLine& rLine = mlstLines[rSel.line];
        if (rSel.word >= rLine.words_size())
          continue;
        const sWord& rWord = mlstLineWords[rLine[rSel.word]];
        const tF32 fYPos = rLine.y_pos;
        tBool bDrawn = eFalse;
        if (rSel.line != nLastLine) {
          if (nLastLine != eInvalidHandle) {
            sVec2f br;
            br.x = fLastWordPos + fLastWordWidth;
            br.y = tl.y + fLastLineHeight;
            apCanvas->BlitFill(sRectf(tl,br),selColor);
            bDrawn = eTrue;
          }
          tl.x = rWord.x_pos;
          tl.y = fYPos;
        }
        // when processing the last word we need to draw
        // what's left of the selection!
        if (i == mlstSelections.size()-1 && !bDrawn) {
          sVec2f br;
          br.x = rWord.x_pos + rWord.width;
          br.y = tl.y + rLine.height;
          apCanvas->BlitFill(sRectf(tl,br),selColor);
        }

        nLastLine = rSel.line;
        fLastLineHeight = rLine.height;
        fLastWordPos   = rWord.x_pos;
        fLastWordWidth = rWord.width;
      }
    }

    const tBool hasClipping = aClippingRect != sRectf::Null();

    // Render text
    niLoop(i,(tU32)mlstLines.size()) {
      const sLine& rLine = mlstLines[i];
      iFont* pFont = rLine.words_empty() ? mptrDefaultFont : mlstLineWords[rLine[0]].font;

      // occluding text
      tF32 fLinePos = rLine.y_pos + rLine.height;
      tF32 fOffsetPos = rLine.y_pos;
      if (fLinePos <= mvSize.y && fOffsetPos >= 0.0f)
      {
        const tF32 fLineYPos = rLine.y_pos;
        if (!hasClipping || (fLinePos >= aClippingRect.GetTop() &&
                             fLinePos < aClippingRect.GetBottom()))
        {
          tF32 fWordYPos = fLineYPos + (rLine.ascent - pFont->GetAscent());
          niLoop(j,rLine.words_size()) {
            const sWord& rWord = mlstLineWords[rLine[j]];
            if (rWord.font != pFont) {
              pFont = rWord.font;
              if (!pFont) {
                pFont = mptrDefaultFont;
              }
              fWordYPos = fLineYPos + (rLine.ascent - pFont->GetAscent());
            }
            sRawString& rawString = mlstWords[rWord.index];
            const cString& string = rawString.truncatedString.empty() ?
                rawString.string : rawString.truncatedString;
            if (TextLineMetric_IsEmpty(&rawString.textMetric)) {
              TextLineMetric_AddText(
                &rawString.textMetric,
                pFont,
                ni::UnitSnapf(rWord.x_pos),
                ni::UnitSnapf(fWordYPos),
                string.Chars(), string.size(),
                mbKerning);
            }
            TextLineMetric_PushToCanvas(
              &rawString.textMetric, apCanvas, pFont,
              pFont->GetColor(), avPos,
              NULL);
          }
        }
      }
    }

    return eTrue;
  }

  ///////////////////////////////////////////////
  iTextOccluder* __stdcall AddOccluder(const sRectf& aRect, const Var& aUserData) {
    Ptr<iTextOccluder> to = niNew sTextOccluder(this);
    to->SetUserData(aUserData);
    to->SetRect(aRect);
    mvOccluders.push_back(to);
    _NotifyWrapText();
    return to;
  }
  tBool __stdcall RemoveOccluder(tU32 anIndex) {
    niCheckSilent(anIndex < mvOccluders.size(),eFalse);
    mvOccluders.erase(mvOccluders.begin()+anIndex);
    return eTrue;
  }
  void __stdcall ClearOccluders() {
    niCheckSilent(!mvOccluders.empty(),;);
    mvOccluders.clear();
  }
  tU32 __stdcall GetNumOccluders() const {
    return (tU32)mvOccluders.size();
  }
  iTextOccluder* __stdcall GetOccluder(tU32 anIndex) const {
    niCheckSilent(anIndex < mvOccluders.size(),NULL);
    return mvOccluders[anIndex];
  }

  ///////////////////////////////////////////////
  cString __stdcall GetSelectedString() const {
    cString s;
    niLoop(i,mlstSelections.size()) {
      const sSelection& rSel = mlstSelections[i];
      if (rSel.line >= mlstLines.size()) continue;
      const sLine& rLine = mlstLines[rSel.line];
      if (rSel.word >= rLine.words_size()) continue;
      const sWord& rWord = mlstLineWords[rLine[rSel.word]];
      const sRawString& rRaw = mlstWords[rWord.index];
      s << rRaw.string;
    }
    return s;
  }

  ///////////////////////////////////////////////
  void __stdcall SetText(const achar* aaszString) {
    // niDebugFmt(("... SetText: %s", aaszString));
    mstrText = aaszString;
    mlstWords.clear();
    this->Update();
  }
  const achar* __stdcall GetText() const {
    return mstrText.Chars();
  }

  ///////////////////////////////////////////////
  void __stdcall AddText(const achar* aaszString) {
    if (!mlstLines.empty()) {
      tI32 nLastIndex = (tI32)mlstLines.size()-1;
      sLine& rLine = mlstLines.back();
      while (rLine.words_empty()) {
        nLastIndex--;
        if (nLastIndex < 0)
          break;
        rLine = mlstLines[nLastIndex];
      }
      const tF32 fYPos = rLine.y_pos;

      tU32 nWordIndex = mlstLineWords[rLine[0]].index;
      mnLastWordIndex = nWordIndex;
      mlstLines.pop_back();

      _ListWords(aaszString);
      tU32 nNumLinesAdded = (tU32)mlstLines.size();
      mfCurrentTotalHeight = fYPos;
      do {
        nWordIndex = _DoWrapText(nNumLinesAdded, mfCurrentTotalHeight);
        if (!mlstLines.empty()) {
          mfCurrentTotalHeight += mlstLines.back().height;
        }
        nNumLinesAdded++;
      } while (nWordIndex != eInvalidHandle);
      _DoAlignText((tU32)mlstLines.size()-nNumLinesAdded);

      this->Update();
    }
    else {
      mstrText += aaszString;
      mlstWords.clear();
      this->Update();
    }
  }

  ///////////////////////////////////////////////
  tU32 __stdcall FindWordIndexFromPosition(sVec2f avPosition) const {
    tF32 h = 0.0f;
    tU32 lastWordIndex = eInvalidHandle;

    niLoop(i,mlstLines.size()) {
      const sLine& line = mlstLines[i];
      const tF32 nextLineY = h + line.height;

      if (line.words_size() > 0) {
        if (avPosition.y > h && avPosition.y <= nextLineY) {
          // position is in the line
          niLoop(j,line.words_size()) {
            const sWord& word = mlstLineWords[line[j]];
            const tF32 position = word.x_pos;
            if ((avPosition.x > position) && (avPosition.x < position + word.width)) {
              return word.index;
            }
          }
        }

        lastWordIndex = mlstLineWords[line[line.words_size()-1]].index + 1;
      }

      h = nextLineY;
      if (h > avPosition.y) {
        break;
      }
    }

    return ni::Min(lastWordIndex, mlstWords.size());
  }

  tBool __stdcall RegisterColumn(tU32 anID, const achar* aaszName, tF32 afPosition) {
    if (anID < 1) {
      return eFalse;
    }
    mmapColPos[anID] = afPosition;
    if (niStringIsOK(aaszName)) {
      mmapColNames[aaszName] = anID;
    }
    // niLog(Info,niFmt("Registered text column: id: %d, name: '%s', pos: %.2f.",
                     // anID, aaszName, afPosition));
    return eTrue;
  }

  void __stdcall SetSelectionColor(sVec4f avColor) {
    mvSelectionColor = avColor;
  }
  sVec4f __stdcall GetSelectionColor() const {
    return mvSelectionColor;
  }

  void __stdcall ClearSelection() {
    niLoop(i,mlstSelections.size()) {
      const sSelection& rSel = mlstSelections[i];
      sLine& rLine = mlstLines[rSel.line];
      sWord& rWord = mlstLineWords[rLine[rSel.word]];
      rWord.selected = eFalse;

      sRawString& rRaw = mlstWords[rSel.index];
      rRaw.selected = eFalse;
      rRaw.selectionID = eInvalidHandle;
    }
    mlstSelections.clear();
  }

  void __stdcall _SelectWord(tU32 anLine, tU32 anWord) {
    if (anLine >= mlstLines.size()) return;
    sLine& rLine = mlstLines[anLine];
    if (anWord >= rLine.words_size()) return;
    sWord& rWord = mlstLineWords[rLine[anWord]];
    if (rWord.selected) return;
    rWord.selected = eTrue;

    sSelection s;
    s.index = rWord.index;
    s.line = anLine;
    s.word = anWord;
    {
      // Save selection infos in the raw list of words
      // We do this because that list is permanent compared
      // to the mlstLines that gets constantly refreshed
      sRawString& rRaw = mlstWords[s.index];
      rRaw.selected     = eTrue;
      rRaw.selectionID = (tU32)mlstSelections.size();
    }
    mlstSelections.push_back(s);
  }

  void __stdcall SelectRange(tU32 anBegin, tU32 anEnd) {
    ClearSelection();
    if (anBegin > anEnd) {
      ni::Swap(anBegin, anEnd);
    }

    tU32 nNumSelections = (anEnd - anBegin) + 1;
    // we need to remove any 'invisible' character
    // such as newlines '\n'
    for (tU32 i = anBegin; i < anEnd; i++) {
      const sRawString& rRaw = mlstWords[i];
      if (rRaw.size == sVec2f::Zero()) {
        nNumSelections--;
      }
    }
    if (!nNumSelections) {
      return;
    }

    niLoop(i,mlstLines.size()) {
      const sLine& pLine = mlstLines[i];
      if (pLine.words_empty())
        continue;
      const sWord& pFirstWord = mlstLineWords[pLine[0]];
      const tU32 nIndexBegin = pFirstWord.index;

      // tests
      if (nIndexBegin > anEnd) {
        break;
      }
      else if (anBegin < nIndexBegin) {
        anBegin = nIndexBegin;
      }
      const tU32 nIndexEnd = nIndexBegin + pLine.words_size();

      // lines contains words ?
      if (anBegin >= nIndexBegin && anBegin < nIndexEnd) {
        // we got the right line
        const tU32 nOffset = anBegin - nIndexBegin;

        // if we need to select more words than there are on this line then we
        // will need to go to the next one
        const tU32 nEndOfSelection = ((nOffset + nNumSelections) >= pLine.words_size()) ? pLine.words_size() : (nOffset + nNumSelections);
        for (tU32 j = nOffset; j < nEndOfSelection; j++) {
          _SelectWord(i, j);
          nNumSelections--;
        }
      }
    }
  }

 private:
  Ptr<iGraphics> mptrGraphics;
  Ptr<iFont> mptrDefaultFont;
  Ptr<iCallback> mptrLoadFontCallback;
  sVec2f mvSize;
  cString mstrText;
  Ptr<iExpressionContext> mptrExpressionContext;
  tBool mbKerning;
  tF32 mfContentsScale;

  struct sFontManager {
   public:
    sFontManager() {
      Clear();
    }
    ~sFontManager() {
      Clear();
    }
    void Clear() {
      mmapFonts.clear();
      mbCanProcess = eFalse;
    }
    iFont* GetFont(iHString* ahspName) {
      niProfileBlock(WidgetText_GetFont);
      tFonts::const_iterator pIt = mmapFonts.find(ahspName);
      if (pIt != mmapFonts.end()) {
        return pIt->second;
      }
      return mpParent->mptrDefaultFont;
    }
    void Init(cTextObject* apTextObject) {
      mmapFonts.clear();
      mstackFamily.clear();
      mstackColor.clear();
      mstackSize.clear();
      mstackRes.clear();
      mstackLineSpacing.clear();
      mstackBold.clear();
      mstackItalic.clear();
      mstackAlignment.clear();
      mpParent = apTextObject;
      mbCanProcess = eTrue;
    }
    void Continue() {
      mstackFamily.clear();
      mstackColor.clear();
      mstackSize.clear();
      mstackRes.clear();
      mstackLineSpacing.clear();
      mstackBold.clear();
      mstackItalic.clear();
      mstackAlignment.clear();
      mbCanProcess = eTrue;
    }

    // BEGIN and END will translate the tags into data
    // if you need to add new tags, you should start from here!
    void BeginTag(const cString& astrToken) {
      cString strToken = astrToken;
      cString strValue = _A("");
      if (astrToken.contains(_A("="))) {
        strToken = astrToken.Before(_A("="));
        strToken.Normalize();
        strValue = astrToken.After(_A("="));
        strValue.Normalize();
      }

      // hardcoded... I guess this could be better
      if (strToken == _A("b") || strToken == _A("bold")) {
        mstackBold.push_back(eTrue);
      }
      else if (strToken == _A("i") || strToken == _A("italic")) {
        mstackItalic.push_back(eTrue);
      }
      else if (strToken == _A("a") || strToken == _A("align")) {
        tU32 col = 0;
        tU32 align = eTextAlignment_Left;

        cString strAlign, strCol;
        if (strValue.contains(":")) {
          strCol = strValue.Before(":");
          strAlign = strValue.After(":");
        }
        else {
          strAlign = strValue;
        }

        if (strAlign == "right") {
          align = eTextAlignment_Right;
        }
        else if (strAlign == "center") {
          align = eTextAlignment_Center;
        }
        else if (strAlign == "justify") {
          align = eTextAlignment_Justify;
        }
        else if (strAlign == "justify_all") {
          align = eTextAlignment_JustifyAll;
        }
        else /*if (strAlign == "left")*/ {
          align = eTextAlignment_Left;
        }

        if (!strCol.IsEmpty()) {
          tColNameMap::const_iterator itColName = mpParent->mmapColNames.find(strCol);
          if (itColName == mpParent->mmapColNames.end()) {
            const tU32 maybeColID = strCol.Long();
            tColPosMap::const_iterator itColPos = mpParent->mmapColPos.find(maybeColID);
            if (itColPos == mpParent->mmapColPos.end()) {
              niWarning(niFmt("Can't find column '%s'.", strCol));
            }
            else {
              col = maybeColID;
            }
          }
          else {
            col = itColName->second;
          }
        }

        mstackAlignment.push_back(TA_BUILD(align,col));
      }
      else if (strToken == _A("f") || strToken == _A("family")) {
        mstackFamily.push_back(strValue);
      }
      else if (strToken == _A("c") || strToken == _A("color")) {
        TRACE_TEXT_OBJECT(("... Push color: %s", strValue));
        mstackColor.push_back(strValue);
      }
      else if (strToken == _A("s") || strToken == _A("size")) {
        mstackSize.push_back(strValue.Long());
      }
      else if (strToken == _A("r") || strToken == _A("res")) {
        mstackRes.push_back(strValue.Long());
      }
      else if (strToken == _A("l") || strToken == _A("line_spacing")) {
        mstackLineSpacing.push_back(strValue.Float());
      }
    }
    void EndTag(const cString& astrToken) {
      cString strToken = astrToken;
      cString strValue = _A("");
      if (astrToken.contains(_A("="))) {
        strToken = astrToken.Before(_A("="));
        strToken.Normalize();
        strValue = astrToken.After(_A("="));
        strValue.Normalize(); // remove the strings
      }
      if (strToken == _A("b") || strToken == _A("bold")) {
        if (!mstackBold.empty()) mstackBold.pop_back();
      }
      else if (strToken == _A("i") || strToken == _A("italic")) {
        if (!mstackItalic.empty()) mstackItalic.pop_back();
      }
      else if (strToken == _A("a") || strToken == _A("align")) {
        if (!mstackAlignment.empty()) mstackAlignment.pop_back();
      }
      else if (strToken == _A("f") || strToken == _A("family")) {
        if (!mstackFamily.empty()) mstackFamily.pop_back();
      }
      else if (strToken == _A("c") || strToken == _A("color")) {
        if (!mstackColor.empty()) mstackColor.pop_back();
      }
      else if (strToken == _A("s") || strToken == _A("size")) {
        if (!mstackSize.empty()) mstackSize.pop_back();
      }
      else if (strToken == _A("r") || strToken == _A("res")) {
        if (!mstackRes.empty()) mstackRes.pop_back();
      }
      else if (strToken == _A("l") || strToken == _A("line_spacing")) {
        if (!mstackLineSpacing.empty()) mstackLineSpacing.pop_back();
      }
    }

    Ptr<iFont> ProcessToken() {
      if (!mbCanProcess)
        return NULL;

      tHStringPtr hspFontName;
      {
        cString strFontName = mstackFamily.empty() ? _A("Default") : mstackFamily.back();
        if (!mstackBold.empty() && !strFontName.contains(_A("Bold")))
          strFontName += _A(" Bold");
        if (!mstackItalic.empty() && !strFontName.contains(_A("Italic")))
          strFontName += _A(" Italic");
        hspFontName = _H(strFontName);
      }

      const tU32 fontSize = (tU32)((mstackSize.empty() ? (tF32)16 : (tF32)mstackSize.back()));
      const tU32 fontRes = (tU32)((mstackRes.empty() ? (tF32)fontSize : (tF32)mstackRes.back()) * mpParent->mfContentsScale);
      const tU32 fontColor = mstackColor.empty() ? 0xFFFFFFFF :  ULColorBuild(
          mpParent->GetGraphics()->GetColor4FromName(_H(mstackColor.back())));
      const tF32 fontLineSpacing = mstackLineSpacing.empty() ? 1.0 : mstackLineSpacing.back();
      const tHStringPtr hspFontCacheKey = _H(niFmt(
        "%s-%d-%d-%x-%g",hspFontName,fontSize,fontRes,fontColor,fontLineSpacing));

      // check if the font has already been cached
      tFonts::const_iterator pIt = mmapFonts.find(hspFontCacheKey);
      if (pIt != mmapFonts.end()) {
        return pIt->second;
      }

      // font hasnt been cached, load it
      QPtr<iFont> ptrBaseFont;
      if (mpParent->mptrLoadFontCallback.IsOK()) {
        ptrBaseFont = mpParent->mptrLoadFontCallback->RunCallback(hspFontName,niVarNull);
      }
      else {
        ptrBaseFont = mpParent->mptrGraphics->LoadFont(hspFontName);
      }
      if (!ptrBaseFont.IsOK()) {
        ptrBaseFont = mpParent->mptrDefaultFont.ptr();
      }

      // setup the font
      {
        Ptr<iFont> ptrFont = ptrBaseFont->CreateFontInstance(NULL);
        ptrFont->SetSizeAndResolution(Vec2f((tF32)fontSize, (tF32)fontSize),fontRes,mpParent->mfContentsScale);
        ptrFont->SetColor(fontColor);
        ptrFont->SetLineSpacing(fontLineSpacing);
        astl::upsert(mmapFonts, hspFontCacheKey, ptrFont);
        TRACE_TEXT_OBJECT(("... font loaded '%s', key '%s'", ptrFont->GetName(), hspFontCacheKey));
        return ptrFont;
      }
    }

   public:
    cTextObject* mpParent;
    typedef astl::hstring_hash_map<Ptr<iFont> > tFonts;
    tFonts mmapFonts;
    tBool mbCanProcess;
    astl::vector<tU32>    mstackSize;
    astl::vector<tU32>    mstackRes;
    astl::vector<tU32>    mstackAlignment;
    astl::vector<cString> mstackTags;
    astl::vector<cString> mstackColor;
    astl::vector<cString> mstackFamily;
    astl::vector<tBool>   mstackBold;
    astl::vector<tBool>   mstackItalic;
    astl::vector<tF32>    mstackLineSpacing;
  } mFontManager;

  struct sExpression : public ImplRC<iUnknown> {
    cString mstrExpr;
    Ptr<iExpression> mptrExpr;
    Ptr<iExpressionVariable> mptrResult;
    tU32 mnWordIndex;

    tBool UpdateExpression() {
      if (mptrResult.IsOK() && niFlagIs(mptrResult->GetFlags(),eExpressionVariableFlags_Constant)) {
        return eFalse;
      }
      if (mptrExpr.IsOK()) {
        mptrResult = mptrExpr->Eval();
      }
      return eTrue;
    }

    cString ResultToString() {
      cString r;
      if (!mstrExpr.empty()) {
        r += mstrExpr;
        r += " = ";
      }
      if (mptrResult.IsOK()) {
        r += mptrResult->GetString();
      }
      else {
        r += "#EVALERROR#";
      }
      return r;
    }
  };

  struct sRawString {
    sRawString(tU32 aTextAlignment) {
      selected = eFalse;
      selectionID = eInvalidHandle;
      size = sVec2f::Zero();
      alignment = aTextAlignment;
    }

    void InitExpression(iExpressionContext* apExpressionContext, const achar* aaszExpr, tBool abPrintExpr, tBool abKerning) {
      expr = niNew sExpression();
      expr->mptrExpr = apExpressionContext->CreateExpression(aaszExpr);
      if (abPrintExpr) {
        expr->mstrExpr = aaszExpr;
      }
      UpdateExpression(abKerning);
    }

    tBool UpdateExpression(const tBool abKerning) {
      if (!font.IsOK()) {
        return eFalse;
      }
      if (!expr.IsOK()) {
        return eTrue;
      }
      const tBool exprUpdated = expr->UpdateExpression();
      if (exprUpdated) {
        this->string = expr->ResultToString();
        this->size = Vec2f(
          TextLineMetric_ComputeRect(font, string, abKerning).GetWidth(),
          TextLineMetric_ComputeLineHeight(font));
      }
      return eTrue;
    }

    Ptr<iFont>  font;
    cString     string;
    cString     truncatedString;
    sVec2f      size;
    sTextLineMetric textMetric;
    tU32        selectionID;
    tBool       selected;
    tU32        alignment;
    Ptr<sExpression> expr;
  };
  typedef astl::vector<sRawString> tRawStrings;
  tRawStrings mlstWords;

  struct sWord {
    sWord() {
      index = 0;
      segment = 0;
      width = 0.0f;
      x_pos = 0.0f;
      selected = eFalse;
      font = NULL;
      alignment = eTextAlignment_Left;
    }
    ~sWord() {
    }
    tU32  index;
    tU32  segment;
    tF32  width;
    tF32  x_pos;
    tU32  alignment;
    tBool selected;
    Ptr<iFont> font;
  };
  struct sLine {
    sLine() {
      clear();
    }
    ~sLine() {
    }
    tF32      y_pos;
    tF32      width;
    tF32      height;
    tF32      ascent;
    tU32      firstWord;
    tU32      numWords;
    // the segments are the spaces
    // where the rendering should be done
    astl::vector<sVec4f> segments;
    void clear() {
      y_pos = 0.0f;
      width = height = ascent = 0;
      firstWord = eInvalidHandle;
      numWords = 0;
      segments.clear();
    }
    inline bool words_empty() const {
      return numWords == 0;
    }
    inline void words_push_back(const sWord& w, astl::vector<sWord>& wordList) {
      const tU32 i = (tU32)wordList.size();
      if (firstWord == eInvalidHandle) {
        firstWord = i;
      }
      ++numWords;
      wordList.push_back(w);
    }
    inline tSize words_size() const {
      return numWords;
    }
    inline tU32 operator [] (tU32 anIndex) const {
      return firstWord+anIndex;
    }
  };
  astl::vector<sLine> mlstLines;
  astl::vector<sWord> mlstLineWords;
  sVec2f mTextSize;

  // selection
  struct sSelection {
    tU32 index;
    tU32 line;
    tU32 word;
  };
  astl::vector<sSelection> mlstSelections;
  sVec4f                   mvSelectionColor;
  eTextTruncation mnTruncationStyle;
  cString         mstrTrunc;
  tU32            mnDirty;
  tBool           mbTrimLeadingSpaces;

  tBool mbAddingText;
  tU32  mnLastWordIndex;
  tF32  mfCurrentTotalHeight;

  // Occluders
  typedef astl::vector<Ptr<iTextOccluder> > tOccluders;
  tOccluders mvOccluders;

  // Columns
  typedef astl::hash_map<tU32,tF32> tColPosMap;
  typedef astl::hash_map<cString,tU32> tColNameMap;
  tColPosMap  mmapColPos;
  tColNameMap mmapColNames;

  enum eTagType {
    eTagType_Unknown = 0,
    eTagType_Font = 1,
    eTagType_Alignment = 2,
    eTagType_Expression = 3,
    eTagType_Col = 4,
    eTagType_Glyph = 5,
  };

  enum eFontTagAttr {
    eFontTagAttr_Name = niBit(0),
    eFontTagAttr_Color = niBit(1),
    eFontTagAttr_Size = niBit(2),
    eFontTagAttr_MaxRes = niBit(3),
    eFontTagAttr_LineSpacing = niBit(4)
  };

  struct sParseTagsXmlParserSink : public ImplRC<iXmlParserSink> {
    cTextObject* _this;
    cTextObject::tRawStrings& mapStrings; // tag-free string list
    astl::stack<eTagType> tagType;
    astl::stack<tU32>     tagValue;
    astl::stack<cString>  expr;
    astl::stack<tU32>     fontAttrs;
    astl::stack<tU32>     colAttrs;

    sParseTagsXmlParserSink(cTextObject* apThis, cTextObject::tRawStrings& aMapStrings)
        : _this(apThis)
        , mapStrings(aMapStrings)
    {
      tagType.push(eTagType_Unknown);
    }

    virtual tBool __stdcall OnXmlParserSink_Node(
        eXmlParserNodeType aType, const ni::achar* aName)
    {
      TRACE_TEXT_OBJECT((".... OnXmlParserSink_Node: %d: %s", aType, aName));
      switch (aType) {
        case eXmlParserNodeType_ElementBegin:
          if (StrIEq(aName,"font") || StrIEq(aName,"f")) {
            tagType.push(eTagType_Font);
            fontAttrs.push(0);
          }
          else if (StrIEq(aName,"a") || StrIEq(aName,"align")) {
            tagType.push(eTagType_Alignment);
            tagValue.push(0);
          }
          else if (StrIEq(aName,"e") || StrIEq(aName,"expr")) {
            tagType.push(eTagType_Expression);
            expr.push("");
            tagValue.push(0);
          }
          else if (StrIEq(aName,"g") || StrIEq(aName,"glyph")) {
            tagType.push(eTagType_Glyph);
            tagValue.push(0);
          }
          else {
            tagType.push(eTagType_Unknown);
            if (StrIEq(aName,"b") || StrIEq(aName,"bold")) {
              _this->mFontManager.BeginTag("b");
            }
            else if (StrIEq(aName,"i") || StrIEq(aName,"italic")) {
              _this->mFontManager.BeginTag("i");
            }
          }
          break;
        case eXmlParserNodeType_ElementEnd:
          if (StrIEq(aName,"font") || StrIEq(aName,"f")) {
            if (!fontAttrs.empty()) {
              const tU32 lastFontAttrs = fontAttrs.top();
              if (lastFontAttrs & eFontTagAttr_LineSpacing) {
                _this->mFontManager.EndTag("l");
              }
              if (lastFontAttrs & eFontTagAttr_MaxRes) {
                _this->mFontManager.EndTag("r");
              }
              if (lastFontAttrs & eFontTagAttr_Size) {
                _this->mFontManager.EndTag("s");
              }
              if (lastFontAttrs & eFontTagAttr_Color) {
                _this->mFontManager.EndTag("c");
              }
              if (lastFontAttrs & eFontTagAttr_Name) {
                _this->mFontManager.EndTag("f");
              }
              fontAttrs.pop();
            }
          }
          else if (StrIEq(aName,"a") || StrIEq(aName,"align")) {
            if (tagValue.top() != eInvalidHandle) {
              _this->mFontManager.EndTag("a");
            }
            tagValue.pop();
          }
          else if (StrIEq(aName,"g") || StrIEq(aName,"glyph")) {
            tagValue.pop();
          }
          else if (StrIEq(aName,"e") || StrIEq(aName,"expr")) {
            if (!expr.top().empty()) {
              if (!_this->mptrExpressionContext.IsOK()) {
                _this->mptrExpressionContext = ni::GetLang()->GetExpressionContext();
              }

              sRawString s(
                  _this->mFontManager.mstackAlignment.empty() ?
                  eTextAlignment_Left : _this->mFontManager.mstackAlignment.back());
              s.InitExpression(_this->mptrExpressionContext, expr.top().Chars(), !!tagValue.top(), _this->mbKerning);
              s.font = _this->mFontManager.ProcessToken();
              s.string = "#EXPR#";
              mapStrings.push_back(s);
            }
            tagValue.pop();
            expr.pop();
          }
          else {
            if (StrIEq(aName,"b") || StrIEq(aName,"bold")) {
              _this->mFontManager.EndTag("b");
            }
            else if (StrIEq(aName,"i") || StrIEq(aName,"italic")) {
              _this->mFontManager.EndTag("i");
            }
          }
          tagType.pop();
          break;
        case eXmlParserNodeType_EmptyText:
          TRACE_TEXT_OBJECT(("... EMPTY TEXT: |%d| %s", ni::StrLen(aName), aName));
          niFallthrough;
        case eXmlParserNodeType_Text: {
          TRACE_TEXT_OBJECT(("... TEXT: |%d| %s", ni::StrLen(aName), aName));
          sRawString s(_this->mFontManager.mstackAlignment.empty() ?
                       eTextAlignment_Left : _this->mFontManager.mstackAlignment.back());
          s.font = _this->mFontManager.ProcessToken();
          if (aType == eXmlParserNodeType_EmptyText) {
            s.string = aName;
          }
          else {
            StringDecodeXml(s.string,aName);
          }
          if (tagType.top() == eTagType_Expression) {
            expr.top() += s.string;
          }
          else {
            mapStrings.push_back(s);
          }
          break;
        }
        case eXmlParserNodeType_Comment:
          break;
        case eXmlParserNodeType_CDATA:
          // niDebugFmt(("... CDATA: %s", aName));
          sRawString s(_this->mFontManager.mstackAlignment.empty() ?
                       eTextAlignment_Left : _this->mFontManager.mstackAlignment.back());
          s.font = _this->mFontManager.ProcessToken();
          s.string = aName;
          if (tagType.top() == eTagType_Expression) {
            expr.top() += s.string;
          }
          else {
            mapStrings.push_back(s);
          }
          break;
      }
      return eTrue;
    }
    virtual tBool __stdcall OnXmlParserSink_Attribute(
        const ni::achar* aName,
        const ni::achar* aValue)
    {
      TRACE_TEXT_OBJECT((".... OnXmlParserSink_Attribute: %d: '%s' = '%s'", tagType.top(), aName, aValue));
      switch (tagType.top()) {
        case eTagType_Unknown:
        case eTagType_Col:
          break;
        case eTagType_Font: {
          if (StrIEq(aName,"name")) {
            _this->mFontManager.BeginTag(niFmt("f=%s",aValue));
            if (!fontAttrs.empty())
              fontAttrs.top() |= eFontTagAttr_Name;
          }
          else if (StrIEq(aName,"color")) {
            _this->mFontManager.BeginTag(niFmt("c=%s",aValue));
            if (!fontAttrs.empty())
              fontAttrs.top() |= eFontTagAttr_Color;
          }
          else if (StrIEq(aName,"size")) {
            _this->mFontManager.BeginTag(niFmt("s=%s",aValue));
            if (!fontAttrs.empty())
              fontAttrs.top() |= eFontTagAttr_Size;
          }
          else if (StrIEq(aName,"res")) {
            _this->mFontManager.BeginTag(niFmt("r=%s",aValue));
            if (!fontAttrs.empty())
              fontAttrs.top() |= eFontTagAttr_MaxRes;
          }
          else if (StrIEq(aName,"line_spacing")) {
            _this->mFontManager.BeginTag(niFmt("l=%s",aValue));
            if (!fontAttrs.empty())
              fontAttrs.top() |= eFontTagAttr_LineSpacing;
          }
          break;
        }
        case eTagType_Alignment: {
          TRACE_TEXT_OBJECT((".... eTagType_Alignment Attr: '%s' = '%s'", aName, aValue));
          if (StrEq(aName,aValue)) {
            tagValue.top() = 1;
            _this->mFontManager.BeginTag(niFmt("a=%s",aValue));
          }
          else if (StrEq(aName,"left") ||
                   StrEq(aName,"right") ||
                   StrEq(aName,"center") ||
                   StrEq(aName,"justify") ||
                   StrEq(aName,"justify_all"))
          {
            tagValue.top() = 1;
            _this->mFontManager.BeginTag(niFmt("a=%s:%s",aValue,aName));
          }
          else {
            tagValue.top() = 1;
            _this->mFontManager.BeginTag(niFmt("a=%s:%s",aName,aValue));
          }
          break;
        }
        case eTagType_Expression: {
          if (StrIEq(aName,"print")) {
            tagValue.top() = 1;
          }
          break;
        }
        case eTagType_Glyph: {
          if (StrIEq(aName,"name")) {
            TRACE_TEXT_OBJECT(("... eTagType_Glyph fromName: %s", aValue));
            Ptr<iFont> font = _this->mFontManager.ProcessToken();
            const tU32 glyphCodePoint = font->GetGlyphCodePointFromName(aValue);
            if (glyphCodePoint) {
              sRawString s(_this->mFontManager.mstackAlignment.empty() ?
                           eTextAlignment_Left : _this->mFontManager.mstackAlignment.back());
              s.font = font;
              s.string.appendChar(glyphCodePoint);
              mapStrings.push_back(s);
            }
          }
          break;
        }
      }
      return eTrue;
    }
  };

  niEndClass(cTextObject);
};

Ptr<iTextObject> __stdcall cGraphics::CreateTextObject(const achar* aaszText,const sVec2f& avSize, const tF32 afContentsScale) {
  Ptr<iTextObject> ptrTextObject = niNew cTextObject(this,avSize,afContentsScale);
  ptrTextObject->SetText(aaszText);
  return ptrTextObject;
}
