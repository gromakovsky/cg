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
   std::vector< std::pair<Scalar, Scalar> > rasterize(triangle_2t<Scalar> t)
   {
      std::vector< std::pair<Scalar, Scalar> > ans;

      std::vector< point_2t<Scalar> > points({t[0], t[1], t[2]});
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

      return ans;

   }
}