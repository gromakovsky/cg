#include <QColor>
#include <QApplication>

#include <boost/optional.hpp>
#include <utility>

#include "cg/visualization/viewer_adapter.h"
#include "cg/visualization/draw_util.h"

#include "cg/io/point.h"

#include <cg/primitives/segment.h>
#include <cg/primitives/point.h>

#include <cg/operations/has_intersection/segment_segment.h>

using cg::point_2f;
using cg::point_2;
using cg::segment_2f;
using cg::segment_2;

struct segment_intersects_segment_viewer : cg::visualization::viewer_adapter {

   segment_intersects_segment_viewer()
   : segments(segment_2f(point_2f(0, 0), point_2f(50, 50)),
   segment_2f(point_2f(25, 0), point_2f(25, 25))) {
   }

   void draw(cg::visualization::drawer_type & drawer) const {
      drawer.set_color(cg::has_intersection(segments.first, segments.second) ? Qt::green : Qt::white);

      drawer.draw_line(segments.first);
      drawer.draw_line(segments.second);
      if (current_point) {
         drawer.set_color((rbutton_pressed_) ? Qt::red : Qt::yellow);
         drawer.draw_point(*current_point, 5);
      }
   }

   void print(cg::visualization::printer_type & p) const {
      p.corner_stream() << "Press mouse rbutton near segment vertex to move it"
              << cg::visualization::endl
              << "If segments are green they intersect"
              << cg::visualization::endl;
   }

   bool on_press(const point_2f & p) {
      rbutton_pressed_ = true;
      return set_current_point(p);
   }

   bool on_release(const point_2f & p) {
      rbutton_pressed_ = false;
      return false;
   }

   bool on_move(const point_2f & p) {
      if (!rbutton_pressed_) {
         set_current_point(p);
      } else {
         current_point = p;
      }
      return true;
   }

private:
   bool set_current_point(const point_2f & p) {
      current_point.reset();
      float max_r;
      for (size_t i = 0; i != 2; ++i) {
         for (size_t j = 0; j != 2; ++j) {
            point_2 current = (i == 1 ? segments.second[j] : segments.first[j]);
            float current_r = (p.x - current.x) * (p.x - current.x) + (p.y - current.y) * (p.y - current.y);
            if ((current_point && current_r < max_r) || (!current_point && current_r < 100)) {
               current_point = (i == 1 ? segments.second[i] : segments.first[i]);
               max_r = current_r;
            }
         }
      }
      return current_point;
   }
   
   std::pair<segment_2f, segment_2f> segments;
   boost::optional<cg::point_2f *> current_point;
   bool rbutton_pressed_;

};

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   segment_intersects_segment_viewer viewer;
   cg::visualization::run_viewer(&viewer, "Segment intersects segment");
}

