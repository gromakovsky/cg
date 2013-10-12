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
   visibility_graph_viewer() : polygons(1)
   {
   }

   void draw(cg::visualization::drawer_type & drawer) const
   {
      drawer.set_color(Qt::white);

      for (auto it = polygons.cbegin(); it != polygons.cend(); ++it)
      {
         if (it->size() == 0)
         {
            continue;
         }

         if (it->size() == 1)
         {
            drawer.draw_point(it->front());
            continue;
         }

         auto pol = *it;

         for (size_t lp = pol.size() - 1, l = 0; l != pol.size(); lp = l++)
         {
            drawer.draw_point(pol[l], 3);
            drawer.draw_line(pol[lp], pol[l]);
         }

      }

      drawer.set_color(Qt::blue);

      for (auto it = edges.cbegin(); it != edges.cend(); ++it)
      {
         drawer.draw_line(it->first, it->second);
      }

   }

   void print(cg::visualization::printer_type & p) const
   {
      p.corner_stream() << "press N to start new polygon" << cg::visualization::endl
                        << "press mouse rbutton to add vertex" << cg::visualization::endl
                        << "double click clears the screen" << cg::visualization::endl;
   }

   bool on_double_click(const point_2 & p)
   {
      polygons.clear();
      return true;
   }

   bool test_visibility(const point_2 & a, const point_2 & b)
   {
      for (auto it = polygons.cbegin(); it != polygons.cend(); ++it)
      {

         if (it->size() <= 1)
         {
            continue;
         }

         auto pol = *it;

         for (size_t lp = pol.size() - 1, l = 0; l != pol.size(); lp = l++)
         {
            if (pol[lp] == a || pol[l] == a || pol[lp] == b || pol[l] == b)
            {
               continue;
            }

            if (cg::has_intersection(segment_2f(pol[lp], pol[l]), segment_2f(a, b)))
            {
               return false;
            }
         }
      }

      return true;
   }

   void update()
   {
      size_t n = polygons.size();
      edges.clear();

      for (size_t i = 0; i != n; ++i)
      {
         for (size_t j = 0; j != n; ++j)
         {
            if (i == j)
            {
               continue;
            }

            auto pol1 = polygons[i];
            auto pol2 = polygons[j];

            for (size_t a = 0; a != pol1.size(); ++a)
            {
               for (size_t b = 0; b != pol2.size(); ++b)
               {

                  if (test_visibility(pol1[a], pol2[b]))
                  {
                     edges.push_back(std::make_pair(pol1[a], pol2[b]));
                  }
               }
            }

         }
      }
   }

   bool on_key(int key)
   {
      switch (key)
      {
      case Qt::Key_N :
         polygons.push_back(std::vector<point_2>());
         break;

      default :
         return false;
      }

      return true;
   }

   bool on_press(const point_2f & p)
   {
      bool new_obs = false;
      point_2 last;

      if (polygons.back().size() != 0)
      {
         new_obs = true;
         last = polygons.back().back();
      }

      polygons.back().push_back(p);

      if (new_obs)
      {
         obstacles.push_back(segment_2f(last, p));
      }

      update();
      return true;
   }

   bool on_release(const point_2f & p)
   {
      return false;
   }

   bool on_move(const point_2f & p)
   {
      return false;
   }

private:
   std::vector< std::vector<point_2> > polygons;
   std::vector<segment_2f> obstacles;
   std::vector< std::pair<point_2, point_2> > edges;
};

int main(int argc, char ** argv)
{
   QApplication app(argc, argv);
   visibility_graph_viewer viewer;
   cg::visualization::run_viewer(&viewer, "visibility_graph viewer");
}