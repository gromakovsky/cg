#pragma once

#include <cg/primitives/rectangle.h>
#include <cg/primitives/segment.h>
#include <cg/primitives/triangle.h>

#include <cg/operations/has_intersection/segment_segment.h>
#include <cg/operations/contains/triangle_point.h>

namespace cg
{
   template<class Scalar>
   bool has_intersection(rectangle_2t<Scalar> const & r, triangle_2t<Scalar> const & t)
   {
      if (contains(t, r.corner(0, 0)) || contains(t, r.corner(1, 0)) || contains(t, r.corner(0, 1)) || contains(t, r.corner(1, 1)))
      {
         return true;
      }

      for (size_t i = 0; i != 3; ++i)
      {
         auto s = t.side(i);

         if (has_intersection(segment_2t<Scalar>(r.corner(0, 0), r.corner(0, 1)), s) ||
               has_intersection(segment_2t<Scalar>(r.corner(0, 1), r.corner(1, 1)), s) ||
               has_intersection(segment_2t<Scalar>(r.corner(1, 1), r.corner(1, 0)), s) ||
               has_intersection(segment_2t<Scalar>(r.corner(1, 0), r.corner(0, 0)), s)
            )
         {
            return true;
         }

      }

      return false;
   }
}
