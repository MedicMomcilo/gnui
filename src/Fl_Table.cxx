//
// "$Id$"
//
// fltk3::Table -- A table widget
//
// Copyright 2002 by Greg Ercolano.
// Copyright (c) 2004 O'ksi'D
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
// 
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//

#include <stdio.h>		// fprintf
#include <fltk3/draw.h>
#include <fltk3/Table.h>

#if defined(USE_UTF8) && ( defined(MICROSOFT) || defined(LINUX) )
#include <fltk3/fl_utf8.h>	// currently only Windows and Linux
#endif

#define SCROLLBAR_SIZE	16

// Scroll display so 'row' is at top
void fltk3::Table::row_position(int row) {
  if ( _row_position == row ) return;		// OPTIMIZATION: no change? avoid redraw
  if ( row < 0 ) row = 0;
  else if ( row >= rows() ) row = rows() - 1;
  if ( table_h <= tih ) return;			// don't scroll if table smaller than window
  double newtop = row_scroll_position(row);
  if ( newtop > vscrollbar->maximum() ) {
    newtop = vscrollbar->maximum();
  }
  vscrollbar->fltk3::Slider::value(newtop);
  table_scrolled();
  redraw();
  _row_position = row;	// HACK: override what table_scrolled() came up with
}

// Scroll display so 'col' is at left
void fltk3::Table::col_position(int col) {
  if ( _col_position == col ) return;	// OPTIMIZATION: no change? avoid redraw
  if ( col < 0 ) col = 0;
  else if ( col >= cols() ) col = cols() - 1;
  if ( table_w <= tiw ) return;		// don't scroll if table smaller than window
  double newleft = col_scroll_position(col);
  if ( newleft > hscrollbar->maximum() ) {
    newleft = hscrollbar->maximum();
  }
  hscrollbar->fltk3::Slider::value(newleft);
  table_scrolled();
  redraw();
  _col_position = col;	// HACK: override what table_scrolled() came up with
}

// Find scroll position of a row (in pixels)
long fltk3::Table::row_scroll_position(int row) {
  int startrow = 0;
  long scroll = 0; 
  // OPTIMIZATION: 
  //     Attempt to use precomputed row scroll position
  //
  if ( toprow_scrollpos != -1 && row >= toprow ) {
    scroll = toprow_scrollpos;
    startrow = toprow;
  }
  for ( int t=startrow; t<row; t++ ) {
    scroll += row_height(t);
  }
  return(scroll);
}

// Find scroll position of a column (in pixels)
long fltk3::Table::col_scroll_position(int col) {
  int startcol = 0;
  long scroll = 0;
  // OPTIMIZATION: 
  //     Attempt to use precomputed row scroll position
  //
  if ( leftcol_scrollpos != -1 && col >= leftcol ) {
    scroll = leftcol_scrollpos;
    startcol = leftcol;
  }
  for ( int t=startcol; t<col; t++ ) {
    scroll += col_width(t);
  }
  return(scroll);
}

// Ctor
fltk3::Table::Table(int X, int Y, int W, int H, const char *l) : fltk3::Group(X,Y,W,H,l) {
  _rows             = 0;
  _cols             = 0;
  _row_header_w     = 40;
  _col_header_h     = 18;
  _row_header       = 0;
  _col_header       = 0;
  _row_header_color = color();
  _col_header_color = color();
  _row_resize       = 0;
  _col_resize       = 0;
  _row_resize_min   = 1;
  _col_resize_min   = 1;
  _redraw_toprow    = -1;
  _redraw_botrow    = -1;
  _redraw_leftcol   = -1;
  _redraw_rightcol  = -1;
  table_w           = 0;
  table_h           = 0;
  toprow            = 0;
  botrow            = 0;
  leftcol           = 0;
  rightcol          = 0;
  toprow_scrollpos  = -1;
  leftcol_scrollpos = -1;
  _last_cursor      = fltk3::CURSOR_DEFAULT;
  _resizing_col     = -1;
  _resizing_row     = -1;
  _dragging_x       = -1;
  _dragging_y       = -1;
  _last_row         = -1;
  _auto_drag        = 0;
  current_col	      = -1;
  current_row       = -1;
  select_row        = -1;
  select_col        = -1;
  
  box(fltk3::THIN_DOWN_FRAME);
  
  vscrollbar = new fltk3::Scrollbar(x()+w()-SCROLLBAR_SIZE, y(),
                                SCROLLBAR_SIZE, h()-SCROLLBAR_SIZE);
  vscrollbar->type(fltk3::VERTICAL);
  vscrollbar->callback(scroll_cb, (void*)this);
  
  hscrollbar = new fltk3::Scrollbar(x(), y()+h()-SCROLLBAR_SIZE,
                                w(), SCROLLBAR_SIZE);
  hscrollbar->type(fltk3::HORIZONTAL);
  hscrollbar->callback(scroll_cb, (void*)this);
  
  table = new fltk3::ScrollGroup(x(), y(), w(), h());
  table->box(fltk3::NO_BOX);
  table->type(0);		// don't show fltk3::ScrollGroup's scrollbars -- use our own
  table->hide();		// hide unless children are present
  table->end();
  
  table_resized();
  redraw();
  
  fltk3::Group::end();		// end the group's begin()
  
  table->begin();		// leave with fltk children getting added to the scroll
}

// Dtor
fltk3::Table::~Table() {
  // The parent fltk3::Group takes care of destroying scrollbars
}

// Set height of a row
void fltk3::Table::row_height(int row, int height) {
  if ( row < 0 ) return;
  if ( row < (int)_rowheights.size() && _rowheights[row] == height ) {
    return;		// OPTIMIZATION: no change? avoid redraw
  }
  // Add row heights, even if none yet
  int now_size = (int)_rowheights.size();
  if ( row >= now_size ) {
    _rowheights.size(row);
    while (now_size < row)
      _rowheights[now_size++] = height;
  }
  _rowheights[row] = height;
  table_resized();
  if ( row <= botrow ) {	// OPTIMIZATION: only redraw if onscreen or above screen
    redraw();
  }
  // ROW RESIZE CALLBACK
  if ( fltk3::Widget::callback() && when() & fltk3::WHEN_CHANGED ) {
    do_callback(CONTEXT_RC_RESIZE, row, 0);
  }
}

// Set width of a column
void fltk3::Table::col_width(int col, int width)
{
  if ( col < 0 ) return;
  if ( col < (int)_colwidths.size() && _colwidths[col] == width ) {
    return;			// OPTIMIZATION: no change? avoid redraw
  }
  // Add column widths, even if none yet
  int now_size = (int)_colwidths.size();
  if ( col >= now_size ) {
    _colwidths.size(col);
    while (now_size < col) {
      _colwidths[now_size++] = width;
    }
  }
  _colwidths[col] = width;
  table_resized();
  if ( col <= rightcol ) {	// OPTIMIZATION: only redraw if onscreen or to the left
    redraw();
  }
  // COLUMN RESIZE CALLBACK
  if ( fltk3::Widget::callback() && when() & fltk3::WHEN_CHANGED ) {
    do_callback(CONTEXT_RC_RESIZE, 0, col);
  }
}

// Return row/col clamped to reality
int fltk3::Table::row_col_clamp(TableContext context, int &R, int &C) {
  int clamped = 0;
  if ( R < 0 ) { R = 0; clamped = 1; }
  if ( C < 0 ) { C = 0; clamped = 1; }
  switch ( context ) {
    case CONTEXT_COL_HEADER:
      // Allow col headers to draw even if no rows
      if ( R >= _rows && R != 0 ) { R = _rows - 1; clamped = 1; }
      break;
      
    case CONTEXT_ROW_HEADER:
      // Allow row headers to draw even if no columns
      if ( C >= _cols && C != 0 ) { C = _cols - 1; clamped = 1; }
      break;
      
    case CONTEXT_CELL:
    default:
      // CLAMP R/C TO _rows/_cols
      if ( R >= _rows ) { R = _rows - 1; clamped = 1; }
      if ( C >= _cols ) { C = _cols - 1; clamped = 1; }
      break;
  }
  return(clamped);
}

// Return bounding region for given context
void fltk3::Table::get_bounds(TableContext context, int &X, int &Y, int &W, int &H) {
  switch ( context ) {
    case CONTEXT_COL_HEADER:
      // Column header clipping.
      X = tox;
      Y = wiy;
      W = tow;
      H = col_header_height();
      return;
      
    case CONTEXT_ROW_HEADER:
      // Row header clipping.
      X = wix;
      Y = toy;
      W = row_header_width();
      H = toh;
      return;
      
    case CONTEXT_TABLE:
      // Table inner dimensions
      X = tix; Y = tiy; W = tiw; H = tih;
      return;
      
      // TODO: Add other contexts..
    default:
      fprintf(stderr, "fltk3::Table::get_bounds(): context %d unimplemented\n", (int)context);
      return;
  }
  //NOTREACHED
}

// Find row/col beneath cursor
//
//    Returns R/C and context.
//    Also returns resizeflag, if mouse is hovered over a resize boundary.
//
fltk3::Table::TableContext fltk3::Table::cursor2rowcol(int &R, int &C, ResizeFlag &resizeflag) {
  // return values
  R = C = 0;
  resizeflag = RESIZE_NONE;
  // Row header?
  int X, Y, W, H;
  if ( row_header() ) {
    // Inside a row heading?
    get_bounds(CONTEXT_ROW_HEADER, X, Y, W, H);
    if ( fltk3::event_inside(X, Y, W, H) ) {
      // Scan visible rows until found
      for ( R = toprow; R <= botrow; R++ ) {
        find_cell(CONTEXT_ROW_HEADER, R, 0, X, Y, W, H);
        if ( fltk3::event_y() >= Y && fltk3::event_y() < (Y+H) ) {
          // Found row?
          //     If cursor over resize boundary, and resize enabled,
          //     enable the appropriate resize flag.
          //
          if ( row_resize() ) {
            if ( fltk3::event_y() <= (Y+3-0) ) { resizeflag = RESIZE_ROW_ABOVE; }
            if ( fltk3::event_y() >= (Y+H-3) ) { resizeflag = RESIZE_ROW_BELOW; }
          }
          return(CONTEXT_ROW_HEADER);
        }
      }
      // Must be in row header dead zone
      return(CONTEXT_NONE);
    }
  }
  // Column header?
  if ( col_header() ) {
    // Inside a column heading?
    get_bounds(CONTEXT_COL_HEADER, X, Y, W, H);
    if ( fltk3::event_inside(X, Y, W, H) ) {
      // Scan visible columns until found
      for ( C = leftcol; C <= rightcol; C++ ) {
        find_cell(CONTEXT_COL_HEADER, 0, C, X, Y, W, H);
        if ( fltk3::event_x() >= X && fltk3::event_x() < (X+W) ) {
          // Found column?
          //     If cursor over resize boundary, and resize enabled,
          //     enable the appropriate resize flag.
          //
          if ( col_resize() ) {
            if ( fltk3::event_x() <= (X+3-0) ) { resizeflag = RESIZE_COL_LEFT; }
            if ( fltk3::event_x() >= (X+W-3) ) { resizeflag = RESIZE_COL_RIGHT; }
          }
          return(CONTEXT_COL_HEADER);
        }
      }
      // Must be in column header dead zone
      return(CONTEXT_NONE);
    }
  }
  // Mouse somewhere in table?
  //     Scan visible r/c's until we find it.
  //
  if ( fltk3::event_inside(tox, toy, tow, toh) ) {
    for ( R = toprow; R <= botrow; R++ ) {
      find_cell(CONTEXT_CELL, R, C, X, Y, W, H);
      if ( fltk3::event_y() < Y ) break;		// OPT: thanks lars
      if ( fltk3::event_y() >= (Y+H) ) continue;	// OPT: " "
      for ( C = leftcol; C <= rightcol; C++ ) {
        find_cell(CONTEXT_CELL, R, C, X, Y, W, H);
        if ( fltk3::event_inside(X, Y, W, H) ) {
          return(CONTEXT_CELL);			// found it
        }
      }
    }
    // Must be in a dead zone of the table
    R = C = 0;
    return(CONTEXT_TABLE);
  }
  // Somewhere else
  return(CONTEXT_NONE);
}

// Find X/Y/W/H for cell at R/C
//     If R or C are out of range, returns -1 
//     with X/Y/W/H set to zero.
//
int fltk3::Table::find_cell(TableContext context, int R, int C, int &X, int &Y, int &W, int &H) {
  if ( row_col_clamp(context, R, C) ) {		// row or col out of range? error
    X=Y=W=H=0;
    return(-1);
  }
  X = col_scroll_position(C) - hscrollbar->value() + tix;
  Y = row_scroll_position(R) - vscrollbar->value() + tiy;
  W = col_width(C);
  H = row_height(R);
  
  switch ( context ) {
    case CONTEXT_COL_HEADER:
      Y = wiy;
      H = col_header_height();
      return(0);
      
    case CONTEXT_ROW_HEADER:
      X = wix;
      W = row_header_width();
      return(0);
      
    case CONTEXT_CELL:
      return(0);
      
    case CONTEXT_TABLE:
      return(0);
      
      // TODO -- HANDLE OTHER CONTEXTS
    default:
      fprintf(stderr, "fltk3::Table::find_cell: unknown context %d\n", (int)context);
      return(-1);
  }
  //NOTREACHED
}

// Enable automatic scroll-selection
void fltk3::Table::_start_auto_drag() {
  if (_auto_drag) return;
  _auto_drag = 1;
  fltk3::add_timeout(0.3, _auto_drag_cb2, this);
}

// Disable automatic scroll-selection
void fltk3::Table::_stop_auto_drag() {
  if (!_auto_drag) return;
  fltk3::remove_timeout(_auto_drag_cb2, this);
  _auto_drag = 0;
}

void fltk3::Table::_auto_drag_cb2(void *d) {
  ((fltk3::Table*)d)->_auto_drag_cb();
}

// Handle automatic scroll-selection if mouse selection dragged off table edge
void fltk3::Table::_auto_drag_cb() {
  int lx = fltk3::e_x;
  int ly = fltk3::e_y;
  if (_selecting == CONTEXT_COL_HEADER)
  { ly = y() + col_header_height(); }
  else if (_selecting == CONTEXT_ROW_HEADER)
  { lx = x() + row_header_width(); }
  if (lx > x() + w() - 20) {
    fltk3::e_x = x() + w() - 20;
    if (hscrollbar->visible())
      ((fltk3::Slider*)hscrollbar)->value(hscrollbar->clamp(hscrollbar->value() + 30));
    hscrollbar->do_callback();
    _dragging_x = fltk3::e_x - 30;
  }
  else if (lx < (x() + row_header_width())) {
    fltk3::e_x = x() + row_header_width() + 1;
    if (hscrollbar->visible()) {
      ((fltk3::Slider*)hscrollbar)->value(hscrollbar->clamp(hscrollbar->value() - 30));
    }
    hscrollbar->do_callback();
    _dragging_x = fltk3::e_x + 30;
  }
  if (ly > y() + h() - 20) {
    fltk3::e_y = y() + h() - 20;
    if (vscrollbar->visible()) {
      ((fltk3::Slider*)vscrollbar)->value(vscrollbar->clamp(vscrollbar->value() + 30));
    }
    vscrollbar->do_callback();
    _dragging_y = fltk3::e_y - 30;
  }
  else if (ly < (y() + col_header_height())) {
    fltk3::e_y = y() + col_header_height() + 1;
    if (vscrollbar->visible()) {
      ((fltk3::Slider*)vscrollbar)->value(vscrollbar->clamp(vscrollbar->value() - 30));
    }
    vscrollbar->do_callback();
    _dragging_y = fltk3::e_y + 30;
  }
  _auto_drag = 2;
  handle(fltk3::DRAG);
  _auto_drag = 1;
  fltk3::e_x = lx;
  fltk3::e_y = ly;
  fltk3::check();
  fltk3::flush();
  if (fltk3::event_buttons() && _auto_drag) {
    fltk3::add_timeout(0.05, _auto_drag_cb2, this);
  }
}

// Recalculate the window dimensions
void fltk3::Table::recalc_dimensions() {
  // Recalc to* (Table Outer), ti* (Table Inner), wi* ( Widget Inner)
  wix = ( x() + fltk3::box_dx(box())); tox = wix; tix = tox + fltk3::box_dx(table->box());
  wiy = ( y() + fltk3::box_dy(box())); toy = wiy; tiy = toy + fltk3::box_dy(table->box());
  wiw = ( w() - fltk3::box_dw(box())); tow = wiw; tiw = tow - fltk3::box_dw(table->box());
  wih = ( h() - fltk3::box_dh(box())); toh = wih; tih = toh - fltk3::box_dh(table->box());
  // Trim window if headers enabled
  if ( col_header() ) {
    tiy += col_header_height(); toy += col_header_height();
    tih -= col_header_height(); toh -= col_header_height();
  }
  if ( row_header() ) {
    tix += row_header_width(); tox += row_header_width();
    tiw -= row_header_width(); tow -= row_header_width();
  } 
  // Make scroll bars disappear if window large enough
  {
    // First pass: can hide via window size?
    int hidev = (table_h <= tih);
    int hideh = (table_w <= tiw); 
    // Second pass: Check for interference
    if ( !hideh & hidev ) { hidev = (( table_h - tih + SCROLLBAR_SIZE ) <= 0 ); } 
    if ( !hidev & hideh ) { hideh = (( table_w - tiw + SCROLLBAR_SIZE ) <= 0 ); } 
    // Determine scrollbar visibility, trim ti[xywh]/to[xywh]
    if ( hidev ) { vscrollbar->hide(); } 
    else { vscrollbar->show(); tiw -= SCROLLBAR_SIZE; tow -= SCROLLBAR_SIZE; }
    if ( hideh ) { hscrollbar->hide(); } 
    else { hscrollbar->show(); tih -= SCROLLBAR_SIZE; toh -= SCROLLBAR_SIZE; }
  } 
  // Resize the child table
  table->resize(tox, toy, tow, toh);
  table->init_sizes();
}

// Recalculate internals after a scroll.
//
//    Call this if table has been scrolled or resized.
//    Does not handle redraw().
//    TODO: Assumes ti[xywh] has already been recalculated.
//
void fltk3::Table::table_scrolled() {
  // Find top row
  int y, row, voff = vscrollbar->value();
  for ( row=y=0; row < _rows; row++ ) {
    y += row_height(row);
    if ( y > voff ) { y -= row_height(row); break; }
  }
  _row_position = toprow = ( row >= _rows ) ? (row - 1) : row;
  toprow_scrollpos = y;		// OPTIMIZATION: save for later use 
  // Find bottom row
  voff = vscrollbar->value() + tih;
  for ( ; row < _rows; row++ ) {
    y += row_height(row);
    if ( y >= voff ) { break; }
  }
  botrow = ( row >= _rows ) ? (row - 1) : row; 
  // Left column
  int x, col, hoff = hscrollbar->value();
  for ( col=x=0; col < _cols; col++ ) {
    x += col_width(col);
    if ( x > hoff ) { x -= col_width(col); break; }
  }
  _col_position = leftcol = ( col >= _cols ) ? (col - 1) : col;
  leftcol_scrollpos = x;	// OPTIMIZATION: save for later use 
  // Right column
  //    Work with data left over from leftcol calculation
  //
  hoff = hscrollbar->value() + tiw;
  for ( ; col < _cols; col++ ) {
    x += col_width(col);
    if ( x >= hoff ) { break; }
  }
  rightcol = ( col >= _cols ) ? (col - 1) : col; 
  // First tell children to scroll
  draw_cell(CONTEXT_RC_RESIZE, 0,0,0,0,0,0);
}

// Table resized: recalc internal data
//    Call this whenever the window is resized.
//    Recalculates the scrollbar sizes.
//    Makes no assumptions about any pre-initialized data.
//
void fltk3::Table::table_resized() {
  table_h = row_scroll_position(rows());
  table_w = col_scroll_position(cols()); 
  recalc_dimensions(); 
  // Recalc scrollbar sizes
  //    Clamp scrollbar value() after a resize.
  //    Resize scrollbars to enforce a constant trough width after a window resize.
  //
  {
    // Vertical scrollbar
    float vscrolltab = ( table_h == 0 || tih > table_h ) ? 1 : (float)tih / table_h;
    float hscrolltab = ( table_w == 0 || tiw > table_w ) ? 1 : (float)tiw / table_w;
    vscrollbar->bounds(0, table_h-tih);
    vscrollbar->precision(10);
    vscrollbar->slider_size(vscrolltab);
    vscrollbar->resize(wix+wiw-SCROLLBAR_SIZE, wiy,
                       SCROLLBAR_SIZE, 
                       wih - ((hscrollbar->visible())?SCROLLBAR_SIZE:0));
    vscrollbar->fltk3::Valuator::value(vscrollbar->clamp(vscrollbar->value()));	
    // Horizontal scrollbar
    hscrollbar->bounds(0, table_w-tiw);
    hscrollbar->precision(10);
    hscrollbar->slider_size(hscrolltab);
    hscrollbar->resize(wix, wiy+wih-SCROLLBAR_SIZE,
                       wiw - ((vscrollbar->visible())?SCROLLBAR_SIZE:0), 
                       SCROLLBAR_SIZE);
    hscrollbar->fltk3::Valuator::value(hscrollbar->clamp(hscrollbar->value()));
  }
  
  // Tell FLTK child widgets were resized
  fltk3::Group::init_sizes();
  
  // Recalc top/bot/left/right
  table_scrolled();
  
  // DO *NOT* REDRAW -- LEAVE THIS UP TO THE CALLER
  // redraw();
}

// Someone moved a scrollbar
void fltk3::Table::scroll_cb(fltk3::Widget*w, void *data) {
  fltk3::Table *o = (fltk3::Table*)data;
  o->recalc_dimensions();	// recalc tix, tiy, etc.
  o->table_scrolled();
  o->redraw();
}

// Set number of rows
void fltk3::Table::rows(int val) {
  int oldrows = _rows;
  _rows = val;
  {
    int default_h = ( _rowheights.size() > 0 ) ? _rowheights.back() : 25;
    int now_size = _rowheights.size();
    _rowheights.size(val);			// enlarge or shrink as needed
    while ( now_size < val ) {
      _rowheights[now_size++] = default_h;	// fill new
    }
  }
  table_resized();
  
  // OPTIMIZATION: redraw only if change is visible.
  if ( val >= oldrows && oldrows > botrow ) {
    // NO REDRAW
  } else {
    redraw();
  }
}

// Set number of cols
void fltk3::Table::cols(int val) {
  _cols = val;
  {
    int default_w = ( _colwidths.size() > 0 ) ? _colwidths[_colwidths.size()-1] : 80;
    int now_size = _colwidths.size();
    _colwidths.size(val);			// enlarge or shrink as needed
    while ( now_size < val ) {
      _colwidths[now_size++] = default_w;	// fill new
    }
  }
  table_resized();
  redraw();
}

// Change mouse cursor to different type
void fltk3::Table::change_cursor(fltk3::Cursor newcursor) {
  if ( newcursor != _last_cursor ) {
    fltk3::cursor(newcursor, fltk3::BLACK, fltk3::WHITE);
    _last_cursor = newcursor;
  }
}

void fltk3::Table::damage_zone(int r1, int c1, int r2, int c2, int r3, int c3) {
  int R1 = r1, C1 = c1;
  int R2 = r2, C2 = c2;
  if (r1 > R2) R2 = r1;
  if (r2 < R1) R1 = r2;
  if (r3 > R2) R2 = r3;
  if (r3 < R1) R1 = r3;
  if (c1 > C2) C2 = c1;
  if (c2 < C1) C1 = c2;
  if (c3 > C2) C2 = c3;
  if (c3 < C1) C1 = c3;
  if (R1 < 0) {
    if (R2 < 0) return;
    R1 = 0;
  }
  if (C1 < 0) {
    if (C2 < 0) return;
    C1 = 0;
  }
  if (R1 < toprow) R1 = toprow;
  if (R2 > botrow) R2 = botrow;
  if (C1 < leftcol) C1 = leftcol;
  if (C2 > rightcol) C2 = rightcol;
  redraw_range(R1, R2, C1, C2);
}

int fltk3::Table::move_cursor(int R, int C) {
  if (select_row == -1) R++;
  if (select_col == -1) C++;
  R += select_row;
  C += select_col;
  if (R < 0) R = 0;
  if (R >= rows()) R = rows() - 1;
  if (C < 0) C = 0;
  if (C >= cols()) C = cols() - 1;
  if (R == select_row && C == select_col) return 0;
  damage_zone(current_row, current_col, select_row, select_col, R, C);
  select_row = R;
  select_col = C;
  if (!fltk3::event_state(fltk3::SHIFT)) {
    current_row = R;
    current_col = C;
  }
  if (R < toprow + 1 || R > botrow - 1) row_position(R);
  if (C < leftcol + 1 || C > rightcol - 1) col_position(C);
  return 1;
}

// #define DEBUG 1
#ifdef DEBUG
#include "eventnames.h"
#define PRINTEVENT \
fprintf(stderr,"Table %s: ** Event: %s --\n", (label()?label():"none"), eventnames[event]);
#else
#define PRINTEVENT
#endif

// Handle FLTK events
int fltk3::Table::handle(int event) {
  FLTK3_OBJECT_VCALLS_WRAPPER_INT(handle(event), Handle)
  PRINTEVENT;
  int ret = fltk3::Group::handle(event);	// let FLTK group handle events first
  if (ret) {
    if (fltk3::event_inside(hscrollbar) || fltk3::event_inside(vscrollbar)) return 1;
    if (fltk3::focus() != this && contains(fltk3::focus())) return 1;
  }
  // Which row/column are we over?
  int R, C;  				// row/column being worked on
  ResizeFlag resizeflag;		// which resizing area are we over? (0=none)
  TableContext context = cursor2rowcol(R, C, resizeflag);
  switch ( event ) {
    case fltk3::PUSH:
      if (fltk3::event_button() == 1 && !fltk3::event_clicks()) {
        if (fltk3::focus() != this) {
          take_focus();
          do_callback(CONTEXT_TABLE, -1, -1);
          ret = 1;
        }
        damage_zone(current_row, current_col, select_row, select_col, R, C);
        if (context == CONTEXT_CELL) {
          current_row = select_row = R;
          current_col = select_col = C;
          _selecting = CONTEXT_CELL;
        } else {
          current_row = select_row = -1;
          current_col = select_col = -1;
        }
      }
      // Need this for eg. right click to pop up a menu
      if ( fltk3::Widget::callback() &&		// callback defined?
          resizeflag == RESIZE_NONE ) {	// not resizing?
        do_callback(context, R, C);		// do callback
      }
      switch ( context ) {
        case CONTEXT_CELL:
          // fltk3::PUSH on a cell?
          ret = 1; 			// express interest in fltk3::RELEASE
          break;
          
        case CONTEXT_NONE:
          // fltk3::PUSH on table corner?
          if ( fltk3::event_button() == 1 && 
              fltk3::event_x() < x() + row_header_width()) {
            current_col = 0;
            select_col = cols() - 1;
            current_row = 0;
            select_row = rows() - 1;				
            damage_zone(current_row, current_col, select_row, select_col);
            ret = 1;
          }
          break;
          
        case CONTEXT_COL_HEADER:
          // fltk3::PUSH on a column header?
          if ( fltk3::event_button() == 1) {
            // Resizing? Handle it
            if ( resizeflag ) {
              // Start resize if left click on column border.
              //    "ret=1" ensures we get drag events from now on.
              //    (C-1) is used if mouse is over the left hand side 
              //    of cell, so we resize the next column on the left.
              //
              _resizing_col = ( resizeflag & RESIZE_COL_LEFT ) ? C-1 : C; 
              _resizing_row = -1;
              _dragging_x = fltk3::event_x(); 
              ret = 1;
            } else {
              // Not resizing? Select the column
              current_col = select_col = C;
              current_row = 0;
              select_row = rows() - 1;
              _selecting = CONTEXT_COL_HEADER;
              damage_zone(current_row, current_col, select_row, select_col);
              ret = 1;
            }
          }
          break;
          
        case CONTEXT_ROW_HEADER:
          // fltk3::PUSH on a row header?
          if ( fltk3::event_button() == 1 ) {
            // Resizing? Handle it
            if ( resizeflag ) {
              // Start resize if left mouse clicked on row border.
              //    "ret = 1" ensures we get drag events from now on.
              //    (R-1) is used if mouse is over the top of the cell,
              //    so that we resize the row above.
              //
              _resizing_row = ( resizeflag & RESIZE_ROW_ABOVE ) ? R-1 : R; 
              _resizing_col = -1;
              _dragging_y = fltk3::event_y(); 
              ret = 1;
            } else {
              // Not resizing? Select the row
              current_row = select_row = R;
              current_col = 0;
              select_col = cols() - 1;
              _selecting = CONTEXT_ROW_HEADER;
              damage_zone(current_row, current_col, select_row, select_col);
              ret = 1;
            }
          }
          break;
          
        default:
          ret = 0;		// express disinterest
          break;
      }
      _last_row = R;
      break;
      
    case fltk3::DRAG:
      if (_auto_drag == 1) {
        ret = 1;
        break;
      } 
      if ( _resizing_col > -1 ) {
        // Dragging column?
        //
        //    Let user drag even /outside/ the row/col widget.
        //    Don't allow column width smaller than 1.
        //    Continue to show fltk3::CURSOR_WE at all times during drag.
        //
        int offset = _dragging_x - fltk3::event_x();
        int new_w = col_width(_resizing_col) - offset;
        if ( new_w < _col_resize_min ) new_w = _col_resize_min;
        col_width(_resizing_col, new_w);
        _dragging_x = fltk3::event_x();
        table_resized();
        redraw();
        change_cursor(fltk3::CURSOR_WE);
        ret = 1;
        if ( fltk3::Widget::callback() && when() & fltk3::WHEN_CHANGED ) {
          do_callback(CONTEXT_RC_RESIZE, R, C);
        }
      }
      else if ( _resizing_row > -1 ) {
        // Dragging row?
        //
        //    Let user drag even /outside/ the row/col widget.
        //    Don't allow row width smaller than 1.
        //    Continue to show fltk3::CURSOR_NS at all times during drag.
        //
        int offset = _dragging_y - fltk3::event_y();
        int new_h = row_height(_resizing_row) - offset;
        if ( new_h < _row_resize_min ) new_h = _row_resize_min;
        row_height(_resizing_row, new_h);
        _dragging_y = fltk3::event_y();
        table_resized();
        redraw();
        change_cursor(fltk3::CURSOR_NS);
        ret = 1;
        if ( fltk3::Widget::callback() && when() & fltk3::WHEN_CHANGED ) {
          do_callback(CONTEXT_RC_RESIZE, R, C);
        }
      } else {
        if (fltk3::event_button() == 1 && 
            _selecting == CONTEXT_CELL &&
            context == CONTEXT_CELL) {
          if (select_row != R || select_col != C) {
            damage_zone(current_row, current_col, select_row, select_col, R, C);
          }
          select_row = R;
          select_col = C;
          ret = 1;
        }
        else if (fltk3::event_button() == 1 && 
                 _selecting == CONTEXT_ROW_HEADER && 
                 context & (CONTEXT_ROW_HEADER|CONTEXT_COL_HEADER|CONTEXT_CELL)) {
          if (select_row != R) {
            damage_zone(current_row, current_col, select_row, select_col, R, C);
          }
          select_row = R;
          ret = 1;
        }
        else if (fltk3::event_button() == 1 && 
                 _selecting == CONTEXT_COL_HEADER 
                 && context & (CONTEXT_ROW_HEADER|CONTEXT_COL_HEADER|CONTEXT_CELL)) {
          if (select_col != C) {
            damage_zone(current_row, current_col, select_row, select_col, R, C);
          }
          select_col = C;
          ret = 1;
        }
      }
      // Enable autodrag if not resizing, and mouse has moved off table edge
      if ( _resizing_row < 0 && _resizing_col < 0 && _auto_drag == 0 && 
          ( fltk3::event_x() > x() + w() - 20 ||
           fltk3::event_x() < x() + row_header_width() || 
           fltk3::event_y() > y() + h() - 20 ||
           fltk3::event_y() < y() + col_header_height()
           ) ) {
            _start_auto_drag();
          }
      break;
      
    case fltk3::RELEASE:
      _stop_auto_drag();
      switch ( context ) {
        case CONTEXT_ROW_HEADER:		// release on row header
        case CONTEXT_COL_HEADER:		// release on col header
        case CONTEXT_CELL:			// release on a cell
        case CONTEXT_TABLE:			// release on dead zone
          if ( _resizing_col == -1 &&		// not resizing a column
              _resizing_row == -1 &&		// not resizing a row
              fltk3::Widget::callback() && 	// callback defined
              when() & fltk3::WHEN_RELEASE && 	// on button release
              _last_row == R ) {		// release on same row PUSHed?
            // Need this for eg. left clicking on a cell to select it
            do_callback(context, R, C);
          }
          break;
          
        default:
          break;
      }
      if ( fltk3::event_button() == 1 ) {
        change_cursor(fltk3::CURSOR_DEFAULT);
        _resizing_col = -1;
        _resizing_row = -1;
        ret = 1;
      }
      break;
      
    case fltk3::MOVE:
      if ( context == CONTEXT_COL_HEADER && 		// in column header?
          resizeflag ) {				// resize + near boundary?
        change_cursor(fltk3::CURSOR_WE);			// show resize cursor
      }
      else if ( context == CONTEXT_ROW_HEADER && 	// in row header?
               resizeflag ) {				// resize + near boundary?
        change_cursor(fltk3::CURSOR_NS);			// show resize cursor
      } else {
        change_cursor(fltk3::CURSOR_DEFAULT);		// normal cursor
      }
      ret = 1;
      break;
      
    case fltk3::ENTER:		// See FLTK event docs on the fltk3::ENTER widget
      if (!ret) take_focus();
      ret = 1;
      //FALLTHROUGH
      
    case fltk3::LEAVE:		// We want to track the mouse if resizing is allowed.
      if ( resizeflag ) {
        ret = 1;
      }
      if ( event == fltk3::LEAVE ) {
        _stop_auto_drag();
        change_cursor(fltk3::CURSOR_DEFAULT);
      }
      break;
      
    case fltk3::FOCUS:
      fltk3::focus(this);
      //FALLTHROUGH
      
    case fltk3::UNFOCUS:
      _stop_auto_drag();
      ret = 1;
      break;
      
    case fltk3::KEYBOARD: {
      ret = 0;
      int is_row = select_row;
      int is_col = select_col;
      switch(fltk3::event_key()) {
        case fltk3::HomeKey:
          ret = move_cursor(0, -1000000);
          break;
        case fltk3::EndKey:
          ret = move_cursor(0, 1000000);
          break;
        case fltk3::PageUpKey:
          ret = move_cursor(-(botrow - toprow - 1), 0);
          break;
        case fltk3::PageDownKey:
          ret = move_cursor(botrow - toprow - 1 , 0);
          break;
        case fltk3::LeftKey:
          ret = move_cursor(0, -1);
          break;
        case fltk3::RightKey:
          ret = move_cursor(0, 1);
          break;
        case fltk3::UpKey:
          ret = move_cursor(-1, 0);
          break;
        case fltk3::DownKey:
          ret = move_cursor(1, 0);
          break;
	case fltk3::TabKey:
	  if ( fltk3::event_state() & fltk3::SHIFT ) {
            ret = move_cursor(0, -1);		// shift-tab -> left
	  } else {
	    ret = move_cursor(0, 1);		// tab -> right
	  }
          break;
      }
      if (ret && fltk3::focus() != this) {
        do_callback(CONTEXT_TABLE, -1, -1);
        take_focus();
      }
      //if (!ret && fltk3::Widget::callback() && when() & fltk3::WHEN_NOT_CHANGED  )
      if ( fltk3::Widget::callback() && 
          (
           ( !ret && when() & fltk3::WHEN_NOT_CHANGED ) || 
           ( is_row!= select_row || is_col!= select_col ) 
           )
          ) {
        do_callback(CONTEXT_CELL, select_row, select_col); 
        //damage_zone(current_row, current_col, select_row, select_col);
        ret = 1;
      }
      break;
    }
      
    default:
      change_cursor(fltk3::CURSOR_DEFAULT);
      break;
  }
  return(ret);
}

// Resize FLTK override
//     Handle resize events if user resizes parent window.
//
void fltk3::Table::resize(int X, int Y, int W, int H) {
  // Tell group to resize, and recalc our own widget as well
  fltk3::Group::resize(X, Y, W, H);
  table_resized();
  redraw();
}

// Draw a cell
void fltk3::Table::_redraw_cell(TableContext context, int r, int c) {
  if ( r < 0 || c < 0 ) return;
  int X,Y,W,H;
  find_cell(context, r, c, X, Y, W, H);	// find positions of cell
  draw_cell(context, r, c, X, Y, W, H);	// call users' function to draw it
}

/**
 See if the cell at row \p r and column \p c is selected.
 \returns 1 if the cell is selected, 0 if not.
 */
int fltk3::Table::is_selected(int r, int c) {
  int s_left, s_right, s_top, s_bottom;
  
  if (select_col > current_col) {
    s_left = current_col;
    s_right = select_col;
  } else {
    s_right = current_col;
    s_left = select_col;
  }
  if (select_row > current_row) {
    s_top = current_row;
    s_bottom = select_row;
  } else {
    s_bottom = current_row;
    s_top = select_row;
  }
  if (r >= s_top && r <= s_bottom && c >= s_left && c <= s_right) {
    return 1;
  }
  return 0;
}

/**
  Gets the region of cells selected (highlighted).

  \param[in] row_top   Returns the top row of selection area
  \param[in] col_left  Returns the left column of selection area
  \param[in] row_bot   Returns the bottom row of selection area
  \param[in] col_right Returns the right column of selection area
*/
void fltk3::Table::get_selection(int& row_top, int& col_left, int& row_bot, int& col_right) {
  if (select_col > current_col) {
    col_left  = current_col;
    col_right = select_col;
  } else {
    col_right = current_col;
    col_left  = select_col;
  }
  if (select_row > current_row) {
    row_top = current_row;
    row_bot = select_row;
  } else {
    row_bot = current_row;
    row_top = select_row;
  }
}

/**
  Sets the region of cells to be selected (highlighted).

  So for instance, set_selection(0,0,0,0) selects the top/left cell in the table.
  And set_selection(0,0,1,1) selects the four cells in rows 0 and 1, column 0 and 1.

  \param[in] row_top   Top row of selection area
  \param[in] col_left  Left column of selection area
  \param[in] row_bot   Bottom row of selection area
  \param[in] col_right Right column of selection area
*/
void fltk3::Table::set_selection(int row_top, int col_left, int row_bot, int col_right) {
  damage_zone(current_row, current_col, select_row, select_col);
  current_col = col_left;
  current_row = row_top;
  select_col  = col_right;
  select_row  = row_bot;
  damage_zone(current_row, current_col, select_row, select_col);
}

// Draw the entire fltk3::Table
//    Override the draw() routine to draw the table.
//    Then tell the group to draw over us.
//
void fltk3::Table::draw() {   
  FLTK3_OBJECT_VCALLS_WRAPPER(draw(), Draw)
  draw_cell(CONTEXT_STARTPAGE, 0, 0,	 	// let user's drawing routine
            tix, tiy, tiw, tih);		// prep new page
  
  // Let fltk widgets draw themselves first. Do this after
  // draw_cell(CONTEXT_STARTPAGE) in case user moves widgets around.
  // Use window 'inner' clip to prevent drawing into table border.
  // (unfortunately this clips FLTK's border, so we must draw it explicity below)
  //
  fltk3::push_clip(wix, wiy, wiw, wih);
  {
    fltk3::Group::draw();
  }
  fltk3::pop_clip();
  
  // Explicitly draw border around widget, if any
  draw_box(box(), x(), y(), w(), h(), color());
  
  // If fltk3::ScrollGroup 'table' is hidden, draw its box
  //    Do this after fltk3::Group::draw() so we draw over scrollbars
  //    that leak around the border.
  //
  if ( ! table->visible() ) {
    if ( damage() & fltk3::DAMAGE_ALL || damage() & fltk3::DAMAGE_CHILD ) {
      draw_box(table->box(), tox, toy, tow, toh, table->color());
    }
  } 
  // Clip all further drawing to the inner widget dimensions
  fltk3::push_clip(wix, wiy, wiw, wih);
  {
    // Only redraw a few cells?
    if ( ! ( damage() & fltk3::DAMAGE_ALL ) && _redraw_leftcol != -1 ) {
      fltk3::push_clip(tix, tiy, tiw, tih);
      for ( int c = _redraw_leftcol; c <= _redraw_rightcol; c++ ) {
        for ( int r = _redraw_toprow; r <= _redraw_botrow; r++ ) { 
          _redraw_cell(CONTEXT_CELL, r, c);
        }
      }
      fltk3::pop_clip();
    }
    if ( damage() & fltk3::DAMAGE_ALL ) {
      int X,Y,W,H;
      // Draw row headers, if any
      if ( row_header() ) {
        get_bounds(CONTEXT_ROW_HEADER, X, Y, W, H);
        fltk3::push_clip(X,Y,W,H);
        for ( int r = toprow; r <= botrow; r++ ) {
          _redraw_cell(CONTEXT_ROW_HEADER, r, 0);
        }
        fltk3::pop_clip();
      }
      // Draw column headers, if any
      if ( col_header() ) {
        get_bounds(CONTEXT_COL_HEADER, X, Y, W, H);
        fltk3::push_clip(X,Y,W,H);
        for ( int c = leftcol; c <= rightcol; c++ ) {
          _redraw_cell(CONTEXT_COL_HEADER, 0, c);
        }
        fltk3::pop_clip();
      } 
      // Draw all cells.
      //    This includes cells partially obscured off edges of table.
      //    No longer do this last; you might think it would be nice
      //    to draw over dead zones, but on redraws it flickers. Avoid
      //    drawing over deadzones; prevent deadzones by sizing columns.
      //
      fltk3::push_clip(tix, tiy, tiw, tih); {
        for ( int r = toprow; r <= botrow; r++ ) {
          for ( int c = leftcol; c <= rightcol; c++ ) {
            _redraw_cell(CONTEXT_CELL, r, c); 
          }
        }
      }
      fltk3::pop_clip(); 
      // Draw little rectangle in corner of headers
      if ( row_header() && col_header() ) {
        fltk3::rectf(wix, wiy, row_header_width(), col_header_height(), color());
      }
      
      // Table has a boxtype? Close those few dead pixels
      if ( table->box() ) {
        if ( col_header() ) {
          fltk3::rectf(tox, wiy, fltk3::box_dx(table->box()), col_header_height(), color());
        }
        if ( row_header() ) {
          fltk3::rectf(wix, toy, row_header_width(), fltk3::box_dx(table->box()), color());
        }
      }
      
      // Table width smaller than window? Fill remainder with rectangle
      if ( table_w < tiw ) {
        fltk3::rectf(tix + table_w, tiy, tiw - table_w, tih, color()); 
        // Col header? fill that too
        if ( col_header() ) {
          fltk3::rectf(tix + table_w, 
                   wiy, 
                   // get that corner just right..
                   (tiw - table_w + fltk3::box_dw(table->box()) - 
                    fltk3::box_dx(table->box())),
                   col_header_height(),
                   color());
        }
      } 
      // Table height smaller than window? Fill remainder with rectangle
      if ( table_h < tih ) {
        fltk3::rectf(tix, tiy + table_h, tiw, tih - table_h, color()); 
        if ( row_header() ) {
          // NOTE:
          //     Careful with that lower corner; don't use tih; when eg. 
          //     table->box(fltk3::THIN_UPFRAME) and hscrollbar hidden, 
          //     leaves a row of dead pixels.
          //
          fltk3::rectf(wix, tiy + table_h, row_header_width(), 
                   (wiy+wih) - (tiy+table_h) - 
                   ( hscrollbar->visible() ? SCROLLBAR_SIZE : 0),
                   color());
        }
      }
    } 
    // Both scrollbars? Draw little box in lower right
    if ( vscrollbar->visible() && hscrollbar->visible() ) {
      fltk3::rectf(vscrollbar->x(), hscrollbar->y(), 
               vscrollbar->w(), hscrollbar->h(), color());
    } 
    draw_cell(CONTEXT_ENDPAGE, 0, 0,		// let user's drawing
              tix, tiy, tiw, tih);		// routines cleanup
    
    _redraw_leftcol = _redraw_rightcol = _redraw_toprow = _redraw_botrow = -1;
  }
  fltk3::pop_clip();
}

//
// End of "$Id$".
//
