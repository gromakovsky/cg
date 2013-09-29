#include <cg/operations/intersection/segment_segment.h>
#include <gtest/gtest.h>
#include <vector>
#include <initializer_list>
#include <boost/variant.hpp>
#include "random_utils.h"

using namespace cg;
const int EPS_PWR = -300;

TEST ( intersection, simple )
{
   auto test = cg::intersection ( {{0, 0}, {1, 1}}, {{1, 0}, {0, 1}}, -2 );

   ASSERT_TRUE ( test == cg::segment_segment_intersection_t ( cg::point_2 ( 0.5, 0.5 ) ) );
}

TEST ( intersection, singular_segments )
{
   auto res = cg::intersection ( {{0, 0}, {0, 0}}, {{-1, -1}, {1, 1}}, -500 );
   ASSERT_TRUE ( res == segment_segment_intersection_t ( point_2 ( 0, 0 ) ) );

   res = cg::intersection ( {{-1, -1}, {1, 1}}, {{0, 0}, {0, 0}}, -500 );
   ASSERT_TRUE ( res == segment_segment_intersection_t ( point_2 ( 0, 0 ) ) );

   res = cg::intersection ( {{0, 0}, {0, 0}}, {{0, 0}, {0, 0}}, -500 );
   ASSERT_TRUE ( res == segment_segment_intersection_t ( point_2 ( 0, 0 ) ) );
}

TEST ( intersection, segment )
{
   auto res = cg::intersection ( {{0, 0}, {1, 1}}, {{-1, -1}, {1, 1}}, -500 );
   ASSERT_TRUE ( res == segment_segment_intersection_t ( segment_2 ( {0, 0}, {1, 1} ) ) );
}

struct : boost::static_visitor<>
{
   void operator() ( boost::none_t& )
   {}

   void operator() ( segment_2& )
   {}

   void operator() ( point_2& p )
   {
      point_2 rational_res = detail::intersection_r ( {(*points)[l], (*points)[lp]}, {(*points)[k], (*points)[kp]} );
      auto bound = pow ( 2.0, EPS_PWR );
      ASSERT_TRUE ( fabs(rational_res.x - p.x) <= bound && fabs(rational_res.y - p.y) <= bound );
   }

   std::vector<point_2> * points = nullptr;
   int l, k, lp, kp;
} visitor;

TEST ( intersection, random )
{
   auto points = uniform_points ( 1000 );

   visitor.points = &points;

   for ( size_t l = 0, lp = points.size() - 1; l != points.size(); lp = l++ )
   {
      for ( size_t k = 0, kp = points.size() - 1; k != points.size(); kp = k++ )
      {
         auto res = intersection ( {points[l], points[lp]}, {points[k], points[kp]}, EPS_PWR );
         visitor.l = l;
         visitor.k = k;
         visitor.lp = lp;
         visitor.kp = kp;
         res.apply_visitor<> ( visitor );
      }
   }
}

