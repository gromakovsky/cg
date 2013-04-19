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

   segment_intersects_segment_viewer() {
      segments.push_back(segment_2f(point_2f(0, 0), point_2f(100, 100))); 
      segments.push_back(segment_2f(point_2f(50, 0), point_2f(50, 50)));
   }

   void draw(cg::visualization::drawer_type & drawer) const {
      drawer.set_color(cg::has_intersection(segments[0], segments[1]) ? Qt::green : Qt::white);

      drawer.draw_line(segments[0]);
      drawer.draw_line(segments[1]);
      if (idx) {
         drawer.set_color((rbutton_pressed_) ? Qt::red : Qt::yellow);
         drawer.draw_point(segments[idx->first][idx->second], 5);
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
      return set_idx(p);
   }

   bool on_release(const point_2f & p) {
      rbutton_pressed_ = false;
      return false;
   }

   bool on_move(const point_2f & p) {
      if (!rbutton_pressed_) {
         set_idx(p);
      } else {
         if (idx) {
            idx_to_point(*idx) = p;
         }
      }
      return true;
   }

private:
   typedef std::pair<size_t, size_t> idx_type;
   bool set_idx(const point_2f & p) {
      idx.reset();
      float max_r;
      for (size_t i = 0; i != 2; ++i) {
         for (size_t j = 0; j != 2; ++j) {
            point_2 current = idx_to_point(idx_type(i, j));
            float current_r = (p.x - current.x) * (p.x - current.x) + (p.y - current.y) * (p.y - current.y);
            if ((idx && current_r < max_r) || (!idx && current_r < 100)) {
               idx = idx_type(i, j);
               max_r = current_r;
            }
         }
      }
      return idx;
   }
   
   inline point_2f & idx_to_point(idx_type idx) {
      return segments[idx.first][idx.second];
   }
   
   std::vector<segment_2f> segments;
   boost::optional<idx_type> idx;
   bool rbutton_pressed_;

};

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   segment_intersects_segment_viewer viewer;
   cg::visualization::run_viewer(&viewer, "Segment intersects segment");
}

