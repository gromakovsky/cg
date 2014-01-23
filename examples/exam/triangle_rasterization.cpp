#include <QColor>
#include <QApplication>

#include <boost/optional.hpp>

#include <cg/visualization/viewer_adapter.h>
#include <cg/visualization/draw_util.h>

#include <cg/io/point.h>

#include <cg/primitives/point.h>
#include <cg/primitives/triangle.h>
#include <cg/exam/triangle_rasterization.h>

using cg::point_2f;
using cg::point_2;

struct triangle_rasterization_viewer : cg::visualization::viewer_adapter
{
   triangle_rasterization_viewer()
      : n(0)
   {
   }

   void draw(cg::visualization::drawer_type & drawer) const
   {
      drawer.set_color(Qt::white);

      for (size_t i = 0; i != n; ++i)
      {
         drawer.draw_point(points[i], 5);
      }

      if (n < 3)
      {
         return;
      }

      for (size_t i = 0; i != n; ++i)
      {
         drawer.draw_line(points[i], points[(i + 1) % 3]);
      }

      drawer.set_color(Qt::red);

      auto result = cg::rasterize(cg::triangle_2f(points[0], points[1], points[2]));

      for (auto p : result)
      {
         drawer.draw_line({p.first, p.second}, {p.first + 1, p.second});
         drawer.draw_line({p.first, p.second}, {p.first, p.second + 1});
         drawer.draw_line({p.first + 1, p.second + 1}, {p.first + 1, p.second});
         drawer.draw_line({p.first + 1, p.second + 1}, {p.first, p.second + 1});
      }

   }

   void print(cg::visualization::printer_type & p) const
   {
      p.corner_stream() << "press mouse rbutton to add point" << cg::visualization::endl
                        << "double click clears the screen" << cg::visualization::endl;
   }

   bool on_double_click(const point_2f & p)
   {
      n = 0;
      return true;
   }

   bool on_release(const point_2f & p)
   {
      if (n < 3)
      {
         points[n++] = p;

         if (n == 3)
         {
            std::cerr << points[0] << std::endl << points[1] << std::endl << points[2] << std::endl;
         }

         return true;
      }
      else
      {
         return false;
      }
   }

   bool on_key(int key)
   {
      return false;
   }

private:
   std::array< point_2f, 3> points;
   size_t n;

};

int main(int argc, char ** argv)
{
   QApplication app(argc, argv);
   triangle_rasterization_viewer viewer;
   cg::visualization::run_viewer(&viewer, "Triangle rasterization");
}
