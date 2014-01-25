#pragma once

#include <cg/primitives/point.h>
#include <cg/primitives/segment.h>
#include <cg/primitives/contour.h>

#include <cg/operations/intersection/segment_segment.h>
#include <cg/operations/compare_dist.h>
#include <cg/operations/contains/segment_point.h>

#include <cmath>

#include <vector>
#include <algorithm>
#include <iostream>

namespace cg
{

   template <class Scalar>
   Scalar projection(point_2t<Scalar> p, segment_2t<Scalar> seg)
   {
      auto v = p - seg[0];
      auto s = seg[1] - seg[0];

      return (v * s) / (s * s);

      //auto projection = (v * s) / (s * s) * s;

      //return seg[0] + projection;
   }

   template <class Scalar>
   std::pair< point_2t<Scalar>, point_2t<Scalar> > distance(contour_2t<Scalar> c1, contour_2t<Scalar> c2)
   {
      //typedef typename contour_2t<Scalar>::const_iterator iterator;
      typedef point_2t<Scalar> point;
      auto p = std::min_element(c1.begin(), c1.end());
      auto q = std::max_element(c2.begin(), c2.end());

      auto min = p;
      auto max = q;

      /*      auto next_p = [c1] (iterator p)
            {
               ++p;

               if (p == c1.end())
               {
                  return c1.begin();
               }

               return p;
               //return (p == c1.end()) ? c1.begin() : p;
            };

            auto next_q = [c2] (iterator q)
            {
               ++q;

               if (q == c2.end())
               {
                  return c2.begin();
               }

               return q;
               //return (q == c2.end()) ? c2.begin() : q;
            };*/

      auto p_n = p;
      ++p_n;

      if (p_n == c1.end())
      {
         p_n = c1.begin();
      }

      auto q_n = q;
      ++q_n;

      if (q_n == c2.end())
      {
         q_n = c2.begin();
      }

      auto ans = std::make_pair(*p, *q);
      auto less = [] (point const &a, point const &b,
                      point const &c, point const &d)
      {
         return orientation(a, b, c, d) != CG_RIGHT;
      };


      do
      {
         if (compare_dist(*p, *q, ans.first, ans.second))
         {
            ans = std::make_pair(*p, *q);
         }

         bool less1 = less(*p, *p_n, *q_n, *q);
         bool less2 = less(*p, *p_n, *q, *q_n);

         if (less1 && less2)
         {
            segment_2t<Scalar> seg(*p, *p_n);
            auto pr = projection(*q, seg);

            if (pr >= 0 && pr <= 1)
            {
               auto proj = seg[0] + pr * (seg[1] - seg[0]);

               if (compare_dist(proj, *q, ans.first, ans.second))
               {
                  ans = std::make_pair(proj, *q);
               }
            }

            p = p_n;
            ++p_n;

            if (p_n == c1.end())
            {
               p_n = c1.begin();
            }

            if (compare_dist(*p, *q, ans.first, ans.second))
            {
               ans = std::make_pair(*p, *q);
            }

            seg = {*q, *q_n};
            pr = projection(*p, seg);

            if (pr >= 0 && pr <= 1)
            {
               auto proj = seg[0] + pr * (seg[1] - seg[0]);

               if (compare_dist(proj, *p, ans.first, ans.second))
               {
                  ans = std::make_pair(proj, *p);
               }
            }

            q = q_n;
            ++q_n;

            if (q_n == c2.end())
            {
               q_n = c2.begin();
            }

            if (compare_dist(*p, *q, ans.first, ans.second))
            {
               ans = std::make_pair(*p, *q);
            }

         }
         else
         {
            if (less1)
            {
               segment_2t<Scalar> seg(*p, *p_n);
               auto pr = projection(*q, seg);

               if (pr >= 0 && pr <= 1)
               {
                  auto proj = seg[0] + pr * (seg[1] - seg[0]);

                  if (compare_dist(proj, *q, ans.first, ans.second))
                  {
                     ans = std::make_pair(proj, *q);
                  }
               }

               p = p_n;
               ++p_n;

               if (p_n == c1.end())
               {
                  p_n = c1.begin();
               }

               if (compare_dist(*p, *q, ans.first, ans.second))
               {
                  ans = std::make_pair(*p, *q);
               }
            }
            else
            {
               segment_2t<Scalar> seg(*q, *q_n);
               auto pr = projection(*p, seg);

               if (pr >= 0 && pr <= 1)
               {
                  auto proj = seg[0] + pr * (seg[1] - seg[0]);

                  if (compare_dist(proj, *p, ans.first, ans.second))
                  {
                     ans = std::make_pair(proj, *p);
                  }
               }

               q = q_n;
               ++q_n;

               if (q_n == c2.end())
               {
                  q_n = c2.begin();
               }

               if (compare_dist(*p, *q, ans.first, ans.second))
               {
                  ans = std::make_pair(*p, *q);
               }
            }
         }

      }
      while (p != min || q != max);



      return ans;
   }
}
