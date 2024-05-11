#include "stdafx.h"
#include "prof.h"

using namespace ni;

#if defined niProfiler

class cProfiler : public ImplLocal<ni::iProf>
{
  tBool mbRecord;

 public:
  cProfiler() {
    mbRecord = eTrue;
  }

  virtual void __stdcall SetRecord(ni::tBool abRecord) {
    mbRecord = abRecord;
  }
  virtual tBool __stdcall GetRecord() const {
    return mbRecord;
  }

  virtual void __stdcall Update() {
    Prof_update(mbRecord?1:0);
  }

  virtual ni::sProfilerZoneStack* __stdcall GetZoneStackDummy() const {
    return &Prof_dummy;
  }
  virtual ni::sProfilerZoneStack** __stdcall GetZoneStack() const {
    return &Prof_stack;
  }
  virtual void __stdcall SetZoneStack(ni::sProfilerZoneStack* apZoneStack) {
    Prof_stack = apZoneStack;
  }
  virtual ni::sProfilerZoneStack* __stdcall StackAppend(ni::sProfilerZone* apZoneStack) {
    return Prof_StackAppend(apZoneStack);
  }
  virtual void __stdcall GetTimeStamp(tI64* apTimeStamp) const {
    Prof_get_timestamp(apTimeStamp);
  }

  virtual void __stdcall SetReportMode(ni::eProfilerReportMode aMode) {
    Prof_set_report_mode(aMode);
  }
  virtual ni::eProfilerReportMode __stdcall GetReportMode() {
    return Prof_get_report_mode();
  }

  virtual void __stdcall InputMoveCursor(ni::tI32 anDelta) {
    Prof_move_cursor(anDelta);
  }
  virtual void __stdcall InputSelect() {
    Prof_select();
  }
  virtual void __stdcall InputSelectParent() {
    Prof_select_parent();
  }
  virtual void __stdcall InputMoveFrame(ni::tI32 anDelta) {
    Prof_move_frame(anDelta);
  }
  virtual void __stdcall InputSetFrame(ni::tI32 anFrame) {
    Prof_set_frame(anFrame);
  }
  virtual void __stdcall InputSetCursor(ni::tI32 anLine) {
    Prof_set_cursor(anLine);
  }

  virtual tBool __stdcall DrawTable(
      ni::iProfDraw* drawer,
      tF32 sx, tF32 sy,
      tF32 full_width, tF32 height,
      tI32 precision)
  {
    niCheckIsOK(drawer,eFalse);
    Prof_draw(drawer,
              sx,sy,full_width,height,
              precision);
    return eTrue;
  }

  virtual tBool __stdcall DrawGraph(
      ni::iProfDraw* drawer, tF32 sx, tF32 sy,
      tF32 x_spacing, tF32 y_spacing)
  {
    niCheckIsOK(drawer,eFalse);
    Prof_draw_graph(drawer,sx,sy,x_spacing,y_spacing);
    return eTrue;
  }

  virtual cString __stdcall TextReport(tU32 cols, tU32 rows) {
    if (cols <= 8 || rows <= 0)
      return AZEROSTR;
    struct sTextReport {
      tU32 _cols, _rows;
      astl::vector<achar> _txt;
      sTextReport(tU32 cols, tU32 rows) {
        _cols = ni::Min(cols,1024);
        _rows = ni::Min(rows,1024);
        _txt.resize(_cols*_rows);
        astl::fill(_txt,' ');
      }
      void putChar(tU32 x, tU32 y, achar c) {
        if (x >= _cols) return;
        if (y >= _rows) return;
        _txt[(y*_cols)+x] = c;
      }
      cString getLine(tU32 y) {
        if (y >= _rows) return AZEROSTR;
        astl::vector<achar> row; row.resize(_cols);
        memcpy(row.data(),&_txt[y*_cols],(row.size()-1)*sizeof(row[0]));
        niLoopr(ri,row.size()-1) {
          if (row[ri] != ' ') {
            return cString(row.data(),row.data()+ri+1);
          }
        }
        return AZEROSTR;
      }
    } text(cols,rows);
    struct _Local {
      static void printText(void* c,
                            float x, float y, const char *str,
                            const sColor4f& avCol) {
        sTextReport* r = (sTextReport*)c;
        const char* p = str;
        const tU32 my = (tU32)y;
        tU32 mx = (tU32)x;
        while (*p) {
          r->putChar(mx,my,*p);
          ++mx;
          ++p;
        }
      }
      static float textWidth(void* c, const char *str) {
        return (float)ni::StrLen(str);
      }
    };

    Prof_draw_text((void*)&text,
                   0,0,
                   (float)cols,(float)rows,
                   1,5,
                   NULL,NULL,NULL,
                   NULL,NULL,
                   _Local::printText,
                   _Local::textWidth);

    cString txt;
    niLoop(i,text._rows) {
      cString line = text.getLine(i);
      if (line.empty())
        break;
      txt += line + "\n";
    }

    text._txt[text._txt.size()-1] = 0;
    return txt;
  }
};

///////////////////////////////////////////////
niExportFunc(ni::iProf*) GetProf()
{
  static cProfiler _profiler;
  return &_profiler;
}

#else

niExportFunc(ni::iProf*) GetProf()
{
  niDoAssertUnreachable("Profiler not implemented.");
  return NULL;
}

#endif
