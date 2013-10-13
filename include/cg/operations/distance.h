#pragma once

#include <cg/primitives/point.h>
#include <cg/primitives/segment.h>
#include <cmath>

namespace cg
{

   template <class Scalar>
   inline double distance(const point_2t<Scalar> & a, const point_2t<Scalar> & b)
   {
      auto v = b - a;
      return std::sqrt(v * v);
   }

   template <class Scalar>
   inline double projection_on_segment(const segment_2t<Scalar> & s, const point_2t<Scalar> & p)
   {
      auto v = s[1] - s[0];
      return (v * (p - s[0])) / (v * v);
   }

   template <class Scalar>
   inline double length(const segment_2t<Scalar> & s)
   {
      return distance(s[0], s[1]);
   }

   template <class Scalar>
   inline double distance(const segment_2t<Scalar> & s, const point_2t<Scalar> & p)
   {
      auto projection = projection_on_segment(s, p);

      if (projection < 0)
      {
         return distance(s[0], p);
      }
      else if (projection > 1)
      {
         return distance(s[1], p);
      }
      else
      {
         return fabs((s[1] - s[0]) ^ (p - s[0])) / length(s);
      }
   }
}
