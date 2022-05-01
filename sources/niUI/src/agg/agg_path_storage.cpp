//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.3
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
// Class path_storage
//
//----------------------------------------------------------------------------
#include <string.h>
#include <math.h>
#include "agg_path_storage.h"
#include "agg_math.h"
#include "agg_bezier_arc.h"


namespace agg
{

//------------------------------------------------------------------------
path_storage::~path_storage()
{
  if(m_total_blocks)
  {
    agg_real** coord_blk = m_coord_blocks + m_total_blocks - 1;
    while(m_total_blocks--)
    {
      delete [] *coord_blk;
      --coord_blk;
    }
    delete [] m_coord_blocks;
  }
}


//------------------------------------------------------------------------
path_storage::path_storage() :
    m_total_vertices(0),
    m_total_blocks(0),
    m_max_blocks(0),
    m_coord_blocks(0),
    m_cmd_blocks(0),
    m_iterator(0),
    m_initial_x(0),
    m_initial_y(0),
    m_current_x(0),
    m_current_y(0)
{
}


//------------------------------------------------------------------------
path_storage::path_storage(const path_storage& ps) :
    m_total_vertices(0),
    m_total_blocks(0),
    m_max_blocks(0),
    m_coord_blocks(0),
    m_cmd_blocks(0),
    m_iterator(0),
    m_initial_x(0),
    m_initial_y(0),
    m_current_x(0),
    m_current_y(0)
{
  copy_from(ps);
}


//------------------------------------------------------------------------
void path_storage::remove_all()
{
  m_total_vertices = 0;
  m_iterator = 0;
}


//------------------------------------------------------------------------
void path_storage::copy_from(const path_storage& ps)
{
  remove_all();
  unsigned i;
  for(i = 0; i < ps.total_vertices(); i++)
  {
    agg_real x, y;
    unsigned cmd = ps.vertex(i, &x, &y);
    add_vertex(x, y, cmd);
  }
}


//------------------------------------------------------------------------
void path_storage::allocate_block(unsigned nb)
{
  if(nb >= m_max_blocks)
  {
    agg_real** new_coords =
        new agg_real* [(m_max_blocks + block_pool) * 2];

    unsigned char** new_cmds =
        (unsigned char**)(new_coords + m_max_blocks + block_pool);

    if(m_coord_blocks)
    {
      memcpy(new_coords,
             m_coord_blocks,
             m_max_blocks * sizeof(agg_real*));

      memcpy(new_cmds,
             m_cmd_blocks,
             m_max_blocks * sizeof(unsigned char*));

      delete [] m_coord_blocks;
    }
    m_coord_blocks = new_coords;
    m_cmd_blocks = new_cmds;
    m_max_blocks += block_pool;
  }
  m_coord_blocks[nb] =
      new agg_real [block_size * 2 +
                    block_size /
                    (sizeof(agg_real) / sizeof(unsigned char))];

  m_cmd_blocks[nb]  =
      (unsigned char*)(m_coord_blocks[nb] + block_size * 2);

  m_total_blocks++;
}


//------------------------------------------------------------------------
void path_storage::rewind(unsigned path_id)
{
  m_iterator = path_id;
}



//------------------------------------------------------------------------
void path_storage::arc_to(agg_real rx, agg_real ry,
                          agg_real angle,
                          bool large_arc_flag,
                          bool sweep_flag,
                          agg_real x, agg_real y)
{
  if(m_total_vertices && is_vertex(command(m_total_vertices - 1)))
  {
    const agg_real epsilon = agg_real(1e-30);
    agg_real x0 = 0.0;
    agg_real y0 = 0.0;
    last_vertex(&x0, &y0);

    rx = fabs(rx);
    ry = fabs(ry);

    // Ensure radii are valid
    //-------------------------
    if(rx < epsilon || ry < epsilon)
    {
      line_to(x, y);
      return;
    }

    if(calc_distance(x0, y0, x, y) < epsilon)
    {
      // If the endpoints (x, y) and (x0, y0) are identical, then this
      // is equivalent to omitting the elliptical arc segment entirely.
      return;
    }
    bezier_arc_svg a(x0, y0, rx, ry, angle, large_arc_flag, sweep_flag, x, y);
    if(a.radii_ok())
    {
      add_path(a, 0, true);
    }
    else
    {
      line_to(x, y);
    }
  }
  else
  {
    move_to(x, y);
  }
}


//------------------------------------------------------------------------
void path_storage::arc_rel(agg_real rx, agg_real ry,
                           agg_real angle,
                           bool large_arc_flag,
                           bool sweep_flag,
                           agg_real dx, agg_real dy)
{
  rel_to_abs(&dx, &dy);
  arc_to(rx, ry, angle, large_arc_flag, sweep_flag, dx, dy);
}


//------------------------------------------------------------------------
void path_storage::curve3(agg_real x_ctrl, agg_real y_ctrl,
                          agg_real x_to,   agg_real y_to)
{
  add_vertex(x_ctrl, y_ctrl, path_cmd_curve3);
  add_vertex(x_to,   y_to,   path_cmd_curve3);
}

//------------------------------------------------------------------------
void path_storage::curve3_rel(agg_real dx_ctrl, agg_real dy_ctrl,
                              agg_real dx_to,   agg_real dy_to)
{
  rel_to_abs(&dx_ctrl, &dy_ctrl);
  rel_to_abs(&dx_to,   &dy_to);
  add_vertex(dx_ctrl, dy_ctrl, path_cmd_curve3);
  add_vertex(dx_to,   dy_to,   path_cmd_curve3);
}

//------------------------------------------------------------------------
void path_storage::curve3(agg_real x_to, agg_real y_to)
{
  agg_real x0 = 0;
  agg_real y0 = 0;
  if(is_vertex(last_vertex(&x0, &y0)))
  {
    agg_real x_ctrl = 0;
    agg_real y_ctrl = 0;
    unsigned cmd = prev_vertex(&x_ctrl, &y_ctrl);
    if(is_curve(cmd))
    {
      x_ctrl = x0 + x0 - x_ctrl;
      y_ctrl = y0 + y0 - y_ctrl;
    }
    else
    {
      x_ctrl = x0;
      y_ctrl = y0;
    }
    curve3(x_ctrl, y_ctrl, x_to, y_to);
  }
}


//------------------------------------------------------------------------
void path_storage::curve3_rel(agg_real dx_to, agg_real dy_to)
{
  rel_to_abs(&dx_to, &dy_to);
  curve3(dx_to, dy_to);
}


//------------------------------------------------------------------------
void path_storage::curve4(agg_real x_ctrl1, agg_real y_ctrl1,
                          agg_real x_ctrl2, agg_real y_ctrl2,
                          agg_real x_to,    agg_real y_to)
{
  add_vertex(x_ctrl1, y_ctrl1, path_cmd_curve4);
  add_vertex(x_ctrl2, y_ctrl2, path_cmd_curve4);
  add_vertex(x_to,    y_to,    path_cmd_curve4);
}

//------------------------------------------------------------------------
void path_storage::curve4_rel(agg_real dx_ctrl1, agg_real dy_ctrl1,
                              agg_real dx_ctrl2, agg_real dy_ctrl2,
                              agg_real dx_to,    agg_real dy_to)
{
  rel_to_abs(&dx_ctrl1, &dy_ctrl1);
  rel_to_abs(&dx_ctrl2, &dy_ctrl2);
  rel_to_abs(&dx_to,    &dy_to);
  add_vertex(dx_ctrl1, dy_ctrl1, path_cmd_curve4);
  add_vertex(dx_ctrl2, dy_ctrl2, path_cmd_curve4);
  add_vertex(dx_to,    dy_to,    path_cmd_curve4);
}


//------------------------------------------------------------------------
void path_storage::curve4(agg_real x_ctrl2, agg_real y_ctrl2,
                          agg_real x_to,    agg_real y_to)
{
  agg_real x0 = 0;
  agg_real y0 = 0;
  if(is_vertex(last_vertex(&x0, &y0)))
  {
    agg_real x_ctrl1 = 0;
    agg_real y_ctrl1 = 0;
    unsigned cmd = prev_vertex(&x_ctrl1, &y_ctrl1);
    if(is_curve(cmd))
    {
      x_ctrl1 = x0 + x0 - x_ctrl1;
      y_ctrl1 = y0 + y0 - y_ctrl1;
    }
    else
    {
      x_ctrl1 = x0;
      y_ctrl1 = y0;
    }
    curve4(x_ctrl1, y_ctrl1, x_ctrl2, y_ctrl2, x_to, y_to);
  }
}


//------------------------------------------------------------------------
void path_storage::curve4_rel(agg_real dx_ctrl2, agg_real dy_ctrl2,
                              agg_real dx_to,    agg_real dy_to)
{
  rel_to_abs(&dx_ctrl2, &dy_ctrl2);
  rel_to_abs(&dx_to,    &dy_to);
  curve4(dx_ctrl2, dy_ctrl2, dx_to, dy_to);
}


//------------------------------------------------------------------------
void path_storage::end_poly(unsigned flags)
{
  if(m_total_vertices)
  {
    if(is_vertex(command(m_total_vertices - 1)))
    {
      add_vertex(0.0, 0.0, path_cmd_end_poly | flags);
    }
  }
}


//------------------------------------------------------------------------
unsigned path_storage::start_new_path()
{
  if(m_total_vertices)
  {
    if(!is_stop(command(m_total_vertices - 1)))
    {
      add_vertex(0.0, 0.0, path_cmd_stop);
    }
  }
  return m_total_vertices;
}


//------------------------------------------------------------------------
void path_storage::add_poly(const agg_real* vertices, unsigned num,
                            bool solid_path, unsigned end_flags)
{
  if(num)
  {
    if(!solid_path)
    {
      move_to(vertices[0], vertices[1]);
      vertices += 2;
      --num;
    }
    while(num--)
    {
      line_to(vertices[0], vertices[1]);
      vertices += 2;
    }
    if(end_flags) end_poly(end_flags);
  }
}


//------------------------------------------------------------------------
unsigned path_storage::perceive_polygon_orientation(unsigned start, unsigned end)
{
  // Calculate signed area (agg_real area to be exact)
  //---------------------
  unsigned np = end - start;
  agg_real area = 0.0;
  unsigned i;
  for(i = 0; i < np; i++)
  {
    agg_real x1, y1, x2, y2;
    vertex(start + i,            &x1, &y1);
    vertex(start + (i + 1) % np, &x2, &y2);
    area += x1 * y2 - y1 * x2;
  }
  return (area < 0.0) ? path_flags_cw : path_flags_ccw;
}


//------------------------------------------------------------------------
void path_storage::invert_polygon(unsigned start, unsigned end)
{
  unsigned i;
  unsigned tmp_cmd = command(start);

  --end; // Make "end" inclusive

  // Shift all commands to one position
  for(i = start; i < end; i++)
  {
    modify_command(i, command(i + 1));
  }

  // Assign starting command to the ending command
  modify_command(end, tmp_cmd);

  // Reverse the polygon
  while(end > start)
  {
    unsigned start_nb = start >> block_shift;
    unsigned end_nb   = end   >> block_shift;
    agg_real* start_ptr = m_coord_blocks[start_nb] + ((start & block_mask) << 1);
    agg_real* end_ptr   = m_coord_blocks[end_nb]   + ((end   & block_mask) << 1);
    agg_real tmp_xy;

    tmp_xy       = *start_ptr;
    *start_ptr++ = *end_ptr;
    *end_ptr++   = tmp_xy;

    tmp_xy       = *start_ptr;
    *start_ptr   = *end_ptr;
    *end_ptr     = tmp_xy;

    tmp_cmd = m_cmd_blocks[start_nb][start & block_mask];
    m_cmd_blocks[start_nb][start & block_mask] = m_cmd_blocks[end_nb][end & block_mask];
    m_cmd_blocks[end_nb][end & block_mask] = (unsigned char)tmp_cmd;

    ++start;
    --end;
  }
}


//------------------------------------------------------------------------
unsigned path_storage::arrange_polygon_orientation(unsigned start,
                                                   path_flags_e orientation)
{
  if(orientation == path_flags_none) return start;

  // Skip all non-vertices at the beginning
  while(start < m_total_vertices && !is_vertex(command(start))) ++start;

  // Skip all insignificant move_to
  while(start+1 < m_total_vertices &&
        is_move_to(command(start)) &&
        is_move_to(command(start+1))) ++start;

  // Find the last vertex
  unsigned end = start + 1;
  while(end < m_total_vertices && !is_next_poly(command(end))) ++end;

  if(end - start > 2)
  {
    if(perceive_polygon_orientation(start, end) != unsigned(orientation))
    {
      // Invert polygon, set orientation flag, and skip all end_poly
      invert_polygon(start, end);
      unsigned cmd;
      while(end < m_total_vertices && is_end_poly(cmd = command(end)))
      {
        modify_command(end++, set_orientation(cmd, orientation));
      }
    }
  }
  return end;
}


//------------------------------------------------------------------------
unsigned path_storage::arrange_orientations(unsigned start,
                                            path_flags_e orientation)
{
  if(orientation != path_flags_none)
  {
    while(start < m_total_vertices)
    {
      start = arrange_polygon_orientation(start, orientation);
      if(is_stop(command(start)))
      {
        ++start;
        break;
      }
    }
  }
  return start;
}


//------------------------------------------------------------------------
void path_storage::arrange_orientations_all_paths(path_flags_e orientation)
{
  if(orientation != path_flags_none)
  {
    unsigned start = 0;
    while(start < m_total_vertices)
    {
      start = arrange_orientations(start, orientation);
    }
  }
}


//------------------------------------------------------------------------
void path_storage::flip_x(agg_real x1, agg_real x2)
{
  unsigned i;
  agg_real x, y;
  for(i = 0; i < m_total_vertices; i++)
  {
    unsigned cmd = vertex(i, &x, &y);
    if(is_vertex(cmd))
    {
      modify_vertex(i, x2 - x + x1, y);
    }
  }
}


//------------------------------------------------------------------------
void path_storage::flip_y(agg_real y1, agg_real y2)
{
  unsigned i;
  agg_real x, y;
  for(i = 0; i < m_total_vertices; i++)
  {
    unsigned cmd = vertex(i, &x, &y);
    if(is_vertex(cmd))
    {
      modify_vertex(i, x, y2 - y + y1);
    }
  }
}


}
