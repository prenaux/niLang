#include "stdafx.h"
#include "prof.h"
#include <stdio.h>

#if defined niProfiler

using namespace ni;

// float to string conversion with sprintf() was
// taking up 10-20% of the Prof_draw time, so I
// wrote a faster float-to-string converter

static char int_to_string[100][4] = {{0},{0}};
static char int_to_string_decimal[100][4] = {{0},{0}};
static char int_to_string_mid_decimal[100][4] = {{0},{0}};
static void int_to_string_init(void)
{
  int i;
  for (i=0; i < 100; ++i) {
    sprintf(int_to_string[i], "%d", i);
    sprintf(int_to_string_decimal[i], ".%02d", i);
    sprintf(int_to_string_mid_decimal[i], "%d.%d", i/10, i % 10);
  }
}

static const char *formats[5] =
{
  "%.0f",
  "%.1f",
  "%.2f",
  "%.3f",
  "%.4f",
};

void Prof_float_to_string(char *buf, float num, int precision)
{
  int x,y;
  switch(precision) {
    case 2:
      if (num < 0 || num >= 100)
        break;
      x = (int)num;
      y = (int)((num - (float)x) * 100.0f);
      strcpy(buf, int_to_string[x]);
      strcat(buf, int_to_string_decimal[y]);
      return;
    case 3:
      if (num < 0 || num >= 10)
        break;
      num *= 10;
      x = (int)num;
      y = (int)((num - (float)x) * 100.0f);
      strcpy(buf, int_to_string_mid_decimal[x]);
      strcat(buf, int_to_string_decimal[y]+1);
      return;
    case 4:
      if (num < 0 || num >= 1)
        break;
      num *= 100;
      x = (int)num;
      y = (int)((num - (float)x) * 100.0f);
      buf[0] = '0';
      strcpy(buf+1, int_to_string_decimal[x]);
      strcat(buf, int_to_string_decimal[y]+1);
      return;
  }
  sprintf(buf, formats[precision], num);
}

// use factor to compute a glow amount
int Prof_get_colors(float factor,
                    float text_color_ret[3],
                    float glow_color_ret[3],
                    float *glow_alpha_ret)
{
  const float GLOW_RANGE = 0.5f;
  const float GLOW_ALPHA_MAX = 0.5f;
  float glow_alpha;
  int i;
  float hot[3] = {1.0f, 1.0f, 0.9f};
  float cold[3] = {0.15f, 0.9f, 0.15f};

  float glow_cold[3] = {0.5f, 0.5f, 0};
  float glow_hot[3] = {1.0f, 1.0f, 0};

  if (factor < 0) factor = 0;
  if (factor > 1) factor = 1;

  for (i=0; i < 3; ++i)
    text_color_ret[i] = cold[i] + (hot[i] - cold[i]) * factor;

  // Figure out whether to start up the glow as well.
  glow_alpha = (factor - GLOW_RANGE) / (1 - GLOW_RANGE);
  if (glow_alpha < 0) {
    *glow_alpha_ret = 0;
    return 0;
  }

  for (i=0; i < 3; ++i)
    glow_color_ret[i] = glow_cold[i] + (glow_hot[i] - glow_cold[i]) * factor;

  *glow_alpha_ret = glow_alpha * GLOW_ALPHA_MAX;
  return 1;
}

typedef struct
{
  iProfDraw* drawer;
  float x0,y0;
  float sx,sy;
} GraphLocation;

static void graph_func(int id, int x0, int x1, float *values, void *data)
{
  GraphLocation *loc = (GraphLocation *) data;
  int i, r,g,b;

  // trim out values that are under 0.2 ms to accelerate rendering
  while (x0 < x1 && (*values < 0.0002f)) { ++x0; ++values; }
  while (x1 > x0 && (values[x1-1-x0] < 0.0002f)) --x1;

  tU32 lineColor;
  if (id == 0) {
    lineColor = ULColorBuildf(1,1,1,0.5);
  }
  else {
    if (x0 == x1) return;
    id = (id >> 8) + id;
    r = id * 37;
    g = id * 59;
    b = id * 45;
    lineColor = ULColorBuild((r & 127) + 80, (g & 127) + 80, (b & 127) + 80);
  }

  float px, py;
  if (x0 == x1) {
    float x,y;
    x = loc->x0 + x0 * loc->sx;
    y = loc->y0 + values[0] * loc->sy;
    loc->drawer->DrawLine(x,loc->y0,x,y,lineColor);
    px = x; py = y;
  }
  else {
    px = loc->x0 + x0 * loc->sx;
    py = loc->y0 + values[0] * loc->sy;
  }
  for (i=0; i < x1-x0; ++i) {
    float x,y;
    x = loc->x0 + (i+x0) * loc->sx;
    y = loc->y0 + values[i] * loc->sy;
    loc->drawer->DrawLine(px,py,x,y,lineColor);
    px = x; py = y;
  }
}

void Prof_draw_graph(
    iProfDraw* drawer, float sx, float sy,
    float x_spacing, float y_spacing)
{
#ifdef Prof_ENABLED
  niProf_Begin(iprof_draw_graph);
  GraphLocation loc = { drawer, sx, sy, x_spacing, - y_spacing * 1000 };
  drawer->BeginDraw(eFalse);
  Prof_graph(128, graph_func, &loc);
  drawer->EndDraw();
  niProf_End();
#endif
}

// Make sure you're in 2d rendering mode before calling this function
void Prof_draw(
    iProfDraw* drawer,
    float sx, float sy,
    float full_width, float height,
    int precision)
{
#ifdef Prof_ENABLED
  niProf_Begin(iprof_draw);

  static const tU32 knLineColorA = ULColorBuildf(0.1f, 0.3f, 0.0f, 0.85f);
  static const tU32 knLineColorB = ULColorBuildf(0.2f, 0.1f, 0.1f, 0.85f);
  static const tU32 knTextColorA = ULColorBuildf(0.6f, 0.4f, 0.0f, 1.0f);
  static const tU32 knTextColorB = ULColorBuildf(0.8f, 0.1f, 0.1f, 1.0f);

  int i,j,n,o;
  float backup_sy = sy;
  const float field_width = drawer->GetTextWidth("MMMMM.MM");
  const float name_width  = full_width - field_width * 3;
  const float plus_width  = drawer->GetTextWidth("+");
  const float line_spacing = drawer->GetTextHeight();

  int max_records;

  Prof_Report *pob;

  if (!int_to_string[0][0]) int_to_string_init();

  if (precision < 1) precision = 1;
  if (precision > 4) precision = 4;

  pob = Prof_create_report();
  // First iteration draw rectangles
  drawer->BeginDraw(eTrue);
  sy = backup_sy;
  for (i=0; i < NUM_TITLE; ++i) {
    if (pob->title[i]) {
      float header_x0 = sx;
      float tw = drawer->GetTextWidth(pob->title[i]) + 3.0f;
      float header_x1 = header_x0 + ni::Max(full_width,tw);
      drawer->DrawRect(header_x0, sy-2, header_x1, sy+line_spacing+2,
                       (i == 0) ? knLineColorA : knLineColorB);
      sy += 1.5f*line_spacing;
      height -= ni::Abs(line_spacing)*1.5f;
    }
  }
  drawer->EndDraw();

  // Second iteration draw text
  drawer->BeginText();
  sy = backup_sy;
  for (i=0; i < NUM_TITLE; ++i) {
    if (pob->title[i]) {
      drawer->Text(
          sx+2, sy, pob->title[i],
          (i == 0) ? knTextColorA : knTextColorB);
      sy += 1.5f*line_spacing;
      height -= ni::Abs(line_spacing)*1.5f;
    }
  }
  drawer->EndText();

  max_records = (int)(height / ni::Abs(line_spacing));

  o = 0;
  n = pob->num_record;
  if (n > max_records) n = max_records;
  if (pob->hilight >= o + n) {
    o = pob->hilight - n + 1;
  }

  backup_sy = sy;

  // Draw the background colors for the zone data.
  drawer->BeginDraw(eTrue);
  static const tU32 knRectColor0 = ULColorBuildf(0.0f,0.0f,0.0f,0.85f);
  static const tU32 knRectColorA = ULColorBuildf(0.1f,0.1f,0.2f,0.85f);
  static const tU32 knRectColorB = ULColorBuildf(0.1f,0.1f,0.3f,0.85f);
  static const tU32 knRectColorH = ULColorBuildf(0.3f,0.3f,0.1f,0.85f);
  drawer->DrawRect(sx, sy, sx + full_width, sy + line_spacing, knRectColor0);
  sy += line_spacing;
  for (i = 0; i < n; i++) {
    float y0 = sy;
    float y1 = sy + line_spacing;
    drawer->DrawRect(sx, y0, sx + full_width, y1,
                     (i+0 == pob->hilight) ? knRectColorH :
                     ((i&1) ? knRectColorA : knRectColorB));
    sy += line_spacing;
  }
  drawer->EndDraw();

  // Draw the text
  drawer->BeginText();

  sy = backup_sy;
  static const tU32 knTextColorBright = ULColorBuildf(0.7f,0.7f,0.7f,1.0f);
  if (pob->header[0]) {
    drawer->Text(sx+8, sy, pob->header[0], knTextColorBright);
  }
  for (j=1; j < NUM_HEADER; ++j) {
    if (pob->header[j]) {
      drawer->Text(
          sx + name_width + field_width * (j-1) +
          field_width/2 - drawer->GetTextWidth(pob->header[j])/2,
          sy, pob->header[j], knTextColorBright);
    }
  }

  sy += line_spacing;

  for (i = 0; i < n; i++) {
    char buf[256], *b = buf;
    Prof_Report_Record *r = &pob->record[i+o];
    float text_color[3], glow_color[3];
    float glow_alpha;
    float x = sx + drawer->GetTextWidth(" ") * r->indent + plus_width/2;
    if (r->prefix) {
      buf[0] = r->prefix;
      ++b;
    } else {
      x += plus_width;
    }
    if (r->number)
      sprintf(b, "%s (%d)", r->name, r->number);
    else
      sprintf(b, "%s", r->name);

    if (Prof_get_colors((float)r->heat, text_color, glow_color, &glow_alpha)) {
      const tU32 gc = ULColorBuildf(glow_color[0], glow_color[1], glow_color[2], glow_alpha);
      drawer->Text(x+2, sy-1, buf, gc);
    }

    const tU32 textColor = ULColorBuildf(text_color[0],text_color[1],text_color[2],1.0f);
    drawer->Text(x + 1, sy, buf, textColor);

    for (j=0; j < NUM_VALUES; ++j) {
      if (r->value_flag & (1 << j)) {
        int pad;
        Prof_float_to_string(buf, (float)r->values[j], j == 2 ? 2 : precision);
        pad = (int)(field_width - plus_width - drawer->GetTextWidth(buf));
        if (r->indent) pad += (int)plus_width;
        drawer->Text(
            sx + pad + name_width + field_width * j, sy, buf, textColor);
      }
    }


    sy += line_spacing;
  }

  drawer->EndText();

  Prof_free_report(pob);
  niProf_End();
#endif
}

// Make sure you're in 2d rendering mode before calling this function
void Prof_draw_text(void* context,
                    float sx, float sy,
                    float full_width, float height,
                    float line_spacing, int precision,
                    void (*begin_rects)(void* context),
                    void (*end_rects)(void* context),
                    void (*draw_rectangle)(void* context,
                                           float x0, float y0,
                                           float x1, float y1,
                                           sVec4f& avCol),
                    void (*beginText)(void* c),
                    void (*endText)(void* c),
                    void (*printText)(void* c,
                                      float x, float y, const char *str,
                                      const sColor4f& avCol),
                    float (*textWidth)(void* c, const char *str))
{
#ifdef Prof_ENABLED
  niProf_Begin(iprof_draw);
  niAssert(printText != NULL && textWidth != NULL);

  sVec4f c;

  int i,j,n,o;
  float backup_sy = sy;
  const float field_width = textWidth(context,"MMMMM.MM");
  const float name_width  = full_width - field_width * 3;
  const float plus_width  = textWidth(context,"+");

  int max_records;

  Prof_Report *pob;

  if (!int_to_string[0][0]) int_to_string_init();

  if (precision < 1) precision = 1;
  if (precision > 4) precision = 4;

  pob = Prof_create_report();
  // First iteration draw rectangles
  if (draw_rectangle) {
    if (begin_rects) begin_rects(context);
    sy = backup_sy;
    for (i=0; i < NUM_TITLE; ++i) {
      if (pob->title[i]) {
        float header_x0 = sx;
        float tw = textWidth(context,pob->title[i]) + 3.0f;
        float header_x1 = header_x0 + ni::Max(full_width,tw);
        if (i == 0) {
          c = Vec4(0.1f, 0.3f, 0.0f, 0.85f);
        }
        else {
          c = Vec4(0.2f, 0.1f, 0.1f, 0.85f);
        }
        draw_rectangle(context, header_x0, sy-2, header_x1, sy+line_spacing+2, c);
        sy += 1.5f*line_spacing;
        height -= ni::Abs(line_spacing)*1.5f;
      }
    }
    if (end_rects) end_rects(context);
  }

  // Second iteration draw text
  if (beginText) beginText(context);
  sy = backup_sy;
  for (i=0; i < NUM_TITLE; ++i) {
    if (pob->title[i]) {
      if (i == 0)
        c = Vec4(0.6f, 0.4f, 0.0f, 1.0f);
      else
        c = Vec4(0.8f, 0.1f, 0.1f, 1.0f);
      printText(context, sx+2, sy, pob->title[i], c);

      sy += 1.5f*line_spacing;
      height -= ni::Abs(line_spacing)*1.5f;
    }
  }
  if (endText) endText(context);

  max_records = (int)(height / ni::Abs(line_spacing));

  o = 0;
  n = pob->num_record;
  if (n > max_records) n = max_records;
  if (pob->hilight >= o + n) {
    o = pob->hilight - n + 1;
  }

  backup_sy = sy;

  // Draw the background colors for the zone data.
  if (draw_rectangle) {
    if (begin_rects) begin_rects(context);
    ni::sVec4f rectCol = Vec4(0.0f,0.0f,0.0f,0.85f);
    draw_rectangle(context, sx, sy, sx + full_width, sy + line_spacing, rectCol);
    sy += line_spacing;
    for (i = 0; i < n; i++) {
      float y0, y1;

      if (i & 1) {
        c = Vec4(0.1f, 0.1f, 0.2f, 0.85f);
      }
      else {
        c = Vec4(0.1f, 0.1f, 0.3f, 0.85f);
      }
      if (i+o == pob->hilight) {
        c = Vec4(0.3f, 0.3f, 0.1f, 0.85f);
      }

      y0 = sy;
      y1 = sy + line_spacing;

      draw_rectangle(context, sx, y0, sx + full_width, y1, c);
      sy += line_spacing;
    }
    if (end_rects) end_rects(context);
  }

  // Draw the text
  if (beginText) beginText(context);

  sy = backup_sy;
  c = Vec4(0.7f,0.7f,0.7f,1.0f);
  if (pob->header[0]) {
    printText(context, sx+8, sy, pob->header[0], c);
  }
  for (j=1; j < NUM_HEADER; ++j) {
    if (pob->header[j]) {
      printText(context,
                sx + name_width + field_width * (j-1) +
                field_width/2 - textWidth(context,pob->header[j])/2,
                sy, pob->header[j], c);
    }
  }

  sy += line_spacing;

  for (i = 0; i < n; i++) {
    char buf[256], *b = buf;
    Prof_Report_Record *r = &pob->record[i+o];
    float text_color[3], glow_color[3];
    float glow_alpha;
    float x = sx + textWidth(context," ") * r->indent + plus_width/2;
    if (r->prefix) {
      buf[0] = r->prefix;
      ++b;
    } else {
      x += plus_width;
    }
    if (r->number)
      sprintf(b, "%s (%d)", r->name, r->number);
    else
      sprintf(b, "%s", r->name);
    if (Prof_get_colors((float)r->heat, text_color, glow_color, &glow_alpha)) {
      c = Vec4(glow_color[0], glow_color[1], glow_color[2], glow_alpha);
      printText(context, x+2, sy-1, buf, c);
    }
    c = Vec4(text_color[0],text_color[1],text_color[2],1.0f);
    printText(context, x + 1, sy, buf, c);

    for (j=0; j < NUM_VALUES; ++j) {
      if (r->value_flag & (1 << j)) {
        int pad;
        Prof_float_to_string(buf, (float)r->values[j], j == 2 ? 2 : precision);
        pad = (int)(field_width - plus_width - textWidth(context,buf));
        if (r->indent) pad += (int)plus_width;
        printText(context,
                  sx + pad + name_width + field_width * j, sy, buf, c);
      }
    }


    sy += line_spacing;
  }
  if (endText) endText(context);

  Prof_free_report(pob);
  niProf_End();
#endif
}

#endif
