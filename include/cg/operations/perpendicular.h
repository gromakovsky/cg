#pragma once

#include <cg/primitives/point.h>
#include <cg/primitives/vector.h>

#include <algorithm>

namespace cg
{
   // returns point c: ab * bc == 0
   template <class Scalar>
   inline point_2t<Scalar> perpendicular(const point_2t<Scalar> & a, const point_2t<Scalar> & b)
   {
      auto v = b - a;
      std::swap(v.x, v.y);
      v.x = -v.x;
      return b + v;
   }
}

