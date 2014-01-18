#pragma once

#include <cg/primitives/point.h>
#include <cg/primitives/segment.h>
#include <cg/primitives/triangle.h>
#include <cg/operations/has_intersection/segment_segment.h>
#include <cg/operations/intersection/segment_segment.h>

#include <cmath>

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

namespace cg
{

   template <class Scalar>
   std::vector< std::pair<Scalar, Scalar> > rasterize(triangle_2t<Scalar> t)
   {
      Scalar x0 = floor(std::min(std::min(t[0].x, t[1].x), t[2].x));
      Scalar x1 = ceil(std::max(std::max(t[0].x, t[1].x), t[2].x));
      Scalar y0 = floor(std::min(std::min(t[0].y, t[1].y), t[2].y));
      Scalar y1 = ceil(std::max(std::max(t[0].y, t[1].y), t[2].y));

      std::vector< std::pair<Scalar, Scalar> > ans;

      for (Scalar y = y0; y <= y1; ++y)
      {
         segment_2t<Scalar> line({x0, y}, {x1, y});
         std::vector< boost::variant< boost::none_t, point_2t<Scalar>, segment_2t<Scalar> > > intersections;

         for (size_t i = 0; i != 3; ++i)
         {
            intersections.push_back(intersection(t.side(i), line));
         }

         std::set< point_2t<Scalar> > bounds;

         bool flag = false;

         for (size_t i = 0; i != 3; ++i)
         {
            if (intersections[i].which() == 1)
            {
               bounds.insert(boost::get< point_2t<Scalar> >(intersections[i]));
            }

            if (intersections[i].which() == 2)
            {
               if (orientation(t[i], {x0, y}, {x1, y}) == CG_LEFT)
               {
                  flag = true;
               }

            }
         }

         if (bounds.size() == 1)
         {
            ans.push_back({floor(bounds.begin()->x), y - 1});
            continue;
         }

         if (bounds.size() != 2 || flag)
         {
            continue;
         }

         std::vector< point_2t<Scalar> > b(bounds.begin(), bounds.end());

         Scalar l = floor(std::min(b[0].x, b[1].x));
         Scalar r = ceil(std::max(b[0].x, b[1].x));

         for (Scalar x = l; x != r; ++x)
         {
            ans.push_back({x, y - 1});
            ans.push_back({x, y});
         }
      }

      
      for (Scalar x = x0; x <= x1; ++x)
      {
         segment_2t<Scalar> line({x, y0}, {x, y1});
         std::vector< boost::variant< boost::none_t, point_2t<Scalar>, segment_2t<Scalar> > > intersections;

         for (size_t i = 0; i != 3; ++i)
         {
            intersections.push_back(intersection(t.side(i), line));
         }

         std::set< point_2t<Scalar> > bounds;

         bool flag = false;

         for (size_t i = 0; i != 3; ++i)
         {
            if (intersections[i].which() == 1)
            {
               bounds.insert(boost::get< point_2t<Scalar> >(intersections[i]));
            }

            if (intersections[i].which() == 2)
            {
               if (orientation(t[i], {x, y0}, {x, y1}) == CG_RIGHT)
               {
                  flag = true;
               }

            }
         }

         if (bounds.size() != 2 || flag)
         {
            continue;
         }

         std::vector< point_2t<Scalar> > b(bounds.begin(), bounds.end());

         Scalar l = floor(std::min(b[0].y, b[1].y));
         Scalar r = ceil(std::max(b[0].y, b[1].y));

         for (Scalar y = l; y != r; ++y)
         {
            ans.push_back({x - 1, y});
            ans.push_back({x, y});
         }
      }

      auto it = std::unique(ans.begin(), ans.end());
      ans.resize(std::distance(ans.begin(), it));
      
      return ans;

   }
}