#include <QColor>
#include <QApplication>

#include <boost/optional.hpp>

#include <cg/visualization/viewer_adapter.h>
#include <cg/visualization/draw_util.h>

#include <cg/io/point.h>

#include <cg/primitives/point.h>
#include <cg/primitives/contour.h>
#include <cg/exam/polygons_distance.h>

#include <cg/operations/compare_dist.h>

using cg::point_2f;
using cg::point_2;

struct polygons_distance_viewer : cg::visualization::viewer_adapter
{
   polygons_distance_viewer()
      : entered(0)
   {
      std::cerr << "C1:" << std::endl;
      /*entered = 2;
      c1.add_point({-1, 2});
      c1.add_point({0, 1});
      c1.add_point({1, 2});
      c1.add_point({0, 3});

      c2.add_point({2, 1});
      c2.add_point({3, 1});
      c2.add_point({2, 4});*/
   }

   void draw(cg::visualization::drawer_type & drawer) const
   {
      drawer.set_color(Qt::white);

      for (size_t i = 1; i < c1.size(); ++i)
      {
         drawer.draw_line(c1[i - 1], c1[i]);
      }

      if (c1.size() > 2)
      {
         drawer.draw_line(c1[c1.size() - 1], c1[0]);
      }

      for (size_t i = 1; i < c2.size(); ++i)
      {
         drawer.draw_line(c2[i - 1], c2[i]);
      }

      if (c2.size() > 2)
      {
         drawer.draw_line(c2[c2.size() - 1], c2[0]);
      }

      if (entered < 2)
      {
         return;
      }

      auto ans = cg::distance(c1, c2);

      for (size_t i = 0; i != c1.size(); ++i)
      {
         for (size_t j = 0; j != c2.size(); ++j)
         {
            if (cg::compare_dist(c1[i], c2[j], ans.first, ans.second))
            {
               drawer.set_color(Qt::blue);
               drawer.draw_line(ans.first, ans.second);
            }
         }
      }

      drawer.set_color(Qt::red);

      drawer.draw_line(ans.first, ans.second);

   }

   void print(cg::visualization::printer_type & p) const
   {
      p.corner_stream() << "press mouse rbutton to add point" << cg::visualization::endl
                        << "press N to finish polygon" << cg::visualization::endl
                        << "double click clears the screen" << cg::visualization::endl;
   }

   bool on_double_click(const point_2f & p)
   {
      entered = 0;
      c1.clear();
      c2.clear();
      return true;
   }

   bool on_release(const point_2f & p)
   {
      if (entered == 0)
      {
         c1.add_point(p);
         std::cerr << p << std::endl;
         return true;
      }
      else if (entered == 1)
      {
         c2.add_point(p);
         std::cerr << p << std::endl;
         return true;
      }

      return false;
   }

   bool on_key(int key)
   {
      if (key == Qt::Key_N)
      {
         ++entered;
         std::cerr << "C2:" << std::endl;
         return true;
      }

      return false;
   }

private:
   size_t entered;
   cg::contour_2f c1, c2;

};

int main(int argc, char ** argv)
{
   QApplication app(argc, argv);
   polygons_distance_viewer viewer;
   cg::visualization::run_viewer(&viewer, "Polygons distance");
}

