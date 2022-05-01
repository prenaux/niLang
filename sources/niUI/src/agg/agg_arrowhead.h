//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.4
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software
// is granted provided this copyright notice appears in all copies.
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------
//
// Simple arrowhead/arrowtail generator
//
//----------------------------------------------------------------------------
#ifndef AGG_ARROWHEAD_INCLUDED
#define AGG_ARROWHEAD_INCLUDED

#include "agg_basics.h"

namespace agg
{

//===============================================================arrowhead
//
// See implementation agg_arrowhead.cpp
//
class arrowhead
{
 public:
  arrowhead();

  void head(agg_real d1, agg_real d2, agg_real d3, agg_real d4)
  {
    m_head_d1 = d1;
    m_head_d2 = d2;
    m_head_d3 = d3;
    m_head_d4 = d4;
    m_head_flag = true;
  }

  void head()    { m_head_flag = true; }
  void no_head() { m_head_flag = false; }

  void tail(agg_real d1, agg_real d2, agg_real d3, agg_real d4)
  {
    m_tail_d1 = d1;
    m_tail_d2 = d2;
    m_tail_d3 = d3;
    m_tail_d4 = d4;
    m_tail_flag = true;
  }

  void tail()    { m_tail_flag = true;  }
  void no_tail() { m_tail_flag = false; }

  void rewind(unsigned path_id);
  unsigned vertex(agg_real* x, agg_real* y);

 private:
  agg_real   m_head_d1;
  agg_real   m_head_d2;
  agg_real   m_head_d3;
  agg_real   m_head_d4;
  agg_real   m_tail_d1;
  agg_real   m_tail_d2;
  agg_real   m_tail_d3;
  agg_real   m_tail_d4;
  bool     m_head_flag;
  bool     m_tail_flag;
  agg_real   m_coord[16];
  unsigned m_cmd[8];
  unsigned m_curr_id;
  unsigned m_curr_coord;
};

}

#endif
