#pragma once

#include <iterator>
#include <algorithm>

#include <cg/operations/orientation.h>

#include "graham.h"

namespace cg
{
   template <class RandIter>
   RandIter andrew_hull(RandIter p, RandIter q)
   {
      if (p == q)
      {
         return p;
      }

      typedef typename std::iterator_traits<RandIter>::value_type point;

      if (std::next(p) == q)
      {
         return q;
      }

      std::iter_swap(p, std::min_element(p, q));
      std::iter_swap(std::next(p), std::max_element(p, q));
      auto bound = std::partition(std::next(std::next(p)), q, [p] (point const & x)
      {
         return orientation(x, *p, *std::next(p)) != CG_LEFT;
      });
      std::sort(p, bound);
      std::sort(bound, q, std::greater<point>());

      return contour_graham_hull(p, q);
   }
}
