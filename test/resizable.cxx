// generated by Fast Light User Interface Designer (fluid) version 2.0002

#include "resizable.h"

#include <fltk/run.h>

int main (int argc, char **argv) {

  fltk::Window* w;
   {fltk::Window* o = new fltk::Window(560, 470);
    w = o;
    o->begin();
     {fltk::MenuBar* o = new fltk::MenuBar(515, 0, 45, 25);
      o->tooltip("A right-justified help item can be made by making a second menubar and puttin\
g it to the right of the resizable().");
      o->begin();
       {fltk::ItemGroup* o = new fltk::ItemGroup("Help");
        o->begin();
        new fltk::Item("About...");
        new fltk::Item("Chapter 1");
        o->end();
      }
      o->end();
    }
     {fltk::MenuBar* o = new fltk::MenuBar(0, 0, 515, 25);
      o->begin();
      new fltk::Item("This");
      new fltk::Item("is");
      new fltk::Item("a");
      new fltk::Item("Menu");
      new fltk::Item("Bar");
      o->end();
    }
     {fltk::InvisibleBox* o = new fltk::InvisibleBox(95, 25, 465, 419, "\nThis program demonstrates how you set the resizable() of a window to get th\
ings to resize the way you want.");
      o->box(fltk::FLAT_BOX);
      o->color((fltk::Color)19);
      o->labelcolor((fltk::Color)7);
      o->align(fltk::ALIGN_TOP|fltk::ALIGN_INSIDE|fltk::ALIGN_CLIP|fltk::ALIGN_WRAP);
    }
     {fltk::Button* o = new fltk::Button(0, 25, 95, 25, "These");
      o->color((fltk::Color)47);
    }
     {fltk::Button* o = new fltk::Button(0, 50, 95, 25, "are some");
      o->color((fltk::Color)47);
    }
     {fltk::Button* o = new fltk::Button(0, 75, 95, 25, "buttons");
      o->color((fltk::Color)47);
    }
     {fltk::Button* o = new fltk::Button(0, 100, 95, 25, "we don't want");
      o->color((fltk::Color)47);
    }
     {fltk::Button* o = new fltk::Button(0, 125, 95, 25, "to resize.");
      o->color((fltk::Color)47);
    }
     {fltk::Button* o = new fltk::Button(0, 150, 95, 25, "Things that");
      o->color((fltk::Color)0x8e6f8c00);
    }
     {fltk::Button* o = new fltk::Button(0, 175, 95, 25, "overlap the box");
      o->color((fltk::Color)0x8e6f8c00);
    }
     {fltk::Button* o = new fltk::Button(0, 200, 95, 25, "will scale.");
      o->color((fltk::Color)0x8e6f8c00);
    }
     {fltk::Group* o = new fltk::Group(0, 225, 95, 190, "A group");
      o->box(fltk::ENGRAVED_BOX);
      o->labeltype(fltk::ENGRAVED_LABEL);
      o->labelcolor((fltk::Color)49);
      o->align(fltk::ALIGN_BOTTOM|fltk::ALIGN_INSIDE);
      o->tooltip("This group has resizable(0) (this is the default).  But you can set a group's\
 resizable to nest different resize behavior.");
      o->begin();
       {fltk::Button* o = new fltk::Button(5, 5, 85, 25, "You can put");
        o->color((fltk::Color)47);
      }
       {fltk::Button* o = new fltk::Button(5, 30, 85, 25, "them inside");
        o->color((fltk::Color)47);
      }
       {fltk::Button* o = new fltk::Button(5, 55, 85, 25, "a group to");
        o->color((fltk::Color)47);
      }
       {fltk::Button* o = new fltk::Button(5, 80, 85, 25, "stop scaling.");
        o->color((fltk::Color)47);
      }
      o->end();
    }
     {fltk::Button* o = new fltk::Button(0, 419, 95, 25, "nonscaling");
      o->color((fltk::Color)47);
    }
     {fltk::InvisibleBox* o = new fltk::InvisibleBox(0, 444, 460, 25, "This status message will resize horizontally only.");
      o->box(fltk::BORDER_BOX);
      o->color((fltk::Color)55);
      o->align(fltk::ALIGN_LEFT|fltk::ALIGN_INSIDE);
    }
    new fltk::Button(460, 445, 25, 25, "@<<");
    new fltk::Button(485, 445, 25, 25, "@<");
    new fltk::Button(510, 445, 25, 25, "@>");
    new fltk::Button(535, 445, 25, 25, "@>>");
     {fltk::InvisibleBox* o = new fltk::InvisibleBox(95, 150, 365, 270, "This is the resizable() of the window.\nIt is normally invisible.");
      o->box(fltk::BORDER_BOX);
      o->color((fltk::Color)37);
      o->labelcolor((fltk::Color)1);
      o->align(fltk::ALIGN_CLIP|fltk::ALIGN_WRAP);
      fltk::Group::current()->resizable(o);
      o->tooltip("To make it invisible, use an Fl_Box with the default box(FL_NO_BOX) and no la\
bel.\n\nFltk sets the window's minimum size such that this widget's minimum si\
ze is 100, unless it starts out smaller than that.  You can override this defa\
ult by calling size_range() on the window.");
    }
    o->end();
  }
  // w->size_range(w->w()-w->resizable()->w(), w->h()-w->resizable()->h());
  w->show(argc, argv);
  return  fltk::run();
}
