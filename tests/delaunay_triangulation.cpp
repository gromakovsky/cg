#include <gtest/gtest.h>

#include <boost/assign/list_of.hpp>

#include "cg/primitives/point.h"
#include "cg/primitives/triangle.h"
#include "cg/triangulation/delaunay_triangulation.h"
#include "cg/operations/contains/circumcircle_point.h"
#include <misc/random_utils.h>

#include "random_utils.h"

using namespace util;

TEST(delaunay_triangulation, uniform_points)
{
   std::vector<cg::point_2> pts = uniform_points(10000);
   auto triangulation = cg::delaunay_triangulation(pts.begin(), pts.end());

   for (auto triangle : triangulation)
   {
      for (auto point : pts)
      {
         if (triangle[0] == point || triangle[1] == point || triangle[2] == point)
         {
            continue;
         }

         EXPECT_EQ(cg::circumcircle_contains(triangle, point), false);

      }
   }
}
