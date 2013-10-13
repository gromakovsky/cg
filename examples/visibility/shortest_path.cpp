#include <vector>
#include <set>
#include <map>
#include <limits>

#include <QColor>
#include <QApplication>

#include "cg/visualization/viewer_adapter.h"
#include "cg/visualization/draw_util.h"

#include "cg/primitives/segment.h"
#include "cg/primitives/point.h"

#include "cg/operations/has_intersection/segment_segment.h"
#include "cg/operations/orientation.h"
#include "cg/operations/distance.h"

using cg::point_2;
using cg::point_2f;
using cg::segment_2f;

struct shortest_path_viewer : cg::visualization::viewer_adapter
{
   shortest_path_viewer() : polygons(1)
   {
   }

   void draw(cg::visualization::drawer_type & drawer) const
   {
      drawer.set_color(Qt::red);

      for (point_2 p : points)
      {
         drawer.draw_point(p, 6);
      }

      drawer.set_color(Qt::white);

      for (auto it = polygons.cbegin(); it != polygons.cend() - 1; ++it)
      {
         if (it->size() == 0)
         {
            continue;
         }

         if (it->size() == 1)
         {
            drawer.draw_point(it->front(), 3);
            continue;
         }

         auto pol = *it;

         for (size_t lp = pol.size() - 1, l = 0; l != pol.size(); lp = l++)
         {
            drawer.draw_point(pol[l], 3);
            drawer.draw_line(pol[lp], pol[l]);
         }
      }

      auto pol = polygons.back();
      drawer.set_color(Qt::green);

      for (size_t lp = pol.size() - 1, l = 0; l != pol.size(); lp = l++)
      {
         drawer.draw_point(pol[l], 3);
         drawer.draw_line(pol[lp], pol[l]);
      }

      drawer.set_color(Qt::blue);

      /*for (auto it = edges.cbegin(); it != edges.cend(); ++it)
      {
         drawer.draw_line(it->first, it->second);
      }*/

      if (path.empty())
      {
         return;
      }

      for (size_t lp = path.size() - 1, l = 0; l != path.size(); lp = l++)
      {
         drawer.draw_point(path[l], 3);
         drawer.draw_line(path[lp], path[l]);
      }

   }

   void print(cg::visualization::printer_type & p) const
   {
      if (points.size() == 0)
      {
         p.corner_stream() << "press mouse rbutton to add first vertex" << cg::visualization::endl;
      }

      if (points.size() == 1)
      {
         p.corner_stream() << "press mouse rbutton to add second vertex" << cg::visualization::endl;
      }

      if (points.size() == 2)
      {
         p.corner_stream() << "press N to start new polygon" << cg::visualization::endl
                           << "press mouse rbutton to add vertex to current polygon" << cg::visualization::endl;
      }

      p.corner_stream() << "double click clears the screen" << cg::visualization::endl;
      p.corner_stream() << "size: " << polygons.size() << cg::visualization::endl;
   }

   bool on_double_click(const point_2f & p)
   {
      polygons = std::vector< std::vector<point_2> >(1);
      edges.clear();
      points.clear();
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
            if (pol[lp] == a)
            {
               if (cg::orientation(pol[l], pol[lp], b) == cg::CG_COLLINEAR && cg::collinear_are_ordered_along_line(a, pol[l], b))
               {
                  return false;
               }
            }

            if (pol[l] == a)
            {
               if (cg::orientation(pol[l], pol[lp], b) == cg::CG_COLLINEAR && cg::collinear_are_ordered_along_line(a, pol[lp], b))
               {
                  return false;
               }
            }

            if (pol[lp] == b)
            {
               if (cg::orientation(pol[l], pol[lp], a) == cg::CG_COLLINEAR && cg::collinear_are_ordered_along_line(b, pol[l], a))
               {
                  return false;
               }
            }

            if (pol[l] == b)
            {
               if (cg::orientation(pol[l], pol[lp], a) == cg::CG_COLLINEAR && cg::collinear_are_ordered_along_line(b, pol[l], a))
               {
                  return false;
               }
            }

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

   void build_visibility_graph()
   {
      const size_t n = polygons.size();
      edges.clear();

      for (size_t i = 0; i != n; ++i)
      {
         for (size_t j = i + 1; j != n; ++j)
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

                  auto min_point = std::min(pol1[a], pol2[b]);
                  auto max_point = std::max(pol1[a], pol2[b]);

                  auto a_prev = (a == 0 ? pol1.back() : pol1[a - 1]);
                  auto a_next = (a == pol1.size() - 1 ? pol1.front() : pol1[a + 1]);

                  auto a_or1 = cg::orientation(pol2[b], pol1[a], a_prev);
                  auto a_or2 = cg::orientation(pol2[b], pol1[a], a_next);

                  if (cg::opposite(a_or1, a_or2))
                  {
                     continue;
                  }

                  auto b_prev = (b == 0 ? pol2.back() : pol2[b - 1]);
                  auto b_next = (b == pol2.size() - 1 ? pol2.front() : pol2[b + 1]);

                  auto b_or1 = cg::orientation(pol1[a], pol2[b], b_prev);
                  auto b_or2 = cg::orientation(pol1[a], pol2[b], b_next);

                  if (cg::opposite(b_or1, b_or2))
                  {
                     continue;
                  }

                  if (test_visibility(min_point, max_point))
                  {
                     edges.insert(std::make_pair(min_point, max_point));
                  }
               }
            }

         }
      }
   }

   void update()
   {
      path.clear();

      if (points.size() != 2)
      {
         return;
      }

      polygons.push_back(std::vector<point_2>());
      polygons.back().push_back(points.back());
      build_visibility_graph();

      auto max_double = std::numeric_limits<double>::max();

      std::map<point_2, size_t> number_by_point;
      std::vector<point_2> point_by_number;

      for (auto it = polygons.cbegin(); it != polygons.cend(); ++it)
      {
         for (auto iter = it->cbegin(); iter != it->cend(); ++iter)
         {
            number_by_point[*iter] = point_by_number.size();
            point_by_number.push_back(*iter);
         }
      }

      size_t n = point_by_number.size();

      std::vector< std::vector<double> > distances(n, std::vector<double>(n, max_double));

      for (size_t i = 0; i != n; ++i)
      {
         distances[i][i] = 0;
      }

      for (auto it = edges.cbegin(); it != edges.cend(); ++it)
      {
         distances[number_by_point[it->first]][number_by_point[it->second]] = cg::distance(it->first, it->second);
         distances[number_by_point[it->second]][number_by_point[it->first]] = cg::distance(it->first, it->second);
      }

      for (size_t i = 0; i != n; ++i)
      {
         for (size_t j = 0; j != n; ++j)
         {
            //std::cout << distances[i][j] << " ";
         }

         //std::cout << std::endl;
      }

      std::vector<double> d(n, max_double);
      d[0] = 0;
      std::vector<bool> u(n, false);
      std::vector<size_t> p(n, static_cast<size_t>(-1));

      for (size_t i = 0; i != n; ++i)
      {
         auto min = max_double;
         auto min_index = static_cast<size_t>(-1);

         for (size_t j = 0; j != n; ++j)
         {
            if (!u[j] && d[j] < min)
            {
               min = d[(min_index = j)];
            }
         }

         u[min_index] = true;

         for (size_t j = 0; j != n; ++j)
         {
            if (!u[j])
            {
               if (d[j] > d[min_index] + distances[j][min_index])
               {
                  d[j] = d[min_index] + distances[j][min_index];
                  p[j] = min_index;
               }
            }
         }

      }

      size_t v = n - 1;

      while (v != 0)
      {
         path.push_back(point_by_number[v]);
         v = p[v];
      }

      path.push_back(point_by_number[0]);

      polygons.erase(polygons.end() - 1);
   }

   bool on_key(int key)
   {
      if (points.size() != 2)
      {
         return false;
      }

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
      if (points.size() != 2)
      {
         points.push_back(p);

         if (points.size() == 1)
         {
            polygons.back().push_back(points.front());
            polygons.resize(2);
         }
      }
      else

      {
         polygons.back().push_back(p);
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
   std::set< std::pair<point_2, point_2> > edges;
   std::vector<point_2> points;
   std::vector<point_2> path;
};

int main(int argc, char ** argv)
{
   QApplication app(argc, argv);
   shortest_path_viewer viewer;
   cg::visualization::run_viewer(&viewer, "shortest_path viewer");
}
