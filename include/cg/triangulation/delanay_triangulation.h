#pragma once

#include "cg/operations/orientation.h"
#include "cg/primitives/triangle.h"
#include "cg/operations/contains/circumcircle_point.h"
#include "cg/operations/compare_dist.h"

#include <boost/optional.hpp>

#include <iterator>
#include <vector>
#include <random>
#include <array>
#include <utility>
#include <iostream>

namespace cg
{

   template <class Scalar>
   class triangulatable_points_set_2t;

   typedef triangulatable_points_set_2t<double> triangulatable_points_set_2;

   template <class Scalar>
   class triangulatable_points_set_2t
   {

      typedef point_2t<Scalar> point;

      class my_node;
      class my_face;

      typedef typename std::vector<my_node>::iterator node_iterator;
      typedef typename std::vector<my_face>::iterator face_iterator;

      static constexpr double P = 0.5;

      struct my_node
      {
         point p;
         bool inf;
         node_iterator prev_level_node;
         face_iterator face;

         my_node() : inf(false)
         {
         }

         explicit my_node(const point & p) : p(p), inf(false)
         {
         }

         bool operator == (const my_node & b)
         {
            if (inf && b.inf)
            {
               return true;
            }

            if (inf != b.inf)
            {
               return false;
            }

            return p == b.p;
         }

      };

      struct my_face
      {
         std::array<node_iterator, 3> nodes;
         std::array<face_iterator, 3> neighbors;

         my_face()
         {
         }

         my_face(const node_iterator & a, const node_iterator & b, const node_iterator & c)
         {
            nodes[0] = a;
            nodes[1] = b;
            nodes[2] = c;
         }

         void set_neighbors(face_iterator first, face_iterator second, face_iterator third)
         {
            neighbors[0] = first;
            neighbors[1] = second;
            neighbors[2] = third;
         }

         void print() const
         {
            std::cerr << "{(" << nodes[0]->p.x << ", " << nodes[0]->p.y << "), (" << nodes[1]->p.x << ", " << nodes[1]->p.y << "), (" << nodes[2]->p.x << ", " << nodes[2]->p.y << ")}" << std::endl;
         }

         bool is_vertex(const my_node & p) const
         {
            return (*nodes[0] == p) || (*nodes[1] == p) || (*nodes[2] == p);
         }

         void new_neighbor(face_iterator iter)
         {
            for (size_t i = 0; i != 3; ++i)
            {
               if (iter->is_vertex(*nodes[(i + 1) % 3]) && iter->is_vertex(*nodes[(i + 2) % 3]))
               {
                  neighbors[i] = iter;
                  return;
               }

            }
         }

         bool inf() const
         {
            return nodes[0]->inf || nodes[1]->inf || nodes[2]->inf;
         }

         triangle_2t<Scalar> to_triangle() const
         {
            return triangle_2t<Scalar>(nodes[0]->p, nodes[1]->p, nodes[2]->p);
         }

         bool contains(const point & p) const
         {
            return contains(my_node(p));
         }

         bool contains(const my_node & node) const
         {

            if (node.inf)
            {
               std::cerr << "inf in contains argument" << std::endl;
               return inf();
            }

            for (size_t i = 0; i != 3; ++i)
            {

               if (!nodes[i]->inf && !nodes[(i + 1) % 3]->inf && orientation(nodes[i]->p, nodes[(i + 1) % 3]->p, node.p) == CG_RIGHT)
               {
                  //std::cerr << "Containts finished" << std::endl;
                  return false;
               }
            }

            //std::cerr << "Containts finished" << std::endl;

            return true;
         }

         node_iterator & operator [](const size_t i)
         {
            return nodes[i % 3];
         }
      };

      struct layer
      {
         std::vector<my_node> nodes;
         std::vector<my_face> faces;

         size_t size_;

         layer()
         {
            nodes.reserve(100500); // :( TODO: refactor
            faces.reserve(100500); // same
            my_node inf;
            inf.inf = true;
            nodes.push_back(inf);
            size_ = 1;
         }

         size_t size() const
         {
            return size_ - 1;
         }

         bool closer(std::pair<point, point> first, std::pair<point, point> second)
         {
            return compare_dist(first.first, first.second, second.first, second.second);
         }

         node_iterator find_closest(point p)
         {
            auto res = nodes.begin();

            for (auto it = nodes.begin(); it != nodes.end(); ++it)
            {
               if (res->inf)
               {
                  res = it;
                  continue;
               }

               if (!it->inf && closer(std::make_pair(p, it->p), std::make_pair(p, res->p)))
               {
                  res = it;
               }
            }

            return res;

         }

         face_iterator localize(const point & p)
         {
            for (auto it = faces.begin(); it != faces.end(); ++it)
            {
               if (it->contains(p))
               {
                  return it;
               }
            }

            std::cerr << "Something went wrong";
            return faces.begin();
         }

         bool has_intersection(std::pair<my_node, my_node> a, std::pair<my_node, my_node> b)
         {
            if (a.first.inf)
            {
               return orientation(a.second.p, b.first.p, b.second.p) == CG_RIGHT;
            }

            return orientation(b.first.p, b.second.p, a.second.p) != orientation(b.first.p, b.second.p, a.first.p);

         }

         face_iterator localize_from(const point & p, node_iterator from)
         {
            std::cerr << "Localising " << p.x << " " << p.y << " " << std::endl;
            auto res = from->face;

            while (!res->contains(p))
            {
               res->print();

               for (size_t i = 0; i != 3; ++i)
               {

                  if (has_intersection(std::make_pair(*((*res)[i]), my_node(p)), std::make_pair(*((*res)[i + 1]), *((*res)[i + 2]))))
                  {
                     res = res->neighbors[i];
                     break;
                  }
               }
            }

            std::cerr << "Finished localising, found: ";
            res->print();
            return res;
         }

         node_iterator find_closest_from(point p, node_iterator from)
         {
            if (size_ < 3)
            {
               return find_closest(p);
            }

            my_face face = *localize_from(p, from);
            auto res = from;

            for (size_t i = 0; i != 3; ++i)
            {
               auto cur = face[i];

               if (cur->inf)
               {
                  continue;
               }

               if (res->inf || closer(std::make_pair(p, cur->p), std::make_pair(p, res->p)))
               {
                  res = cur;
               }

            }

            return res;
         }

         void three_points() // inf + 2 real points
         {
            //std::cerr << "Three points entered" << std::endl;
            my_face face1(nodes.begin(), nodes.begin() + 1, nodes.begin() + 2);
            my_face face2(nodes.begin(), nodes.begin() + 2, nodes.begin() + 1);

            faces.clear();

            faces.push_back(face1);
            auto f1 = faces.end() - 1;

            faces.push_back(face2);
            auto f2 = faces.end() - 1;

            for (size_t i = 0; i != 3; ++i)
            {
               faces.back()[i]->face = f2;
            }

            f1->set_neighbors(f2, f2, f2);
            f2->set_neighbors(f1, f1, f1);

            //std::cerr << "Three points finished" << std::endl;
         }

         bool circumcircle_contains(my_node a, my_node b, my_node c, my_node p)
         {
            if (p.inf)
            {
               return false;
            }

            if (a.inf + b.inf + c.inf > 1)
            {
               return false;
            }

            if (a.inf)
            {
               return orientation(b.p, c.p, p.p) == CG_LEFT;
            }

            if (b.inf)
            {
               return orientation(c.p, a.p, p.p) == CG_LEFT;
            }

            if (c.inf)
            {
               return orientation(a.p, b.p, p.p) == CG_LEFT;
            }

            return cg::circumcircle_contains(a.p, b.p, c.p, p.p);
         }

         void flip(face_iterator face, size_t neighbor_id, size_t opposite)
         {
            auto neighbor = (face->neighbors[neighbor_id]);
            auto opposite_point = (*neighbor)[opposite];
            std::cerr << "Bad with " << opposite_point->p.x << " " << opposite_point->p.y  << std::endl;
            my_face face1((*face)[neighbor_id + 2], (*face)[neighbor_id], opposite_point);
            my_face face2((*face)[neighbor_id + 1], opposite_point, (*face)[neighbor_id]);
            face1.set_neighbors(neighbor, neighbor->neighbors[(opposite + 2) % 3], face->neighbors[(neighbor_id + 1) % 3]);
            face2.set_neighbors(face, face->neighbors[(neighbor_id + 2) % 3], neighbor->neighbors[(opposite + 1) % 3]);
            *face = face1;
            *neighbor = face2;
            notify_neighbors_and_nodes(face);
            notify_neighbors_and_nodes(neighbor);
            check(face);
            check(neighbor);
         }

         void check(face_iterator face)
         {
            std::cerr << "Checking: ";
            face->print();

            for (size_t i = 0; i != 3; ++i)
            {
               auto cur_neighbor = (face->neighbors[i]);
               size_t opposite = 3;

               for (size_t j = 0; j != 3; ++j)
               {
                  if (cur_neighbor->neighbors[j] == face)
                  {
                     opposite = j;
                     break;
                  }
               }

               if (opposite == 3)
               {
                  std::cerr << "Opposite = 3" << std::endl;
               }

               auto opposite_point = (*cur_neighbor)[opposite];

               if (circumcircle_contains(*(*face)[0], *(*face)[1], *(*face)[2], *opposite_point))
               {
                  flip(face, i, opposite);
               }

            }

         }

         void notify_neighbors_and_nodes(face_iterator face)
         {
            for (size_t i = 0; i != 3; ++i)
            {
               face->neighbors[i]->new_neighbor(face);
               (*face)[i]->face = face;
            }
         }

         void insert_into_face(node_iterator p, face_iterator face_iter)
         {
            my_face face = *face_iter;
            *face_iter = {face[0], face[1], p};
            auto f1 = face_iter;
            //faces.push_back({face[0], face[1], p});
            //auto f1 = faces.end() - 1;
            faces.push_back({face[1], face[2], p});
            auto f2 = faces.end() - 1;
            faces.push_back({face[2], face[0], p});
            auto f3 = faces.end() - 1;

            f1->set_neighbors(f2, f3, face.neighbors[2]);
            notify_neighbors_and_nodes(f1);
            f2->set_neighbors(f3, f1, face.neighbors[0]);
            notify_neighbors_and_nodes(f2);
            f3->set_neighbors(f1, f2, face.neighbors[1]);
            notify_neighbors_and_nodes(f3);

            check(f1);
            check(f2);
            check(f3);
         }

         node_iterator insert(point p)
         {
            nodes.push_back(my_node(p));
            ++size_;

            if (size_ == 3)
            {
               three_points();
            }
            else
            {
               if (size_ > 3)
               {
                  auto face = localize(p);
                  insert_into_face(nodes.end() - 1, face);
               }
            }

            return nodes.end() - 1;
         }

         node_iterator insert_from(point p, node_iterator from)
         {
            nodes.push_back(my_node(p));
            ++size_;

            if (size_ == 3)
            {
               three_points();
            }
            else
            {
               if (size_ > 3)
               {
                  auto face = localize_from(p, from);
                  insert_into_face(nodes.end() - 1, face);
               }
            }

            return nodes.end() - 1;
         }

         std::vector< triangle_2t<Scalar> > get_triangulation() const
         {
            std::vector< triangle_2t<Scalar> > result;

            for (auto face : faces)
            {
               if (!face.inf())
               {
                  result.push_back(face.to_triangle());
               }
            }

            return result;
         }

      };

      std::mt19937 generator;
      std::uniform_real_distribution<> distribution;

      bool random_bool()
      {
         std::cout << distribution(generator) << std::endl;
         return distribution(generator) < P;
         /*std::mt19937 generator;
         std::uniform_real_distribution<> distribution;
         auto dice = std::bind(distribution, generator);

         return dice() < P;
         srand(time(0));
         return rand() % 2;*/

      }

      std::vector<layer> levels;

   public:

      triangulatable_points_set_2t() : levels(1)
      {
      }

      template <class RandIter>
      triangulatable_points_set_2t(RandIter p, RandIter q) : levels(1)
      {
         for (auto it = p; it != q; ++it)
         {
            insert(*it);
         }
      }

      size_t size() const
      {
         return levels.front().size();
      }

      void clear()
      {
         levels.clear();
         levels.push_back(layer());
      }

      bool insert(point p)
      {
         std::cerr << "Inserting " << p.x << " " << p.y << std::endl;
         std::vector<node_iterator> closest(levels.size());
         closest.back() = levels.back().find_closest(p);

         if (closest.back()->p == p)
         {
            return false;
         }

         for (int level = static_cast<int>(levels.size()) - 2; level != -1; --level)
         {
            std::cerr << "Localising on level " << level << std::endl;
            closest[level] = levels[level].find_closest_from(p, closest[level + 1]->prev_level_node);

            if (closest[level]->p == p)
            {
               return false;
            }
         }

         std::cerr << "Level is " << 0 << std::endl;
         std::cerr << "On this level the closest point is " << closest.front()->p.x << " " << closest.front()->p.y << std::endl;
         auto prev = levels.front().insert_from(p, closest.front());

         size_t level = 1;

         while (random_bool())
         {
            if (level == levels.size())
            {
               std::cerr << "Level is " << level << std::endl;
               levels.push_back(layer());
               (levels.back().insert(p))->prev_level_node = prev;
               break;
            }

            std::cerr << "Level is " << level << std::endl;
            std::cerr << "On this level the closest point is " << closest[level]->p.x << " " << closest[level]->p.y << std::endl;
            auto inserted = levels[level].insert_from(p, closest[level]);
            inserted->prev_level_node = prev;
            prev = inserted;
            ++level;
         }

         std::cerr << "Inserted\nLevels = " << levels.size() << std::endl << std::endl;
         return true;
      }

      std::vector< triangle_2t<Scalar> > get_triangulation() const
      {
         return levels.front().get_triangulation();
      }

      boost::optional< triangle_2t<Scalar> > localize(const point_2t<Scalar> p)
      {
         std::vector<node_iterator> closest(levels.size());
         closest.back() = levels.back().find_closest(p);

         for (int level = static_cast<int>(levels.size()) - 2; level != -1; --level)
         {
            closest[level] = levels[level].find_closest_from(p, closest[level + 1]->prev_level_node);
         }

         auto res = levels.front().localize_from(p, closest.front());

         if (res->inf())
         {
            return boost::none;
         }
         else
         {
            return res->to_triangle();
         }

      }

   };

   template <class RandIter>
   std::vector< triangle_2t<typename std::iterator_traits<RandIter>::value_type::scalar_type> > delanay_triangulation(RandIter p, RandIter q)
   {
      typedef typename std::iterator_traits<RandIter>::value_type::scalar_type Scalar;

      triangulatable_points_set_2t<Scalar> points(p, q);

      return points.get_triangulation();
   }

}
