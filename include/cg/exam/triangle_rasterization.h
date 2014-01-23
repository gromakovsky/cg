#pragma once

#include <cg/primitives/point.h>
#include <cg/primitives/segment.h>
#include <cg/primitives/triangle.h>
#include <cg/operations/intersection/segment_segment.h>

#include <cmath>

#include <vector>
#include <algorithm>

namespace cg
{

   template <class Scalar>
   class triangle_rasterization_iterator : public std::iterator< std::input_iterator_tag, std::pair<Scalar, Scalar> >
   {
      std::vector< point_2t<Scalar> > points;
      Scalar x0, x1;

      Scalar x, y;
      Scalar x_left, x_right;

      bool valid;

      std::pair<Scalar, Scalar> get_bounds(Scalar y)
      {
         if (y < points[1].y)
         {
            std::vector< point_2t<Scalar> > intersections;
            Scalar bottom = std::max(y, points[0].y);
            Scalar top = std::min(y + 1, points[1].y);
            segment_2t<Scalar> bottom_line = {{x0, bottom}, {x1, bottom}};
            segment_2t<Scalar> top_line = {{x0, top}, {x1, top}};
            segment_2t<Scalar> s1(points[0], points[1]);
            segment_2t<Scalar> s2(points[0], points[2]);
            intersections.push_back(boost::get< point_2t<Scalar> >(intersection(s1, bottom_line)));
            intersections.push_back(boost::get< point_2t<Scalar> >(intersection(s2, bottom_line)));
            intersections.push_back(boost::get< point_2t<Scalar> >(intersection(s1, top_line)));
            intersections.push_back(boost::get< point_2t<Scalar> >(intersection(s2, top_line)));

            return {floor(std::min_element(intersections.begin(), intersections.end())->x), ceil(std::max_element(intersections.begin(), intersections.end())->x)};
         }
         else
         {
            std::vector< point_2t<Scalar> > intersections;
            Scalar bottom = std::max(y, points[1].y);
            Scalar top = std::min(y + 1, points[2].y);
            segment_2t<Scalar> bottom_line = {{x0, bottom}, {x1, bottom}};
            segment_2t<Scalar> top_line = {{x0, top}, {x1, top}};
            segment_2t<Scalar> s1(points[2], points[0]);
            segment_2t<Scalar> s2(points[2], points[1]);
            intersections.push_back(boost::get< point_2t<Scalar> >(intersection(s1, bottom_line)));
            intersections.push_back(boost::get< point_2t<Scalar> >(intersection(s2, bottom_line)));
            intersections.push_back(boost::get< point_2t<Scalar> >(intersection(s1, top_line)));
            intersections.push_back(boost::get< point_2t<Scalar> >(intersection(s2, top_line)));

            return {floor(std::min_element(intersections.begin(), intersections.end())->x), ceil(std::max_element(intersections.begin(), intersections.end())->x)};
         }
      }

      void increment_y()
      {
         ++y;
         auto bounds = get_bounds(y);
         x_left = bounds.first;
         x_right = bounds.second;
         x = x_left;
      }

   public:

      triangle_rasterization_iterator() : valid(false)
      {
      }

      triangle_rasterization_iterator(triangle_2t<Scalar> t) : valid(true)
      {
         points = {t[0], t[1], t[2]};
         std::sort(points.begin(), points.end(),
                   [](const point_2t<Scalar> & p1, const point_2t<Scalar> & p2)
         {
            return p1.y < p2.y;
         }
                  );

         x0 = floor(std::min_element(points.begin(), points.end())->x);
         x1 = ceil(std::max_element(points.begin(), points.end())->x);

         y = floor(points[0].y);
         auto bounds = get_bounds(y);
         x_left = bounds.first;
         x_right = bounds.second;
         x = x_left;
      }

      bool operator==(const triangle_rasterization_iterator & rhs)
      {
         if (!valid && !rhs.valid)
         {
            return true;
         }

         if (valid != rhs.valid)
         {
            return false;
         }

         return points == rhs.points && x == rhs.x && y == rhs.y;
      }

      bool operator!=(const triangle_rasterization_iterator & rhs)
      {
         return !(*this == rhs);
      }

      std::pair<Scalar, Scalar> operator*()
      {
         return {x, y};
      }

      triangle_rasterization_iterator & operator++()
      {
         ++x;

         if (x == x_right)
         {
            if (y + 1 >= points[2].y)
            {
               valid = false;
            }
            else
            {
               increment_y();
            }
         }

         return *this;
      }

      triangle_rasterization_iterator operator++(int)
      {
         triangle_rasterization_iterator tmp(*this);
         operator++();
         return tmp;
      }

   };

   template <class Scalar>
   triangle_rasterization_iterator<Scalar> rasterization_iterator(triangle_2t<Scalar> t)
   {
      return triangle_rasterization_iterator<Scalar>(t);
   }

   template <class Scalar>
   std::vector< std::pair<Scalar, Scalar> > rasterize(triangle_2t<Scalar> t)
   {
      std::vector< std::pair<Scalar, Scalar> > ans;

      auto it = rasterization_iterator(t);
      std::copy(it, triangle_rasterization_iterator<Scalar>(), std::back_inserter(ans));

      return ans;

      /*std::vector< point_2t<Scalar> > points({t[0], t[1], t[2]});
      std::sort(points.begin(), points.end(),
                [](const point_2t<Scalar> & p1, const point_2t<Scalar> & p2)
      {
         return p1.y < p2.y;
      }
               );

      Scalar x0 = floor(std::min_element(points.begin(), points.end())->x);
      Scalar x1 = ceil(std::max_element(points.begin(), points.end())->x);

      for (Scalar y = floor(points[0].y); y < points[1].y; ++y)
      {
         std::vector< point_2t<Scalar> > intersections;
         Scalar bottom = std::max(y, points[0].y);
         Scalar top = std::min(y + 1, points[1].y);
         segment_2t<Scalar> bottom_line = {{x0, bottom}, {x1, bottom}};
         segment_2t<Scalar> top_line = {{x0, top}, {x1, top}};
         segment_2t<Scalar> s1(points[0], points[1]);
         segment_2t<Scalar> s2(points[0], points[2]);
         intersections.push_back(boost::get< point_2t<Scalar> >(intersection(s1, bottom_line)));
         intersections.push_back(boost::get< point_2t<Scalar> >(intersection(s2, bottom_line)));
         intersections.push_back(boost::get< point_2t<Scalar> >(intersection(s1, top_line)));
         intersections.push_back(boost::get< point_2t<Scalar> >(intersection(s2, top_line)));

         for (Scalar x = floor(std::min_element(intersections.begin(), intersections.end())->x); x < ceil(std::max_element(intersections.begin(), intersections.end())->x); ++x)
         {
            ans.push_back({x, y});
         }
      }

      for (Scalar y = floor(points[1].y); y < points[2].y; ++y)
      {
         std::vector< point_2t<Scalar> > intersections;
         Scalar bottom = std::max(y, points[1].y);
         Scalar top = std::min(y + 1, points[2].y);
         segment_2t<Scalar> bottom_line = {{x0, bottom}, {x1, bottom}};
         segment_2t<Scalar> top_line = {{x0, top}, {x1, top}};
         segment_2t<Scalar> s1(points[2], points[0]);
         segment_2t<Scalar> s2(points[2], points[1]);
         intersections.push_back(boost::get< point_2t<Scalar> >(intersection(s1, bottom_line)));
         intersections.push_back(boost::get< point_2t<Scalar> >(intersection(s2, bottom_line)));
         intersections.push_back(boost::get< point_2t<Scalar> >(intersection(s1, top_line)));
         intersections.push_back(boost::get< point_2t<Scalar> >(intersection(s2, top_line)));

         for (Scalar x = floor(std::min_element(intersections.begin(), intersections.end())->x); x < ceil(std::max_element(intersections.begin(), intersections.end())->x); ++x)
         {
            ans.push_back({x, y});
         }
      }

      return ans;*/

   }
}