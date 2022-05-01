#ifndef __PROF_H_56D56BD3_45CB_4259_AD82_238933EE7E9B__
#define __PROF_H_56D56BD3_45CB_4259_AD82_238933EE7E9B__

#define Prof_ENABLED

#if !defined __NI_PROFGATHER_H__ || !defined niProfiler

#undef __NI_PROFGATHER_H__
#undef niProf
#undef niProf_Scope
#undef niProf_Define
#undef niProf_Begin
#undef niProf_End
#undef niProf_Region
#undef niProf_Counter
#undef niProfileBlock
#undef niProfileEnter
#undef niProfileLeave

#if defined Prof_ENABLED && !defined niProfiler
#define niProfiler
#endif

#include "API/niLang/Utils/ProfGather.h"
#endif

#if defined niProfiler

/*
 *  Prof_update
 *
 *  Pass in true (1) to accumulate history info; pass
 *  in false (0) to throw away the current frame's data
 */
void Prof_update(int record);

void Prof_set_report_mode(ni::eProfilerReportMode e);
ni::eProfilerReportMode Prof_get_report_mode();
void Prof_move_cursor(int delta);
void Prof_select(void);
void Prof_select_parent(void);
void Prof_move_frame(int delta);

void Prof_set_smoothing(int smoothing_mode);
void Prof_set_frame(int frame);
void Prof_set_cursor(int line);


double Prof_get_time(void);
void Prof_get_timestamp(ni::tI64* result);

typedef ni::sProfilerZone Prof_Zone;
typedef ni::sProfilerZoneStack Prof_Zone_Stack;

// number of unique zones allowed in the entire application
// @TODO: remove MAX_PROFILING_ZONES and make it dynamic
#define MAX_PROFILING_ZONES                4096

// report functions

#define NUM_VALUES 4
#define NUM_TITLE 2
#define NUM_HEADER (NUM_VALUES+1)

typedef struct {
  int indent;
  const char *name;
  int number;
  char prefix;
  int value_flag;
  double values[NUM_VALUES];
  double heat;

  // used internally
  void *zone;
} Prof_Report_Record;

typedef struct
{
  char *title[NUM_TITLE];
  char *header[NUM_HEADER];
  int num_record;
  int hilight;
  Prof_Report_Record *record;
} Prof_Report;

void         Prof_free_report(Prof_Report *z);
Prof_Report *Prof_create_report(void);

// really internal functions

void Prof_graph(int num_frames,
                void (*callback)(int id, int x0, int x1, float *values, void *data),
                void *data);

void Prof_init_highlevel();

extern int        Prof_num_zones;
extern ni::sProfilerZone *Prof_zones[MAX_PROFILING_ZONES];
extern niProf_Declare(_global);

extern Prof_Zone_Stack* Prof_stack; // current Zone stack
extern Prof_Zone_Stack  Prof_dummy; // parent never matches
extern Prof_Zone_Stack* Prof_StackAppend(Prof_Zone *zone);

int Prof_get_colors(float factor,
                    float text_color_ret[3],
                    float glow_color_ret[3],
                    float *glow_alpha_ret);
void Prof_float_to_string(char *buf, float num, int precision);

/*
 *  Prof_draw_ni -- display the current report via iProfDraw
 *
 *  You must provide a callable text-printing function.
 *  Put the opengl state into a 2d rendering mode.
 *
 *  Parameters:
 *    <sx,sy>         --  location where top line is drawn
 *    <width, height> --  total size of display (if too small, text will overprint)
 *    line_spacing    --  how much to move sy by after each line; use a
 *                        negative value if y decreases down the screen
 *    precision       --  decimal places of precision for time data, 1..4 (try 2)
 *    print_text      --  function to display a line of text starting at the
 *                        given coordinate; best if 0,1..9 are fixed-width
 *    text_width      --  a function that computes the pixel-width of
 *                        a given string before printing. you can fake with a
 *                        simple approximation of width('0')*strlen(str)
 *
 *  to avoid overprinting, you can make print_text truncate long strings
 */
void Prof_draw(
    ni::iProfDraw* drawer,
    float sx, float sy,
    float full_width, float height, int precision);

/*
 *  Parameters
 *    <sx, sy>      --  origin of the graph--location of (0,0)
 *    x_spacing     --  screenspace size of each history sample; e.g.
 *                         2.0 pixels
 *    y_spacing     --  screenspace size of one millisecond of time;
 *                         for an app with max of 20ms in any one zone,
 *                         8.0 would produce a 160-pixel tall display,
 *                         assuming screenspace is in pixels
 */
void Prof_draw_graph(
    ni::iProfDraw* drawer, float sx, float sy,
    float x_spacing, float y_spacing);

void Prof_draw_text(
    void* context,
    float sx, float sy,
    float full_width, float height,
    float line_spacing, int precision,
    void (*begin_rects)(void* context),
    void (*end_rects)(void* context),
    void (*draw_rectangle)(void* context,
                           float x0, float y0,
                           float x1, float y1,
                           ni::sVec4f& avCol),
    void (*beginText)(void* c),
    void (*endText)(void* c),
    void (*printText)(void* c,
                      float x, float y, const char *str,
                      const ni::sColor4f& avCol),
    float (*textWidth)(void* c, const char *str));

#endif

#endif // __PROF_H_56D56BD3_45CB_4259_AD82_238933EE7E9B__
