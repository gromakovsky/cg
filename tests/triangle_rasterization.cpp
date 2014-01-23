#include <gtest/gtest.h>

#include "random_utils.h"

#include <cg/exam/triangle_rasterization.h>
#include <cg/operations/has_intersection/rectangle_triangle.h>

TEST(triangle_rasterization, random)
{
   using cg::point_2;
   const size_t N = 1000;

   for (size_t i = 0; i != N; ++i)
   {
      std::vector<cg::point_2> pts = uniform_points(3);
      cg::triangle_2 t(pts[0], pts[1], pts[2]);

      auto result = cg::rasterize(t);

      bool ok = true;

      for (auto p : result)
      {
         cg::rectangle_2 r({p.first, p.first + 1}, {p.second, p.second + 1});
         ok &= has_intersection(r, t);

         if (!ok)
         {
            std::cerr << p.first << " " << p.second << std::endl;
            break;
         }
      }

      EXPECT_TRUE(ok);
   }

}