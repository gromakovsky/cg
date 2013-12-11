#include <gtest/gtest.h>

#include <boost/assign/list_of.hpp>

#include "cg/primitives/point.h"
#include "cg/primitives/triangle.h"
#include "cg/triangulation/delaunay_triangulation.h"
#include "cg/operations/contains/circumcircle_point.h"
#include <misc/random_utils.h>

#include "random_utils.h"

using namespace util;
using cg::point_2;
using cg::triangle_2;

bool check_delaunay(const std::vector<point_2> & pts, const std::vector<cg::triangle_2> & triangulation)
{
   for (auto triangle : triangulation)
   {
      for (auto point : pts)
      {
         if (triangle[0] == point || triangle[1] == point || triangle[2] == point)
         {
            continue;
         }

         if (cg::circumcircle_contains(triangle, point), false)
         {
            return false;
         }

      }
   }

   return true;
}

TEST(delaunay_triangulation, uniform_points)
{
   std::vector<cg::point_2> pts = uniform_points(4000);
   auto triangulation = cg::delaunay_triangulation(pts.begin(), pts.end());
   EXPECT_TRUE(check_delaunay(pts, triangulation));
}

TEST(delaunay_triangulation, uniform_points_on_circle)
{
   const double PI = asin(1) * 2;
   const int COUNT = 360;
   const double r = 100;
   const point_2 c(0, 0);
   std::vector<cg::point_2> pts;

   for (int i = 0; i != COUNT; i++)
   {
      double angle = PI * 2 * i / COUNT;
      double x = c.x + r * cos(angle);
      double y = c.y + r * sin(angle);
      pts.push_back({x, y});
   }

   auto triangulation = cg::delaunay_triangulation(pts.begin(), pts.end());
   EXPECT_TRUE(check_delaunay(pts, triangulation));
}

TEST(delaunay_triangulation, uniform_points_on_rectangle)
{
   const double COUNT = 100;
   std::vector<cg::point_2> pts;

   for (double i = 0; i <= COUNT; i++)
   {
      pts.push_back({i, COUNT});
      pts.push_back({-i, COUNT});
      pts.push_back({i, -COUNT});
      pts.push_back({-i, -COUNT});
   }

   for (double i = 0; i <= COUNT; ++i)
   {
      pts.push_back({COUNT, i});
      pts.push_back({COUNT, -i});
      pts.push_back({-COUNT, i});
      pts.push_back({-COUNT, -i});
   }

   auto triangulation = cg::delaunay_triangulation(pts.begin(), pts.end());
   EXPECT_TRUE(check_delaunay(pts, triangulation));
}

TEST(delaunay_triangulation, uniform_points_on_horizontal_line)
{
   const double COUNT = 1000;
   std::vector<cg::point_2> pts;

   for (double i = 0; i <= COUNT; i++)
   {
      pts.push_back({i, COUNT});
   }

   pts.push_back({-COUNT, -COUNT});

   auto triangulation = cg::delaunay_triangulation(pts.begin(), pts.end());
   EXPECT_TRUE(check_delaunay(pts, triangulation));
}

TEST(delaunay_triangulation, uniform_points_on_vertical_line)
{
   const double COUNT = 1000;
   std::vector<cg::point_2> pts;

   for (double i = 0; i <= COUNT; i++)
   {
      pts.push_back({COUNT, i});
   }

   pts.push_back({-COUNT, -COUNT});

   auto triangulation = cg::delaunay_triangulation(pts.begin(), pts.end());
   EXPECT_TRUE(check_delaunay(pts, triangulation));
}
