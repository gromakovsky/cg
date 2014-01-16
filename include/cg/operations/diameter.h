#pragma once

#include <cg/convex_hull/andrew.h>
#include <cg/operations/compare_dist.h>

#include <utility>
#include <algorithm>

namespace cg
{
   template <typename ForwardIter>
   std::pair<ForwardIter, ForwardIter> diameter(ForwardIter begin, ForwardIter end)
   {
      typedef typename std::iterator_traits<ForwardIter>::value_type point;
      std::vector<point> points(begin, end);
      auto convex_begin = points.begin();
      auto convex_end = andrew_hull(points.begin(), points.end());
      auto minmax = std::minmax_element(convex_begin, convex_end);

      auto next_iter = [convex_begin, convex_end] (ForwardIter it)
      {
         ++it;
         return (it == convex_end ? convex_begin : it);
      };

      auto p = minmax.first;
      auto p_n = next_iter(p);
      auto q = minmax.second;
      auto q_n = next_iter(q);
      auto ans = std::make_pair(*p, *q);
      auto less = [] (point const &a, point const &b,
                      point const &c, point const &d)
      {
         return orientation(a, b, c, d) == CG_LEFT;
      };

      do
      {
         if (compare_dist(ans.first, ans.second, *p, *q))
         {
            ans = std::make_pair(*p, *q);
         }

         if (less(*p, *p_n, *q_n, *q))
         {
            p = p_n;
            p_n = next_iter(p);
         }
         else
         {
            q = q_n;
            q_n = next_iter(q);
         }
      }
      while (p != minmax.first || q != minmax.second);

      return std::make_pair(std::find(begin, end, ans.first), std::find(begin, end, ans.second));
   }
}