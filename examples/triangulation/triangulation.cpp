#include <vector>
#include <fstream>
#include <iostream>

#include <boost/optional.hpp>

#include <QColor>
#include <QApplication>

#include "cg/visualization/viewer_adapter.h"
#include "cg/visualization/draw_util.h"

#include "cg/triangulation/delaunay_triangulation.h"

#include "cg/io/point.h"

using cg::point_2;
using cg::point_2f;
using cg::vector_2f;
using cg::triangle_2;
using cg::segment_2;

struct triangulation_viewer : cg::visualization::viewer_adapter
{
   triangulation_viewer() : edit(true)
   {
   }

   // functions to draw circles are from https://github.com/BorisMinaev/cg
   double area(triangle_2 const & tr) const
   {
      double res = 0;

      for (int i = 0; i < 3; i++)
      {
         double dx = tr[i].x - tr[(i+1)%3].x;
         double dy = tr[i].y + tr[(i+1)%3].y;
         res += dx*dy;
      }

      return fabs(res) / 2.;
   }

   double distance(point_2 const & p1, point_2 const & p2) const
   {
      double dx = p1.x - p2.x;
      double dy = p1.y - p2.y;
      return sqrt(dx*dx+dy*dy);
   }

   point_2 intersect(segment_2 const & s1, segment_2 const & s2) const
   {
      double a1 = s1[0].y - s1[1].y;
      double b1 = -s1[0].x + s1[1].x;
      double c1 = -(a1 * s1[0].x + b1 * s1[0].y);
      double a2 = s2[0].y - s2[1].y;
      double b2 = -s2[0].x + s2[1].x;
      double c2 = -(a2 * s2[0].x + b2 * s2[0].y);
      double zn = a1*b2-a2*b1;
      double x = (c2*b1-c1*b2) / zn;
      double y = (c1*a2-c2*a1) / zn;
      return point_2(x, y);
   }

   void draw_circle2(cg::visualization::drawer_type & drawer, point_2 const & c, double r) const
   {
      double PI = asin(1) * 2;
      const int MAX_ITER = 360;

      for (int it = 0; it< MAX_ITER; it++)
      {
         double angle1 = PI *2 *it / MAX_ITER;
         double angle2 = PI *2 * (it+1) / MAX_ITER;
         double x1 = c.x + r * cos(angle1);
         double x2 = c.x + r * cos(angle2);
         double y1 = c.y + r * sin(angle1);
         double y2 = c.y + r * sin(angle2);
         drawer.draw_line(point_2(x1, y1), point_2(x2, y2));
      }
   }

   void draw_circle(cg::visualization::drawer_type & drawer, triangle_2 const & tr) const
   {
      double s = area(tr);
      segment_2 s1, s2;
      {
         point_2 ab((tr[0].x+tr[1].x)/2, (tr[0].y+tr[1].y)/2);
         double dx = ab.x - tr[0].x;
         double dy = ab.y - tr[0].y;
         point_2 p2(ab.x + dy, ab.y - dx);
         s1 = segment_2(ab, p2);
      }
      {
         point_2 ac((tr[0].x+tr[2].x)/2, (tr[0].y+tr[2].y)/2);
         double dx = ac.x - tr[0].x;
         double dy = ac.y - tr[0].y;
         point_2 p2(ac.x + dy, ac.y - dx);
         s2 = segment_2(ac, p2);
      }
      point_2 center = intersect(s1, s2);
      double a = distance(tr[0], tr[1]);
      double b = distance(tr[1], tr[2]);
      double c = distance(tr[2], tr[0]);
      double r = a *b * c / 4 / s;
      draw_circle2(drawer, center, r);
   }


   void draw(cg::visualization::drawer_type & drawer) const
   {
      drawer.set_color(Qt::white);

      for (auto p : points)
      {
         drawer.draw_point(p, 5);
      }

      drawer.set_color(Qt::green);

      for (auto triangle : triangulation)
      {
         for (int i = 0; i < 3; i++)
         {
            drawer.draw_line(triangle[i], triangle[(i + 1) % 3]);
         }
      }

      if (!edit)
      {
         if (triangle_to_highlight)
         {
            drawer.set_color(Qt::red);
            draw_circle(drawer, *triangle_to_highlight);

            for (int i = 0; i < 3; i++)
            {
               drawer.draw_line((*triangle_to_highlight)[i], (*triangle_to_highlight)[(i + 1) % 3]);
            }
         }
      }
   }

   void print(cg::visualization::printer_type & p) const
   {
      p.corner_stream() << "Double-click to clear." << cg::visualization::endl
                        << "There are two modes: show and edit" << cg::visualization::endl
                        << "Press mouse rbutton to insert a point in edit mode" << cg::visualization::endl
                        << "Press somewhere to localize in show mode" << cg::visualization::endl
                        << "Press q to switch between modes" << cg::visualization::endl
                        << "Current mode: " << (edit ? "edit" : "show") << cg::visualization::endl
                        << "Press t to write current test in file" << cg::visualization::endl;

      for (size_t i = 0; i < points.size(); ++i)
      {
         p.global_stream((point_2f)points[i] + vector_2f(5, 0)) << i;
      }
   }

   bool on_double_click(const point_2f & p)
   {
      points.clear();
      set.clear();
      edit = true;
      triangle_to_highlight.reset();
      triangulation.clear();
      return true;
   }

   bool on_press(const point_2f & p)
   {
      if (edit)
      {
         points.push_back(p);
         set.insert(p);
         triangulation = set.get_triangulation();
      }
      else
      {
         triangle_to_highlight = set.localize(p);
      }

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

   bool on_key(int key)
   {
      if (key == Qt::Key_Q)
      {
         edit = !edit;
         triangle_to_highlight.reset();
      }
      else if (key == Qt::Key_T)
      {
         std::ofstream out("tests/" + std::to_string(current_test++));
         out << "Points size = " << points.size() << std::endl;
         out << "Set size = " << set.size() << std::endl;
         out << "Points:" << std::endl;

         for (auto point : points)
         {
            out << point.x << " " << point.y << std::endl;
         }

         out << "Triangles:" << std::endl;

         for (auto triangle : triangulation)
         {
            out << triangle[0] << " " << triangle[1] << " " << triangle[2] << std::endl;
         }
      }
      else
      {
         return false;
      }

      return true;
   }


private:
   std::vector<cg::point_2> points;
   std::vector<triangle_2> triangulation;
   boost::optional<triangle_2> triangle_to_highlight;
   int current_test = 0;
   bool edit; // true if in edit mode
   cg::triangulatable_points_set_2 set;
};

int main(int argc, char ** argv)
{
   QApplication app(argc, argv);
   triangulation_viewer viewer;
   cg::visualization::run_viewer(&viewer, "triangulation viewer");
}
