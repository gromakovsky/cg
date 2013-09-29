#pragma once

#include "cg/primitives/point.h"
#include "cg/primitives/segment.h"
#include "cg/operations/distance.h"
#include "cg/operations/orientation.h"

#include <boost/utility.hpp>
#include <iterator>
#include <algorithm>

namespace cg
{
   namespace detail
   {
      template <class BidIter, class OutIter>
      OutIter simplify_impl(BidIter p, BidIter q, OutIter out, double eps)
      {
         --q;

         if (p == q)
         {
            return out;
         }

         typedef typename std::iterator_traits<BidIter>::value_type point;
         typedef typename point::Sc Scalar;

         segment_2t<Scalar> s(*p, *q);
         BidIter it_max = std::max_element(boost::next(p), q, [p, q, &s](const point a, point b)
         {
            return distance(s, a) < distance(s, b);
         });

         auto max = distance(s, *it_max);

         if (max > eps)
         {
            out = simplify_impl(p, boost::next(it_max), out, eps);
            return simplify_impl(it_max, ++q, out, eps);
         }
         else
         {
            *out++ = *q;
            return out;
         }
      }
   }

   template <class BidIter, class OutIter>
   OutIter simplify(BidIter p, BidIter q, OutIter out, double eps)
   {
      if (p == q)
      {
         return out;
      }

      *out++ = *p;
      return detail::simplify_impl(p, q, out, eps);
   }

}
