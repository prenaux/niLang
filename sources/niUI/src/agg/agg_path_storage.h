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

#ifndef AGG_PATH_STORAGE_INCLUDED
#define AGG_PATH_STORAGE_INCLUDED

#include "agg_basics.h"
#include "agg_math.h"

namespace agg
{

//------------------------------------------------------------path_storage
// A container to store vertices with their flags.
// A path consists of a number of contours separated with "move_to"
// commands. The path storage can keep and maintain more than one
// path.
// To navigate to the beginning of a particular path, use rewind(path_id);
// Where path_id is what start_new_path() returns. So, when you call
// start_new_path() you need to store its return value somewhere else
// to navigate to the path afterwards.
//
// See Implementation: agg_path_storage.cpp
// See also: vertex_source concept
//------------------------------------------------------------------------
class path_storage
{
  // Allocation parameters
  enum block_scale_e
  {
    block_shift = 8,
    block_size  = 1 << block_shift,
    block_mask  = block_size - 1,
    block_pool  = 256
  };

 public:
  typedef agg_real value_type;

  //--------------------------------------------------------------------
  class vertex_source
  {
   public:
    vertex_source() {}
    vertex_source(const path_storage& p) : m_path(&p), m_vertex_idx(0) {}

    void rewind(unsigned path_id)
    {
      m_vertex_idx = path_id;
    }

    unsigned vertex(agg_real* x, agg_real* y)
    {
      return (m_vertex_idx < m_path->total_vertices())?
          m_path->vertex(m_vertex_idx++, x, y):
          path_cmd_stop;
    }

   private:
    const path_storage* m_path;
    unsigned            m_vertex_idx;
  };


  ~path_storage();
  path_storage();
  path_storage(const path_storage& ps);

  void remove_all();

  unsigned last_vertex(agg_real* x, agg_real* y) const;
  unsigned prev_vertex(agg_real* x, agg_real* y) const;
  void current_vertex(agg_real* x, agg_real* y) const;
  void initial_vertex(agg_real* x, agg_real* y) const;

  agg_real last_x() const;
  agg_real last_y() const;

  void set_current_to_initial();
  void rel_to_abs(agg_real* x, agg_real* y) const;

  void move_to(agg_real x, agg_real y);
  void move_rel(agg_real dx, agg_real dy);

  void line_to(agg_real x, agg_real y);
  void line_rel(agg_real dx, agg_real dy);

  void hline_to(agg_real x);
  void hline_rel(agg_real dx);

  void vline_to(agg_real y);
  void vline_rel(agg_real dy);

  void arc_to(agg_real rx, agg_real ry,
              agg_real angle,
              bool large_arc_flag,
              bool sweep_flag,
              agg_real x, agg_real y);

  void arc_rel(agg_real rx, agg_real ry,
               agg_real angle,
               bool large_arc_flag,
               bool sweep_flag,
               agg_real dx, agg_real dy);

  void curve3(agg_real x_ctrl, agg_real y_ctrl,
              agg_real x_to,   agg_real y_to);

  void curve3_rel(agg_real dx_ctrl, agg_real dy_ctrl,
                  agg_real dx_to,   agg_real dy_to);

  void curve3(agg_real x_to, agg_real y_to);

  void curve3_rel(agg_real dx_to, agg_real dy_to);

  void curve4(agg_real x_ctrl1, agg_real y_ctrl1,
              agg_real x_ctrl2, agg_real y_ctrl2,
              agg_real x_to,    agg_real y_to);

  void curve4_rel(agg_real dx_ctrl1, agg_real dy_ctrl1,
                  agg_real dx_ctrl2, agg_real dy_ctrl2,
                  agg_real dx_to,    agg_real dy_to);

  void curve4(agg_real x_ctrl2, agg_real y_ctrl2,
              agg_real x_to,    agg_real y_to);

  void curve4_rel(agg_real x_ctrl2, agg_real y_ctrl2,
                  agg_real x_to,    agg_real y_to);


  void end_poly(unsigned flags = path_flags_close);

  void close_polygon(unsigned flags = path_flags_none)
  {
    end_poly(path_flags_close | flags);
  }

  void add_poly(const agg_real* vertices, unsigned num,
                bool solid_path = false,
                unsigned end_flags = path_flags_none);


  template<class VertexSource>
  void add_path(VertexSource& vs,
                unsigned path_id = 0,
                bool solid_path = true)
  {
    agg_real x, y;
    unsigned cmd;
    vs.rewind(path_id);
    while(!is_stop(cmd = vs.vertex(&x, &y)))
    {
      if(is_move_to(cmd) && solid_path && m_total_vertices)
      {
        cmd = path_cmd_line_to;
      }
      add_vertex(x, y, cmd);
    }
  }


  // Concatenate path. The path is added as is.
  //--------------------------------------------------------------------
  template<class VertexSource>
  void concat_path(VertexSource& vs, unsigned path_id = 0)
  {
    agg_real x, y;
    unsigned cmd;
    vs.rewind(path_id);
    while(!is_stop(cmd = vs.vertex(&x, &y)))
    {
      add_vertex(x, y, cmd);
    }
  }

  //--------------------------------------------------------------------
  // Join path. The path is joined with the existing one, that is,
  // it behaves as if the pen of a plotter was always down (drawing)
  template<class VertexSource>
  void join_path(VertexSource& vs, unsigned path_id = 0)
  {
    agg_real x, y;
    unsigned cmd;
    vs.rewind(path_id);
    cmd = vs.vertex(&x, &y);
    if(!is_stop(cmd))
    {
      if(is_vertex(cmd))
      {
        agg_real x0, y0;
        unsigned cmd0 = last_vertex(&x0, &y0);
        if (is_vertex(cmd0)) {
          if (calc_distance(x, y, x0, y0) > vertex_dist_epsilon) {
            if(is_move_to(cmd)) cmd = path_cmd_line_to;
            add_vertex(x, y, cmd);
          }
        }
        else
        {
          if(is_stop(cmd0))
          {
            cmd = path_cmd_move_to;
          }
          else
          {
            if(is_move_to(cmd)) cmd = path_cmd_line_to;
          }
          add_vertex(x, y, cmd);
        }
      }
      while(!is_stop(cmd = vs.vertex(&x, &y)))
      {
        add_vertex(x, y, is_move_to(cmd) ?
                   path_cmd_line_to :
                   cmd);
      }
    }
  }

  unsigned start_new_path();

  void copy_from(const path_storage& ps);
  const path_storage& operator = (const path_storage& ps)
  {
    copy_from(ps);
    return *this;
  }


  unsigned total_vertices() const { return m_total_vertices; }
  unsigned vertex(unsigned idx, agg_real* x, agg_real* y) const
  {
    unsigned nb = idx >> block_shift;
    const agg_real* pv = m_coord_blocks[nb] + ((idx & block_mask) << 1);
    *x = *pv++;
    *y = *pv;
    return m_cmd_blocks[nb][idx & block_mask];
  }
  void set_vertex(unsigned idx, agg_real x, agg_real y) const
  {
    unsigned nb = idx >> block_shift;
    agg_real* pv = m_coord_blocks[nb] + ((idx & block_mask) << 1);
    *pv++ = x;
    *pv = y;
  }
  unsigned command(unsigned idx) const
  {
    return m_cmd_blocks[idx >> block_shift][idx & block_mask];
  }
  void set_command(unsigned idx, unsigned cmd) {
    m_cmd_blocks[idx >> block_shift][idx & block_mask] = (unsigned char)cmd;
  }

  void     rewind(unsigned path_id);
  unsigned vertex(agg_real* x, agg_real* y);

  // Arrange the orientation of a polygon, all polygons in a path,
  // or in all paths. After calling arrange_orientations() or
  // arrange_orientations_all_paths(), all the polygons will have
  // the same orientation, i.e. path_flags_cw or path_flags_ccw
  unsigned arrange_polygon_orientation(unsigned start, path_flags_e orientation);
  unsigned arrange_orientations(unsigned path_id, path_flags_e orientation);
  void     arrange_orientations_all_paths(path_flags_e orientation);

  // Flip all the vertices horizontally or vertically
  void flip_x(agg_real x1, agg_real x2);
  void flip_y(agg_real y1, agg_real y2);

  // This function adds a vertex with its flags directly. Since there's no
  // checking for errors, keeping proper path integrity is the responsibility
  // of the caller. It can be said the function is "not very public".
  void add_vertex(agg_real x, agg_real y, unsigned cmd);

  // Allows you to modify vertex coordinates. The caller must know
  // the index of the vertex.
  void modify_vertex(unsigned idx, agg_real x, agg_real y)
  {
    agg_real* pv = m_coord_blocks[idx >> block_shift] + ((idx & block_mask) << 1);
    *pv++ = x;
    *pv   = y;
  }

  // Allows you to modify vertex command. The caller must know
  // the index of the vertex.
  void modify_command(unsigned idx, unsigned cmd)
  {
    m_cmd_blocks[idx >> block_shift][idx & block_mask] = (unsigned char)cmd;
  }

  unsigned get_total_vertices() const {
    return m_total_vertices;
  }

  unsigned get_total_blocks() const {
    return m_total_blocks;
  }

 private:
  void allocate_block(unsigned nb);
  unsigned char* storage_ptrs(agg_real** xy_ptr);

  unsigned perceive_polygon_orientation(unsigned start, unsigned end);
  void     invert_polygon(unsigned start, unsigned end);

 private:
  unsigned        m_total_vertices;
  unsigned        m_total_blocks;
  unsigned        m_max_blocks;
  agg_real**        m_coord_blocks;
  unsigned char** m_cmd_blocks;
  unsigned        m_iterator;
  agg_real    m_current_x;
  agg_real    m_current_y;
  agg_real    m_initial_x;
  agg_real    m_initial_y;
};


//------------------------------------------------------------------------
inline unsigned path_storage::vertex(agg_real* x, agg_real* y)
{
  if(m_iterator >= m_total_vertices) return path_cmd_stop;
  return vertex(m_iterator++, x, y);
}

//------------------------------------------------------------------------
inline unsigned path_storage::prev_vertex(agg_real* x, agg_real* y) const
{
  if(m_total_vertices > 1)
  {
    return vertex(m_total_vertices - 2, x, y);
  }
  return path_cmd_stop;
}

//------------------------------------------------------------------------
inline void path_storage::current_vertex(agg_real* x, agg_real* y) const
{
  *x = m_current_x;
  *y = m_current_y;
}

//------------------------------------------------------------------------
inline void path_storage::initial_vertex(agg_real* x, agg_real* y) const
{
  *x = m_initial_x;
  *y = m_initial_y;
}

//------------------------------------------------------------------------
inline unsigned path_storage::last_vertex(agg_real* x, agg_real* y) const
{
  if(m_total_vertices)
  {
    return vertex(m_total_vertices - 1, x, y);
  }
  return path_cmd_stop;
}

//------------------------------------------------------------------------
inline agg_real path_storage::last_x() const
{
  if(m_total_vertices)
  {
    unsigned idx = m_total_vertices - 1;
    return m_coord_blocks[idx >> block_shift][(idx & block_mask) << 1];
  }
  return 0.0;
}

//------------------------------------------------------------------------
inline agg_real path_storage::last_y() const
{
  if(m_total_vertices)
  {
    unsigned idx = m_total_vertices - 1;
    return m_coord_blocks[idx >> block_shift][((idx & block_mask) << 1) + 1];
  }
  return 0.0;
}

//------------------------------------------------------------------------
inline void path_storage::set_current_to_initial()
{
  m_current_x = m_initial_x;
  m_current_y = m_initial_y;
}

//------------------------------------------------------------------------
inline void path_storage::rel_to_abs(agg_real* x, agg_real* y) const
{
  if(m_total_vertices)
  {
    //agg_real x2;
    //agg_real y2;
    //if(is_vertex(vertex(m_total_vertices - 1, &x2, &y2)))
    //{
    *x += m_current_x;
    *y += m_current_y;
    //}
  }
}

//------------------------------------------------------------------------
inline unsigned char* path_storage::storage_ptrs(agg_real** xy_ptr)
{
  unsigned nb = m_total_vertices >> block_shift;
  if(nb >= m_total_blocks)
  {
    allocate_block(nb);
  }
  *xy_ptr = m_coord_blocks[nb] + ((m_total_vertices & block_mask) << 1);
  return m_cmd_blocks[nb] + (m_total_vertices & block_mask);
}


//------------------------------------------------------------------------
inline void path_storage::add_vertex(agg_real x, agg_real y, unsigned cmd)
{
  agg_real* coord_ptr = 0;
  unsigned char* cmd_ptr = storage_ptrs(&coord_ptr);
  *cmd_ptr = (unsigned char)cmd;
  *coord_ptr++ = x;
  *coord_ptr   = y;
  m_total_vertices++;
  m_current_x = x;
  m_current_y = y;
  if (is_move_to(cmd)) {
    m_initial_x = x;
    m_initial_y = y;
  }
}

//------------------------------------------------------------------------
inline void path_storage::move_to(agg_real x, agg_real y)
{
  add_vertex(x, y, path_cmd_move_to);
}

//------------------------------------------------------------------------
inline void path_storage::move_rel(agg_real dx, agg_real dy)
{
  rel_to_abs(&dx, &dy);
  add_vertex(dx, dy, path_cmd_move_to);
}

//------------------------------------------------------------------------
inline void path_storage::line_to(agg_real x, agg_real y)
{
  add_vertex(x, y, path_cmd_line_to);
}

//------------------------------------------------------------------------
inline void path_storage::line_rel(agg_real dx, agg_real dy)
{
  rel_to_abs(&dx, &dy);
  add_vertex(dx, dy, path_cmd_line_to);
}

//------------------------------------------------------------------------
inline void path_storage::hline_to(agg_real x)
{
  add_vertex(x, last_y(), path_cmd_line_to);
}

//------------------------------------------------------------------------
inline void path_storage::hline_rel(agg_real dx)
{
  agg_real dy = 0;
  rel_to_abs(&dx, &dy);
  add_vertex(dx, dy, path_cmd_line_to);
}

//------------------------------------------------------------------------
inline void path_storage::vline_to(agg_real y)
{
  add_vertex(last_x(), y, path_cmd_line_to);
}

//------------------------------------------------------------------------
inline void path_storage::vline_rel(agg_real dy)
{
  agg_real dx = 0;
  rel_to_abs(&dx, &dy);
  add_vertex(dx, dy, path_cmd_line_to);
}

}



#endif
