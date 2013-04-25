#pragma once

#include <algorithm>

#include <cg/operations/orientation.h>

#include "graham.h"

namespace cg
{
   template <class _RandomAccessIterator>
   _RandomAccessIterator andrew_hull(_RandomAccessIterator p, _RandomAccessIterator q)
   {
      if (p == q)
         return p;
      if (boost::next(p) == q)
         return q;

      std::iter_swap(p, std::min_element(p, q));
      std::iter_swap(boost::next(p), std::max_element(p, q));
      auto bound = std::partition(boost::next(boost::next(p)), q, [p] (point_2 const & x) {
         return orientation(x, *p, *boost::next(p)) != CG_LEFT;
      });
      std::sort(p, bound);
      std::sort(bound, q, std::greater<typename std::iterator_traits<_RandomAccessIterator>::value_type>());

      return contour_graham_hull(p, q);
   }
}
