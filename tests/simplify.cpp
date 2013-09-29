#include <cg/operations/simplify.h>
#include <gtest/gtest.h>
#include <vector>

using namespace cg;

TEST(simplify, test1)
{
   std::vector<point_2> v
   {
      {0, 0}, {1, 1}, {2, 2}
   };

   std::vector<point_2> simple;
   simplify(v.begin(), v.end(), std::back_inserter(simple), 0.1);

   std::vector<point_2> expected
   {
      {0, 0}, {2, 2}
   };
   ASSERT_EQ(simple, expected);
}

TEST(simplify, test2)
{
   std::vector<point_2> v
   {
      {0, 0}, {1, 0}, {0, 0.001}
   };
   std::vector<point_2> simple;
   simplify(v.begin(), v.end(), std::back_inserter(simple), 0.5);

   ASSERT_EQ(v, simple);
}

TEST(simplify, test3)
{
   std::vector<point_2> v
   {
      {0, 0}, {1, 0}, {1, 0}
   };
   std::vector<point_2> simple;
   simplify(v.begin(), v.end(), std::back_inserter(simple), 0.1);

   std::vector<point_2> expected
   {
      {0, 0}, {1, 0}
   };
   ASSERT_EQ(simple, expected);
}

TEST(simplify, test4)
{
   std::vector<point_2> v
   {
      {0, 0}, {1, 0}, {2, 0}, {3, 0}
   };
   std::vector<point_2> simple;
   simplify(v.begin(), v.end(), std::back_inserter(simple), 6);

   std::vector<point_2> expected
   {
      {0, 0}, {3, 0}
   };
   ASSERT_EQ(simple, expected);
}

