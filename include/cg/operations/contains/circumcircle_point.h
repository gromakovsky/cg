// From https://github.com/BorisMinaev/cg
#pragma once

#include "cg/primitives/point.h"
#include <boost/numeric/interval.hpp>
#include <gmpxx.h>

#include <boost/optional.hpp>

#include <array>

namespace cg
{

   struct in_circle_r
   {
      boost::optional<bool> operator() (point_2 const & a, point_2 const & b, point_2 const & c, point_2 const & d) const
      {
         mpq_class a00 = (mpq_class(a.x) - d.x);
         mpq_class a01 = (mpq_class(a.y) - d.y);
         mpq_class a02 = (mpq_class(a.x)*a.x - mpq_class(d.x)*d.x) + (mpq_class(a.y)*a.y - mpq_class(d.y)*d.y);
         //std::cerr << "a02 = " << a02.get_d() << std::endl;
         mpq_class a10 = (mpq_class(b.x) - d.x);
         mpq_class a11 = (mpq_class(b.y) - d.y);
         mpq_class a12 = (mpq_class(b.x)*b.x - mpq_class(d.x)*d.x) + (mpq_class(b.y)*b.y - mpq_class(d.y)*d.y);
         mpq_class a20 = (mpq_class(c.x) - d.x);
         mpq_class a21 = (mpq_class(c.y) - d.y);
         mpq_class a22 = (mpq_class(c.x)*c.x - mpq_class(d.x)*d.x) + (mpq_class(c.y)*c.y - mpq_class(d.y)*d.y);
         mpq_class det =  a00 * a11 * a22 + a01 * a12 * a20 + a02 * a10 * a21 -
                          (a20 * a11 * a02 + a21 * a12 * a00 + a01 * a10 * a22);
         //std::cerr << det.get_d() << std::endl;
         return det > 0;
      }
   };

   struct in_circle_d
   {
      boost::optional<bool> operator() (point_2 const & a, point_2 const & b, point_2 const & c, point_2 const & d) const
      {
         return boost::none; // :(

         double a00 = (a.x - d.x);
         double a01 = ((a.y) - d.y);
         double a02 = ((a.x) * a.x - (d.x) * d.x) + ((a.y) * a.y - (d.y) * d.y);
         double a10 = ((b.x) - d.x);
         double a11 = ((b.y) - d.y);
         double a12 = ((b.x) * b.x - (d.x) * d.x) + ((b.y) * b.y - (d.y) * d.y);
         double a20 = ((c.x) - d.x);
         double a21 = ((c.y) - d.y);
         double a22 = ((c.x) * c.x - (d.x) * d.x) + ((c.y) * c.y - (d.y) * d.y);
         double m1 = a00*a11*a22, m2 = a01*a12*a20, m3 = a02*a10*a21, m4=a20*a11*a02, m5=a21*a12*a00, m6=a01*a10*a22;
         double det =  m1+m2+m3-m4-m5-m6;
         double sum = fabs(m1)+fabs(m2)+fabs(m3)+fabs(m4)+fabs(m5)+fabs(m6);
         double eps = sum * 16 * std::numeric_limits<double>::epsilon();
         /*std::cerr << "-------------------" << std::endl;
         std::cerr << a00 << " " << a01 << " " << a02 << std::endl
                   << a10 << " " << a11 << " " << a12 << std::endl
                   << a20 << " " << a21 << " " << a22 << std::endl;
         std::cerr << "det = " << det << "; eps = " << eps << std::endl;
         std::cerr << *in_circle_r()(a, b, c, d) << std::endl;
         std::cerr << "-------------------" << std::endl << std::endl;*/

         if (det > eps)
         {
            return true;
         }

         if (det < -eps)
         {
            return false;
         }

         return boost::none;
      }
   };

   struct in_circle_i
   {
      boost::optional<bool> operator() (point_2 const & a, point_2 const & b, point_2 const & c, point_2 const & d) const
      {
         typedef boost::numeric::interval_lib::unprotect<boost::numeric::interval<double> >::type interval;
         boost::numeric::interval<double>::traits_type::rounding _;
         interval a00 = (interval(a.x) - d.x);
         interval a01 = (interval(a.y) - d.y);
         interval a02 = (interval(a.x)*a.x - interval(d.x)*d.x) + (interval(a.y)*a.y - interval(d.y)*d.y);
         interval a10 = (interval(b.x) - d.x);
         interval a11 = (interval(b.y) - d.y);
         interval a12 = (interval(b.x)*b.x - interval(d.x)*d.x) + (interval(b.y)*b.y - interval(d.y)*d.y);
         interval a20 = (interval(c.x) - d.x);
         interval a21 = (interval(c.y) - d.y);
         interval a22 = (interval(c.x)*c.x - interval(d.x)*d.x) + (interval(c.y)*c.y - interval(d.y)*d.y);
         interval det =  a00 * a11 * a22 + a01 * a12 * a20 + a02 * a10 * a21 -
                         (a20 * a11 * a02 + a21 * a12 * a00 + a01 * a10 * a22);

         if (det.lower() > 0)
         {
            return true;
         }

         if (det.upper() < 0)
         {
            return false;
         }

         return boost::none;
      }
   };

   // true if circumcircle of tr contains p
   inline bool circumcircle_contains(const triangle_2 & tr, const point_2 & p)
   {
      auto a = tr[0];
      auto b = tr[1];
      auto c = tr[2];

      if (boost::optional<bool> v = in_circle_d()(a, b, c, p))
      {
         return *v;
      }

      if (boost::optional<bool> v = in_circle_i()(a, b, c, p))
      {
         return *v;
      }

      return *in_circle_r()(a, b, c, p);
   }
}

