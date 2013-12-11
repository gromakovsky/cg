#pragma once

#include "cg/operations/orientation.h"
#include "cg/primitives/triangle.h"
#include "cg/operations/contains/circumcircle_point.h"
#include "cg/operations/compare_dist.h"

#include <boost/optional.hpp>

#include <iterator>
#include <vector>
#include <list>
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

      struct my_node;
      struct my_face;

      typedef typename std::list<my_node>::iterator node_iterator;
      typedef typename std::list<my_face>::iterator face_iterator;

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

         void print()
         {
            if (inf)
            {
               std::cerr << "inf";
            }
            else
            {
               std::cerr << "(" << p.x << ", " << p.y << ")";
            }
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
            for (size_t i = 0; i != 3; ++i)
            {
               nodes[i]->print();
               std::cerr << ", ";
            }

            std::cerr << std::endl;
         }

         void print_all() const
         {
            std::cerr << "Face: ";
            print();
            std::cerr << "Neighbors: ";

            for (size_t i = 0; i != 3; ++i)
            {
               neighbors[i]->print();
            }
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

         bool is_line()
         {
            return !inf() && orientation(nodes[0]->p, nodes[1]->p, nodes[2]->p) == CG_COLLINEAR;
         }
      };

      struct layer
      {
         std::list<my_node> nodes;
         std::list<my_face> faces;

         layer()
         {
            my_node inf;
            inf.inf = true;
            nodes.push_back(inf);
         }

         size_t size() const
         {
            return nodes.size() - 1;
         }

         bool closer(std::pair<point, point> first, std::pair<point, point> second)
         {
            return compare_dist(first.first, first.second, second.first, second.second);
         }

         bool has_intersection(std::pair<my_node, my_node> a, std::pair<my_node, my_node> b)
         {
            if (a.first.inf)
            {
               return orientation(b.first.p, b.second.p, a.second.p) == CG_RIGHT;
            }

            if (b.first.inf || b.second.inf)
            {
               return false;
            }

            if (orientation(b.first.p, b.second.p, a.second.p) == CG_COLLINEAR)
            {
               return false;
            }

            return orientation(b.first.p, b.second.p, a.second.p) != orientation(b.first.p, b.second.p, a.first.p);

         }

         face_iterator localize(const point & p, boost::optional<node_iterator> close_point)
         {
            if (!close_point)
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
            else
            {
               auto res = (*close_point)->face;
               //res->print();

               while (!res->contains(p))
               {
                  for (size_t i = 0; i != 3; ++i)
                  {

                     if (has_intersection(std::make_pair(*((*res)[i]), my_node(p)), std::make_pair(*((*res)[i + 1]), *((*res)[i + 2]))))
                     {
                        res = res->neighbors[i];
                        //res->print();
                        break;
                     }
                  }
               }

               return res;

            }
         }

         node_iterator find_closest(point p, boost::optional<node_iterator> close_point)
         {
            if (size() < 2 || !close_point)
            {
               auto res = nodes.begin();

               for (auto it = nodes.begin(); it != nodes.end(); ++it)
               {
                  if (it->inf)
                  {
                     continue;
                  }

                  if (res->inf)
                  {
                     res = it;
                     continue;
                  }

                  if (closer(std::make_pair(p, it->p), std::make_pair(p, res->p)))
                  {
                     res = it;
                  }
               }

               return res;
            }
            else
            {
               my_face face = *localize(p, close_point);
               auto res = *close_point;

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

         }

         void three_points() // inf + 2 real points
         {
            faces.clear();

            faces.push_back({nodes.begin(), std::next(nodes.begin()), std::next(std::next(nodes.begin()))});
            auto f1 = std::prev(faces.end());

            faces.push_back({nodes.begin(), std::next(std::next(nodes.begin())), std::next(nodes.begin())});
            auto f2 = std::prev(faces.end());

            for (size_t i = 0; i != 3; ++i)
            {
               faces.back()[i]->face = f2;
            }

            f1->set_neighbors(f2, f2, f2);
            f2->set_neighbors(f1, f1, f1);
         }

         bool circumcircle_contains(const my_node & a, const my_node & b, const my_node & c, const my_node & p) const
         {
            if (p.inf)
            {
               return false;
            }

            if (a.inf + b.inf + c.inf > 1)
            {
               std::cerr << "Strange situation" << std::endl;
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

            return cg::circumcircle_contains({a.p, b.p, c.p}, p.p);
         }

         void flip(face_iterator face, size_t neighbor_id, size_t opposite)
         {
            auto neighbor = (face->neighbors[neighbor_id]);
            auto opposite_point = (*neighbor)[opposite];
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

         void split_face(node_iterator p, face_iterator face_iter)
         {
            my_face face = *face_iter;
            faces.erase(face_iter);
            faces.push_back({face[1], face[2], p});
            faces.push_back({face[2], face[0], p});
            faces.push_back({face[0], face[1], p});
         }

         void insert_into_face(node_iterator p, face_iterator face_iter)
         {
            my_face face = *face_iter;
            //face.print();
            split_face(p, face_iter);
            std::array<face_iterator, 3> iterators;
            iterators[2] = std::prev(faces.end());
            iterators[1] = std::prev(iterators[2]);
            iterators[0] = std::prev(iterators[1]);

            for (size_t i = 0; i != 3; ++i)
            {
               if (iterators[i]->is_line())
               {
                  faces.erase(iterators[i]);
                  auto opposite = face.neighbors[i];
                  my_face opposite_face = *opposite;
                  split_face(p, opposite);

                  std::array<face_iterator, 3> opposite_iterators;
                  opposite_iterators[2] = std::prev(faces.end());
                  opposite_iterators[1] = std::prev(opposite_iterators[2]);
                  opposite_iterators[0] = std::prev(opposite_iterators[1]);

                  size_t j = 0;

                  for (j = 0; j != 3; ++j)
                  {
                     if (opposite_iterators[j]->is_line())
                     {
                        break;
                     }
                  }

                  if (j == 3)
                  {
                     std::cerr << "opposite_iterators[j]->is_line not found" << std::endl;
                  }

                  faces.erase(opposite_iterators[j]);

                  opposite_iterators[(j + 1) % 3]->set_neighbors(opposite_iterators[(j + 2) % 3], iterators[(i + 2) % 3], opposite_face.neighbors[(j + 1) % 3]);
                  notify_neighbors_and_nodes(opposite_iterators[(j + 1) % 3]);
                  opposite_iterators[(j + 2) % 3]->set_neighbors(iterators[(i + 1) % 3], opposite_iterators[(j + 1) % 3], opposite_face.neighbors[(j + 2) % 3]);
                  notify_neighbors_and_nodes(opposite_iterators[(j + 2) % 3]);

                  iterators[(i + 1) % 3]->set_neighbors(iterators[(i + 2) % 3], opposite_iterators[(j + 2) % 3], face.neighbors[(i + 1) % 3]);
                  notify_neighbors_and_nodes(iterators[(i + 1) % 3]);
                  iterators[(i + 2) % 3]->set_neighbors(opposite_iterators[(j + 1) % 3], iterators[(i + 1) % 3], face.neighbors[(i + 2) % 3]);
                  notify_neighbors_and_nodes(iterators[(i + 2) % 3]);

                  check(iterators[(i + 1) % 3]);
                  check(iterators[(i + 2) % 3]);
                  check(opposite_iterators[(j + 1) % 3]);
                  check(opposite_iterators[(j + 2) % 3]);

                  return;
               }
            }


            for (size_t i = 0; i != 3; ++i)
            {
               iterators[i]->set_neighbors(iterators[(i + 1) % 3], iterators[(i + 2) % 3], face.neighbors[i]);
               notify_neighbors_and_nodes(iterators[i]);
            }

            for (size_t i = 0; i != 3; ++i)
            {
               check(iterators[i]);
            }
         }

         node_iterator insert(point p, boost::optional<node_iterator> close_point)
         {
            nodes.push_back(my_node(p));

            if (size() == 2)
            {
               three_points();
            }
            else
            {
               if (size() > 2)
               {
                  auto face = localize(p, close_point);
                  insert_into_face(std::prev(nodes.end()), face);
               }
            }

            for (auto face : faces)
            {
               //face.print_all();
               face.print();
            }

            std::cerr << std::endl;


            return std::prev(nodes.end());
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
         return false;
         return distribution(generator) < P;
      }

      std::vector<layer> levels;

   public:

      triangulatable_points_set_2t() : levels(1)
      {
      }

      template <class InputIter>
      triangulatable_points_set_2t(InputIter p, InputIter q) : levels(1)
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
         //std::cerr << "Inserting " << p.x << " " << p.y << std::endl;
         std::vector<node_iterator> closest(levels.size());

         closest.back() = levels.back().find_closest(p, boost::none);

         if (closest.back()->p == p && !closest.back()->inf)
         {
            return false;
         }

         for (int level = static_cast<int>(levels.size()) - 2; level != -1; --level)
         {
            //std::cerr << "Localising on level " << level << std::endl;
            closest[level] = levels[level].find_closest(p, closest[level + 1]->prev_level_node);

            if (closest[level]->p == p && !closest[level]->inf)
            {
               return false;
            }
         }

         //std::cerr << "Level is " << 0 << std::endl;
         //std::cerr << "On this level the closest point is " << closest.front()->p.x << " " << closest.front()->p.y << std::endl;
         auto prev = levels.front().insert(p, closest.front());

         size_t level = 1;

         while (random_bool())
         {
            if (level == levels.size())
            {
               //std::cerr << "Level is " << level << std::endl;
               levels.push_back(layer());
               auto iter = levels.back().insert(p, boost::none);
               iter->prev_level_node = prev;
               break;
            }

            //std::cerr << "Level is " << level << std::endl;
            //std::cerr << "On this level the closest point is " << closest[level]->p.x << " " << closest[level]->p.y << std::endl;
            auto inserted = levels[level].insert(p, closest[level]);
            inserted->prev_level_node = prev;
            prev = inserted;
            ++level;
         }

         //std::cerr << "Inserted\nLevels = " << levels.size() << std::endl << std::endl;
         return true;
      }

      std::vector< triangle_2t<Scalar> > get_triangulation() const
      {
         return levels.front().get_triangulation();
      }

      boost::optional< triangle_2t<Scalar> > localize(const point & p)
      {
         std::vector<node_iterator> closest(levels.size());
         closest.back() = levels.back().find_closest(p, boost::none);

         for (int level = static_cast<int>(levels.size()) - 2; level != -1; --level)
         {
            closest[level] = levels[level].find_closest(p, closest[level + 1]->prev_level_node);
         }

         auto res = levels.front().localize(p, closest.front());

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

   template <class InputIter>
   std::vector< triangle_2t<typename std::iterator_traits<InputIter>::value_type::scalar_type> > delaunay_triangulation(InputIter p, InputIter q)
   {
      typedef typename std::iterator_traits<InputIter>::value_type::scalar_type Scalar;

      triangulatable_points_set_2t<Scalar> points(p, q);

      return points.get_triangulation();
   }

}
