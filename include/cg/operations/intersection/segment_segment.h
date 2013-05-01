#pragma once

#include <boost/variant.hpp>
#include <boost/none_t.hpp>
#include <gmpxx.h>
#include <cg/primitives/point.h>
#include <cg/primitives/segment.h>
#include <boost/numeric/interval.hpp>
#include <cg/operations/has_intersection/segment_segment.h>
#include <cmath>

namespace cg
{
   typedef boost::variant<boost::none_t, point_2, segment_2> segment_segment_intersection_t;


   namespace detail
   {
      bool same_hotpixel ( double a, double b, double bound )
      {

         double hp1 = floor ( a / bound );
         double hp2 = floor ( b / bound );

         return hp1 == hp2;
      }

      template<class Scalar>
      point_2t<Scalar> segment_intersection_impl ( segment_2t<Scalar> const & a, segment_2t<Scalar> const & b )
      {
         Scalar det = ( a[0].x - a[1].x ) * ( b[0].y - b[1].y ) - ( a[0].y - a[1].y ) * ( b[0].x - b[1].x );
         Scalar t1 = a[0].x * a[1].y - a[0].y * a[1].x;
         Scalar t2 = b[0].x * b[1].y - b[0].y * b[1].x;

         Scalar x = ( t1 * ( b[0].x - b[1].x ) - ( a[0].x - a[1].x ) * t2 ) / det;
         Scalar y = ( t1 * ( b[0].y - b[1].y ) - ( a[0].y - a[1].y ) * t2 ) / det;

         return {x, y};
      }

      boost::optional<point_2> intersection_d ( segment_2 const & a, segment_2 const & b, int eps_pwr )
      {
         typedef double Scalar;

         double det = ( a[0].x - a[1].x ) * ( b[0].y - b[1].y ) - ( a[0].y - a[1].y ) * ( b[0].x - b[1].x );
         double t1 = a[0].x * a[1].y - a[0].y * a[1].x;
         double t2 = b[0].x * b[1].y - b[0].y * b[1].x;

         double det_sup = ( a[0].x + a[1].x ) * ( b[0].y + b[1].y ) + ( a[0].y + a[1].y ) * ( b[0].x + b[1].x );
         int det_ind = 4;
         double t1_sup = a[0].x * a[1].y + a[0].y * a[1].x;
         int t1_ind = 2;
         double t2_sup = b[0].x * b[1].y + b[0].y * b[1].x;
         int t2_ind = 2;

         double xt = t1 * ( b[0].x - b[1].x ) - ( a[0].x - a[1].x ) * t2;
         double yt = t1 * ( b[0].y - b[1].y ) - ( a[0].y - a[1].y ) * t2;

         double xt_sup = t1_sup * ( b[0].x + b[1].x ) + ( a[0].x + a[1].x ) * t2_sup;
         int xt_ind = 4;
         double yt_sup = t1_sup * ( b[0].y + b[1].y ) + ( a[0].y + a[1].y ) * t2_sup;
         int yt_ind = 4;

         double x = ( xt ) / det;
         double y = ( yt ) / det;

         double x_sup = ( fabs ( xt / det ) + xt_sup / det_sup ) / ( fabs ( det ) / det_sup - ( det_ind + 1 ) * 2 * std::numeric_limits<double>::epsilon() );
         int x_ind = 6;
         double y_sup = ( fabs ( yt / det ) + yt_sup / det_sup ) / ( fabs ( det ) / det_sup - ( det_ind + 1 ) * 2 * std::numeric_limits<double>::epsilon() );
         int y_ind = 6;

         double x_eps = x_sup * x_ind * 2 * std::numeric_limits<double>::epsilon();
         double y_eps = y_sup * y_ind * 2 * std::numeric_limits<double>::epsilon();

         double bound = pow ( 2.0, eps_pwr );
         
         std::cout << "int_d: (" << x << ", " << y << ")" << std::endl;

         if ( same_hotpixel ( x - x_eps, x + x_eps, bound ) && same_hotpixel ( y - y_eps, y + y_eps, bound ) )
            return point_2 ( x, y );

         return boost::none;
      }

      boost::optional<point_2> intersection_i ( segment_2 const & a, segment_2 const & b, int eps_pwr )
      {
         typedef boost::numeric::interval_lib::unprotect<boost::numeric::interval<double> >::type interval;

         boost::numeric::interval<double>::traits_type::rounding _;

         segment_2t<interval> a_i {{interval(a[0].x), interval(a[0].y) }, { interval(a[1].x), interval(a[1].y) }};
         segment_2t<interval> b_i {{interval(b[0].x), interval(b[0].y) }, { interval(b[1].x), interval(b[1].y) }};

         auto res = segment_intersection_impl ( a_i, b_i );

         double bound = pow ( 2.0, eps_pwr );

         std::cout << "int_i: x=[" << res.x.lower() << ", " << res.x.upper() << "]\n";
         std::cout << "int_i: y=[" << res.y.lower() << ", " << res.y.upper() << "]" << std::endl;
         
         if ( same_hotpixel ( res.x.lower(), res.x.upper(), bound )
               && same_hotpixel ( res.y.lower(), res.y.upper(), bound ) )
            return point_2 ( res.x.lower(), res.y.lower() );

         return boost::none;
      }

      point_2 intersection_r ( segment_2 const& a, segment_2 const& b )
      {
         segment_2t<mpq_class> a_r {{a[0].x, a[0].y}, {a[1].x, a[1].y}};
         segment_2t<mpq_class> b_r {{b[0].x, b[0].y}, {b[1].x, b[1].y}};
         auto result = segment_intersection_impl ( a_r, b_r );
         std::cout << "int_r: (" << result.x.get_d() << ", " << result.y.get_d() << ")" << std::endl;
         return point_2 ( result.x.get_d(), result.y.get_d() );
      }
   }


   inline segment_segment_intersection_t intersection ( segment_2 const & a, segment_2 const & b, int eps_pwr )
   {
      if ( a[0] == a[1] )
         return has_intersection ( a, b ) ? segment_segment_intersection_t ( a[0] ) : segment_segment_intersection_t ( boost::none );

      if ( b[0] == b[1] )
         return has_intersection ( a, b ) ? segment_segment_intersection_t ( b[0] ) : segment_segment_intersection_t ( boost::none );

      orientation_t ab[2];

      for ( size_t l = 0; l != 2; ++l )
         ab[l] = orientation ( a[0], a[1], b[l] );

      if ( ab[0] == ab[1] && ab[0] == CG_COLLINEAR ) {
         point_2 beg = std::max ( min ( a ), min ( b ) );
         point_2 end = std::min ( max ( a ), max ( b ) );

         if ( beg == end ) return b;
         else if ( beg < end ) return segment_2 ( beg, end );
         else return segment_segment_intersection_t ( boost::none );
      }

      if ( ab[0] == ab[1]
            || orientation ( b[0], b[1], a[0] ) == orientation ( b[0], b[1], a[1] ) ) {
         return boost::none;
      }

      std::cout.precision(20);
      /*auto res_d = detail::intersection_d ( a, b, eps_pwr );
      if ( res_d.is_initialized() ) {
         return *res_d;
      }*/

      auto res = detail::intersection_i ( a, b, eps_pwr );
      if (res) {
         return *res;
      }
      
      return boost::none;

      return detail::intersection_r ( a, b );

   }
}
