#include <vector>
#include <stack>

#include <QColor>
#include <QApplication>

#include <boost/optional.hpp>

#include "cg/visualization/viewer_adapter.h"
#include "cg/visualization/draw_util.h"

#include <cg/operations/simplify.h>

#include "cg/io/point.h"

#include <fstream>

using cg::point_2;
using cg::point_2f;

struct simplify_viewer : cg::visualization::viewer_adapter
{
   simplify_viewer() : simplified(false), eps(1.0)
   {
   }

   void draw(cg::visualization::drawer_type& drawer) const
   {
      drawer.set_color(Qt::white);

      const std::vector<cg::point_2>& drawing(simplified ? points_simplified : points_);

      if (drawing.size() < 2)
      {
         return;
      }

      for (size_t i = 0; i != drawing.size() - 1; ++i)
      {
         drawer.draw_line(drawing[i], drawing[i + 1]);
      }

   }

   void print(cg::visualization::printer_type& p) const
   {
      p.corner_stream() << "press mouse rbutton to add vertex" << cg::visualization::endl
                        << "double click clears the screen" << cg::visualization::endl
                        << "press q to change simplified state" << cg::visualization::endl
                        << "press + to increase epsilon" << cg::visualization::endl
                        << "press - to decrease epsilon" << cg::visualization::endl;

      p.corner_stream() << "eps: " << eps
                        << cg::visualization::endl;
      p.corner_stream() << "simplified: " << (simplified ? "true" : "false")
                        << cg::visualization::endl;
   }

   bool on_double_click(const point_2& p)
   {
      points_.clear();
      points_simplified.clear();
      simplified = false;
      return true;
   }

   void update()
   {
      if (points_.size() < 2)
      {
         return;
      }

      points_simplified.clear();
      cg::simplify(points_.begin(), points_.end(), std::back_inserter(points_simplified), eps);
   }

   bool on_key(int key)
   {
      switch (key)
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
      return true;
   }

   bool on_press(const point_2f& p)
   {
      points_.push_back(p);
      update();
      return true;
   }

   bool on_release(const point_2f& p)
   {
      return false;
   }

   bool on_move(const point_2f& p)
   {
      return false;
   }

private:
   std::vector<point_2> points_;
   std::vector<point_2> points_simplified;
   bool simplified;
   double eps;
};

int main(int argc, char ** argv)
{
   QApplication app(argc, argv);
   simplify_viewer viewer;
   cg::visualization::run_viewer(&viewer, "simplify viewer");
}
