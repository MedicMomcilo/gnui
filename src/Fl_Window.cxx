//
// "$Id: Fl_Window.cxx,v 1.104 2003/10/28 17:45:15 spitzak Exp $"
//
// Window widget class for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2003 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

// The Window is a window in the fltk library.
// This is the system-independent portions.  The huge amount of
// crap you need to do to communicate with X is in x.C, the
// equivalent (but totally different) crap for MSWindows is in win32.C

#include <fltk/Window.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/layout.h>
#include <fltk/run.h>
#include <fltk/x.h>
#include <config.h>
using namespace fltk;

Window *Widget::window() const {
  for (Widget *o = parent(); o; o = o->parent())
    if (o->is_window()) return (Window*)o;
  return 0;
}

void Window::draw() {Group::draw();}

void Window::label(const char *name) {label(name, iconlabel());}

void Window::iconlabel(const char *iname) {label(label(), iname);}

void Window::default_callback(Window* window, void*) {
  window->hide();
  // if there are no visible windows we exit:
  // Not anymore, it has been restored to fltk 1.0 behavior. Instead
  // run() checks after each loop to see if all windows are closed.
  // if (!first()) exit(0);
}

static void revert(fltk::Style* s) {
  s->color = GRAY75;
  s->box = FLAT_BOX;
}
static NamedStyle style("Window", revert, &Window::default_style);
NamedStyle* Window::default_style = &::style;

void Window::_Window() {
  style(default_style);
  type(WINDOW_TYPE);
  i = 0;
  icon_ = 0;
  iconlabel_ = 0;
  //resizable(0); // new default for group
  minw = minh = maxw = maxh = 0;
  size_range_set = 0;
  child_of_ = 0;
  shortcut(EscapeKey);
  callback((Callback*)default_callback);
}

Window::Window(int X,int Y,int W, int H, const char *l, bool begin)
: Group(X, Y, W, H, l, begin) {
  _Window();
}

Window::Window(int W, int H, const char *l)
// fix common user error of a missing end() with current(0):
  : Group((Group::current(0),USEDEFAULT), USEDEFAULT, W, H, l) {
  _Window();
  clear_visible();
}

// SHOW events will normally create and map the window, HIDE will
// unmap.  On both X and Win32 creating a window requires a lot of ugly
// cruft, some of it is here and much of it is in the machine-specific
// code like x.cxx.  There are also static variables (!) used to
// modify how the window is created, such as to create it iconized or
// to create it with a parent.

// This is set by arg to argv[0], or the user can set it.
// It is used by X to look up stuff in the X resource database:
const char* Window::xclass_ = "fltk";

bool fl_show_iconic; // set by iconize() or by -i arg switch

#ifdef _WIN32
static void keep_app_active() {
  // activate some other window so the active app does not change!
  if (grab()) return;
  for (CreatedWindow* x = CreatedWindow::first; x; x = x->next)
    if (!x->window->parent() && x->window->visible()) {
      //BringWindowToTop(x->xid);
      SetActiveWindow(x->xid);
      break;
    }
}
#endif

extern Window* fl_actual_window; // in Fl.cxx

extern Window* fl_actual_window; // in Fl.cxx

int Window::handle(int event) {
  switch (event) {
  case SHOW:
    // create the window, which will recursively call this:
    if (!shown()) {show(); return 1;}
    Group::handle(event); // make the child windows map first
#ifdef _WIN32
    ShowWindow(i->xid, SW_RESTORE);
#elif (defined(__APPLE__) && !USE_X11)
    if (parent()) ; // needs to update clip and redraw...
    else ShowWindow(i->xid);
#else
    XMapWindow(xdisplay, i->xid);
#endif
    return 1;

  case HIDE:
    if (flags()&MODAL) modal(0, false);
    if (i) XUnmapWindow(xdisplay, i->xid);
#ifdef _WIN32
    keep_app_active();
#endif
    break;

  case PUSH:
    // If we are in a modal state, see if the user is clicking on
    // another window. If so, just raise this (the modal) window.
    if (modal()==this && fl_actual_window != this) {
      show();
      return 0;
    }
    break;
  }

  int ret = Group::handle(event); if (ret) return ret;

  // unused events can close windows or raise them:
  if (!parent()) switch (event) {
  case KEY:
  case SHORTCUT:
    // See if the user hit Escape (or whatever the shortcut is)
    // and close the window:
    if (test_shortcut()) {
      if (!event_clicks())// make repeating key not close everything
	do_callback();
      return 1;
    }
    break;
#if !defined(_WIN32) && !(defined(__APPLE__) && !USE_X11)
  case PUSH:
    // unused clicks raise the window.
    if (shown()) XMapRaised(xdisplay, i->xid);
#endif
  }
  return 0;
}

/** Cause the window to become visible.

    For inner windows (with a parent()) this just causes the window
    to appear. Currently no guarantee about stacking order is made.

    For a outer window (one with no parent()) this causes the window
    to appear on the screen, be de-iconized, and be raised to the top.
    Depending on child_of() settings of this window and of windows
    pointing to it, and on system and window manager settings, this
    may cause other windows to also be deiconized and raised, or this
    window may remain iconized.

    Window::show() is not a virtual override of Widget::show(), and
    you can call either one. The \e only difference is that outer windows
    are raised and deiconized by Window::show(), while Widget::show()
    only "maps" the window, which on most systems causes it to appear
    in the same stacking order it was in when hidden.

    On current systems the first time this is called is when the
    actual "system window" is created. Before that an fltk window is
    simply an internal data structure and is not visible by any
    outside code. Calling hide() will unmap this system window, but to
    actually go back to the non-window state you must call destroy()
    or ~Window(). The first time show() is called on any window is
    when fltk will call open_display() and load_theme(), this allows
    these expensive operations to be deferred as long as possible, and
    allows fltk programs to be written that need no display (as long
    as they never show a window).
*/
void Window::show() {
  // get rid of very common user bug: forgot end():
  Group::current(0);
  // Emulate the undocumented back-compatability modal() stuff:
  if (flags()&(MODAL|NON_MODAL)) {
    child_of(first()); // this may unmap window if it changes
    if (flags()&MODAL) modal(this, false);
  }

#if (defined(__APPLE__) && !USE_X11)
  if (!parent()) {
    // bring the application to the front
    ProcessSerialNumber psn;
    OSErr err = GetCurrentProcess( &psn );
    if ( err==noErr ) SetFrontProcess( &psn );
  }
#endif

  if (!shown()) {
    // Create an all-new system window.

    // If the window was created with the xywh constructor, the visible()
    // flag was left on, turn it off:
    clear_visible();

    // open the display:
    load_theme();
    open_display();
    layout();

    // back-compatability automatic size_range() based on resizable():
    if (!parent() && !size_range_set) {
      if (resizable()) {
	// find the innermost nested resizable():
	Widget *o = resizable();
	while (o->is_group()) {
	  Widget* p = ((Group*)o)->resizable();
	  if (!p || p == o) break;
	  o = p;
	}
	int minw = w(); if (o->w() > 72) minw -= (o->w()-72);
	int minh = h(); if (o->h() > 72) minh -= (o->h()-72);
	size_range(minw, minh, 0, 0);
      } else {
	size_range(w(), h(), w(), h());
      }
    }

    // Raise any parent windows, to get around stupid window managers
    // that think "child of" means "next to" rather than "above":
    //if (child_of() && !override()) ((Window*)child_of())->show();

    create();

    // create & map child windows:
    Group::handle(SHOW);

    // We must turn this flag on first so when silly Windows calls
    // the WinProc directly it is already on:
    set_visible();

    // map the window, making it iconized if fl_show_iconic is on:
#ifdef _WIN32
    int showtype;
    if (parent())
      showtype = SW_RESTORE;
    else if (!modal() && fl_show_iconic)
      showtype = SW_SHOWMINNOACTIVE,fl_show_iconic = false;
    // If we've captured the mouse, we don't want do activate any
    // other windows from the code, or we lose the capture.
    // Also, we don't want to activate the window for tooltips.
    else if (grab() || override())
      showtype = SW_SHOWNOACTIVATE;
    else
      showtype = SW_SHOWNORMAL;
    ShowWindow(i->xid, showtype);
#elif (defined(__APPLE__) && !USE_X11)
    if (!modal() && fl_show_iconic) {
      fl_show_iconic = 0;
      CollapseWindow( i->xid, true ); // \todo Mac ; untested
    } else {
      ShowWindow(i->xid);
    }
#else
    // for X, iconic stuff was done by create()
    XMapRaised(xdisplay, i->xid);
#endif

  } else {

    // Raise+deiconize an already-made top-level window:
    if (!parent()) {
      // Raise any parent windows, to get around stupid window managers
      // that think "child of" means "next to" rather than "above":
      //if (child_of() && !override()) ((Window*)child_of())->show();
#ifdef _WIN32
      if (IsIconic(i->xid)) OpenIcon(i->xid);
      if (!grab() && !override()) BringWindowToTop(i->xid);
#elif (defined(__APPLE__) && !USE_X11)
      ShowWindow(i->xid); // does this de-iconize?
      if (!grab() && !override()) {
	BringToFront(i->xid);
	SelectWindow(i->xid);
      }
#else
      XMapRaised(xdisplay, i->xid);
#endif
    }

    // This will call handle(SHOW) if visible is false, which will
    // cause the window to be mapped. Does nothing if visible is
    // already true:
    Widget::show();
  }
}

void Window::show(const Window* w) {
  child_of(w);
  show();
}

void Window::child_of(const Window* w) {
  if (contains(w)) return;
  while (w && w->parent()) w = w->window();
  if (child_of_ != w) destroy();
  child_of_ = w;
}

bool Window::exec(const Window* w, bool grab) {
  clear_value();
  child_of(w ? w : first());
  Widget* saved_modal = fltk::modal(); bool saved_grab = fltk::grab();
  fltk::modal(this, grab);
  show();
  while (modal() && !exit_modal_flag()) wait();
  hide();
  modal(saved_modal, saved_grab);
  return value();
}

#ifdef _WIN32
extern const Window* fl_mdi_window;
void Window::show_inside(const Window* w) {
  fl_mdi_window = w;
  show();
  fl_mdi_window = 0;
}
#else
void Window::show_inside(const Window* w) {
  show(w);
}
#endif

////////////////////////////////////////////////////////////////

void Widget::redraw(int X, int Y, int W, int H) {
  // go up to the window, clipping to each widget's area, quit if empty:
  Widget* window = this;
  for (;;) {
    if (X < 0) {W += X; X = 0;}
    if (Y < 0) {H += Y; Y = 0;}
    if (W > window->w()-X) W = window->w()-X;
    if (H > window->h()-Y) H = window->h()-Y;
    if (W <= 0 || H <= 0) return;
    if (window->is_window()) break;
    X += window->x();
    Y += window->y();
    window = window->parent();
    if (!window) return;
  }
  CreatedWindow* i = CreatedWindow::find((Window*)window);
  if (!i) return; // window not mapped, so ignore it
  window->damage_ |= DAMAGE_EXPOSE;
  i->expose(X, Y, W, H);
}

// Merge a rectangle into a window's expose region. If the entire
// window is damaged we switch to a DAMAGE_ALL mode which will
// avoid drawing it twice:
void CreatedWindow::expose(int X, int Y, int W, int H) {
  // Ignore if window already marked as completely damaged:
  if (window->damage() & DAMAGE_ALL) ;
  // Detect expose events that cover the entire window:
  else if (X<=0 && Y<=0 && W>=window->w() && H>=window->h()) {
    window->set_damage(DAMAGE_ALL);
  } else if (!region) {
    // create a new region:
    region = XRectangleRegion(X,Y,W,H);
  } else {
    // merge with the region:
#ifdef WIN32
    Region R = XRectangleRegion(X, Y, W, H);
    CombineRgn(region, region, R, RGN_OR);
    XDestroyRegion(R);
#elif (defined(__APPLE__) && !USE_X11)
    Region R = NewRgn(); 
    SetRectRgn(R, X, Y, X+W, Y+H);
    UnionRgn(R, region, region);
    DisposeRgn(R);
#else
    XRectangle R;
    R.x = X; R.y = Y; R.width = W; R.height = H;
    XUnionRectWithRegion(&R, region, region);
#endif
  }
  // make flush() search for this window:
  fltk::damage(1); // make flush() do something
}

void Window::flush() {
  make_current();
  unsigned char d = damage();
  if (d & ~DAMAGE_EXPOSE) {
    set_damage(d & ~DAMAGE_EXPOSE);
    draw();
  }
  if (i->region && !(d & DAMAGE_ALL)) {
    clip_region(i->region); i->region = 0;
    set_damage(DAMAGE_EXPOSE); draw();
    clip_region(0);
  }
}

////////////////////////////////////////////////////////////////

void Window::destroy() {
  CreatedWindow* x = i;
  if (!x) return;
  i = 0;

  // remove from the list of windows:
  CreatedWindow** pp = &CreatedWindow::first;
  for (; *pp != x; pp = &(*pp)->next) if (!*pp) return;
  *pp = x->next;

  // recursively remove any subwindows:
  for (CreatedWindow *x1 = CreatedWindow::first; x1;) {
    Window* subwindow = x1->window;
    if (subwindow->window() == this || subwindow->child_of() == this) {
      bool v = subwindow->visible();
      subwindow->destroy();
      if (v) subwindow->set_visible();
      x1 = CreatedWindow::first;
    } else x1 = x1->next;
  }

  // Make sure no events are sent to this window:
  if (flags()&MODAL) modal(0, false);
  throw_focus();
  clear_visible();

  x->free_gc();
  if (x->region) XDestroyRegion(x->region);
  XDestroyWindow(xdisplay, x->xid);
  delete x;
#ifdef _WIN32
  keep_app_active();
#endif
}

Window::~Window() {
  destroy();
}

//
// End of "$Id: Fl_Window.cxx,v 1.104 2003/10/28 17:45:15 spitzak Exp $".
//
