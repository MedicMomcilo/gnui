//
// "$Id: Fl_Choice.cxx,v 1.23 1999/11/10 04:48:49 carl Exp $"
//
// Choice widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

#include <FL/Fl.H>
#include <FL/Fl_Choice.H>
#include <FL/fl_draw.H>

// Emulates the Forms choice widget.  This is almost exactly the same
// as an Fl_Menu_Button.  The only difference is the appearance of the
// button: it draws the text of the current pick and a down-arrow.

extern char fl_draw_shortcut;

void Fl_Choice::draw() {
  draw_button();
  // draw the tick mark:
  int H = labelsize()/2+1;
  FL_THIN_UP_BOX->draw(x()+w()-3*H, y()+(h()-H)/2, 2*H, H,
		       color(), FL_FRAME_ONLY);
  if (mvalue()) {
    // Shouldn't do this, but we can handle it
    Fl_Menu_Item m = *mvalue();
    if (active_r()) m.activate(); else m.deactivate();
    fl_clip(x(), y(), w()-3*H, h());
    fl_draw_shortcut = 2; // hack value to make '&' disappear
    Fl_Boxtype b = box();
    m.draw(x()+b->dx(), y()+b->dy(), w()-b->dw()-3*H, h()-b->dh(), this, 0, 0);
    fl_draw_shortcut = 0;
    fl_pop_clip();
  }
}

Fl_Choice::Fl_Choice(int x,int y,int w,int h, const char *l) : Fl_Menu_(x,y,w,h,l) {
  style(default_style);
  align(FL_ALIGN_LEFT);
  when(FL_WHEN_RELEASE);
}

int Fl_Choice::value(int v) {
  if (!Fl_Menu_::value(v)) return 0;
  redraw();
  return 1;
}

int Fl_Choice::handle(int e) {
  if (!menu() || !menu()->text) return 0;
  const Fl_Menu_Item* v;
  switch (e) {
  case FL_PUSH:
    //Fl::event_is_click(0);
  J1:
    v = menu()->pulldown(x(), y(), w(), h(), mvalue(), this);
    if (!v || v->submenu()) return 1;
    if (v != mvalue()) redraw();
    picked(v);
    return 1;
  case FL_SHORTCUT:
    if (Fl_Widget::test_shortcut()) goto J1;
    v = menu()->test_shortcut();
    if (!v) return 0;
    if (v != mvalue()) redraw();
    picked(v);
    return 1;
  case FL_ENTER:
  case FL_LEAVE:
    if (highlight_color() && takesevents()) damage(FL_DAMAGE_HIGHLIGHT);
    return 1;
  default:
    return 0;
  }
}

Fl_Style Fl_Choice::default_style;
static Fl_Style_Definer x("choice", Fl_Choice::default_style);

//
// End of "$Id: Fl_Choice.cxx,v 1.23 1999/11/10 04:48:49 carl Exp $".
//
