#include <vector>
#include <stack>

#include <QColor>
#include <QApplication>

#include <boost/optional.hpp>

#include "cg/visualization/viewer_adapter.h"
#include "cg/visualization/draw_util.h"

#include "cg/primitives/segment.h"
#include "cg/operations/has_intersection/segment_segment.h"

#include "cg/io/point.h"

using cg::point_2;
using cg::point_2f;
using cg::segment_2f;

struct visibility_graph_viewer : cg::visualization::viewer_adapter
{
   visibility_graph_viewer() : moved(false)
   {
   }

   void draw(cg::visualization::drawer_type & drawer) const
   {
      drawer.set_color(Qt::white);

      for (auto it = points_.cbegin(); it != points_.cend(); ++it)
      {
         drawer.draw_point(*it);
      }

      drawer.set_color(Qt::red);

      for (auto it = obstacles.cbegin(); it != obstacles.cend(); ++it)
      {
         drawer.draw_line(*it);
      }

      drawer.set_color(Qt::blue);

      for (size_t i = 0; i != points_.size(); ++i)
      {
         for (size_t j = 0; j != points_.size(); ++j)
         {
            if (graph[i][j])
            {
               drawer.draw_line(segment_2f(points_[i], points_[j]));
            }
         }
      }
   }

   void print(cg::visualization::printer_type & p) const
   {
      p.corner_stream() << "press mouse rbutton to add vertex" << cg::visualization::endl
                        << "press and move mouse rbutton to add segment" << cg::visualization::endl
                        << "double click clears the screen" << cg::visualization::endl;
   }

   bool on_double_click(const point_2 & p)
   {
      points_.clear();
      obstacles.clear();
      return true;
   }

   void update()
   {
      size_t n = points_.size();
      size_t m = obstacles.size();
      graph.assign(n, std::vector<bool>(n, true));

      for (size_t i = 0; i != n; ++i)
      {
         for (size_t j = 0; j != n; ++j)
         {
            if (i == j)
            {
               graph[i][j] = false;
               continue;
            }

            for (size_t k = 0; k != m; ++k)
            {
               if (cg::has_intersection(obstacles[k], segment_2f(points_[i], points_[j])))
               {
                  graph[i][j] = false;
               }
            }
         }
      }
   }

   bool on_key(int key)
   {
      /*      switch (key)
            {
            case Qt::Key_Plus :
               eps *= 1.5;
               break;

            case Qt::Key_Minus :
               eps /= 1.5;
               break;

            case Qt::Key_Q :
               simplified ^= 1;
               break;

            default :
               return false;
            }

            update();
            return true;*/
      return false;
   }

   bool on_press(const point_2f & p)
   {
      moved = false;
      clicked = p;
      return true;
   }

   bool on_release(const point_2f & p)
   {
      if (!moved)
      {
         points_.push_back(clicked);
      }
      else
      {
         obstacles.push_back(segment_2f(clicked, p));
      }

      update();

      moved = false;
      return true;
   }

   bool on_move(const point_2f & p)
   {
      moved = true;
      return true;
   }

private:
   std::vector<point_2> points_;
   std::vector<segment_2f> obstacles;
   bool moved;
   point_2 clicked;
   std::vector< std::vector<bool> > graph;
};

int main(int argc, char ** argv)
{
   QApplication app(argc, argv);
   visibility_graph_viewer viewer;
   cg::visualization::run_viewer(&viewer, "visibility_graph viewer");
}