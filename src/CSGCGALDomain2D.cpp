// Copyright (C) 2013-2014 Benjamin Kehlet
//
// This file is part of mshr.
//
// mshr is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// mshr is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with mshr.  If not, see <http://www.gnu.org/licenses/>.

// This file must be included to get the compiler flags
// They should idellay have been added via the command line,
// but since CGAL configure time is at mshr compile time, we don't
// have access to CGALConfig.cmake at configure time...
#include <CGAL/compiler_config.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Quotient.h>
#include <CGAL/MP_Float.h>

#include <mshr/CSGCGALDomain2D.h>
#include <mshr/CSGPrimitives2D.h>
#include <mshr/CSGOperators.h>

#include <dolfin/common/constants.h>
#include <dolfin/log/LogStream.h>

#include <CGAL/basic.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Polygon_set_2.h>

#include <CGAL/Min_circle_2.h>
#include <CGAL/Min_circle_2_traits_2.h>

// Polygon typedefs
//typedef CGAL::Exact_predicates_exact_constructions_kernel Exact_Kernel;
typedef CGAL::Quotient<CGAL::MP_Float>                    Quotient;
typedef CGAL::Cartesian<Quotient>                         Exact_Kernel;

typedef Exact_Kernel::Point_2                             Point_2;
typedef Exact_Kernel::Vector_2                            Vector_2;
typedef Exact_Kernel::Segment_2                           Segment_2;
typedef CGAL::Polygon_2<Exact_Kernel>                     Polygon_2;
typedef Polygon_2::Vertex_const_iterator                  Vertex_const_iterator;
typedef CGAL::Polygon_with_holes_2<Exact_Kernel>          Polygon_with_holes_2;
typedef Polygon_with_holes_2::Hole_const_iterator         Hole_const_iterator;
typedef CGAL::Polygon_set_2<Exact_Kernel>                 Polygon_set_2;

// Min enclosing circle typedefs
typedef CGAL::Min_circle_2_traits_2<Exact_Kernel>  Min_Circle_Traits;
typedef CGAL::Min_circle_2<Min_Circle_Traits>      Min_circle;
typedef CGAL::Circle_2<Exact_Kernel> CGAL_Circle;

namespace mshr
{

struct CSGCGALDomain2DImpl
{
  Polygon_set_2 polygon_set;

  CSGCGALDomain2DImpl(){}
  CSGCGALDomain2DImpl(const Polygon_set_2& p)
    : polygon_set(p) {}
};
//-----------------------------------------------------------------------------
Polygon_2 make_circle(const Circle* c)
{
  std::vector<Point_2> pts;
  pts.reserve(c->segments());

  for (std::size_t i = 0; i < c->segments(); i++)
  {
    const double phi = (2*DOLFIN_PI*i) / c->segments();
    const double x = c->center().x() + c->radius()*cos(phi);
    const double y = c->center().y() + c->radius()*sin(phi);
    pts.push_back(Point_2(x, y));
  }

  return Polygon_2(pts.begin(), pts.end());
}
//-----------------------------------------------------------------------------
Polygon_2 make_ellipse(const Ellipse* e)
{
  std::vector<Point_2> pts;

  for (std::size_t i = 0; i < e->segments(); i++)
  {
    const double phi = (2*DOLFIN_PI*i) / e->segments();
    const double x = e->center().x() + e->a()*cos(phi);
    const double y = e->center().y() + e->b()*sin(phi);
    pts.push_back(Point_2(x, y));
  }

  return Polygon_2(pts.begin(), pts.end());
}
//-----------------------------------------------------------------------------
Polygon_2 make_rectangle(const Rectangle* r)
{
  const double x0 = std::min(r->first_corner().x(), r->second_corner().x());
  const double y0 = std::min(r->first_corner().y(), r->second_corner().y());

  const double x1 = std::max(r->first_corner().x(), r->second_corner().x());
  const double y1 = std::max(r->first_corner().y(), r->second_corner().y());

  std::vector<Point_2> pts;
  pts.push_back(Point_2(x0, y0));
  pts.push_back(Point_2(x1, y0));
  pts.push_back(Point_2(x1, y1));
  pts.push_back(Point_2(x0, y1));

  Polygon_2 p(pts.begin(), pts.end());
  
  return p;
}
//-----------------------------------------------------------------------------
Polygon_2 make_polygon(const Polygon* p)
{
  std::vector<Point_2> pts;
  std::vector<dolfin::Point>::const_iterator v;
  for (v = p->vertices().begin(); v != p->vertices().end(); ++v)
    pts.push_back(Point_2(v->x(), v->y()));

  return Polygon_2(pts.begin(), pts.end());
}
//-----------------------------------------------------------------------------
std::unique_ptr<CSGCGALDomain2DImpl> do_transformation(const Polygon_set_2& p, Exact_Kernel::Aff_transformation_2 t)
{
  std::unique_ptr<CSGCGALDomain2DImpl> result(new CSGCGALDomain2DImpl);

  std::list<Polygon_with_holes_2> polygon_list;
  p.polygons_with_holes(std::back_inserter(polygon_list));

  std::list<Polygon_with_holes_2>::const_iterator pit;
  for (pit = polygon_list.begin(); pit != polygon_list.end(); ++pit)
  {
    const Polygon_with_holes_2& pwh = *pit;

    // Transform outer boundary
    Polygon_with_holes_2 transformed(CGAL::transform(t, pwh.outer_boundary()));

    // Transform holes
    for (Hole_const_iterator hit = pwh.holes_begin(); hit != pwh.holes_end(); hit++)
    {
      transformed.add_hole(CGAL::transform(t, *hit));
    }

    result->polygon_set.insert(transformed);
  }

  return result;
}
//-----------------------------------------------------------------------------
CSGCGALDomain2D::CSGCGALDomain2D()
  : impl(new CSGCGALDomain2DImpl)
{
  
}
//-----------------------------------------------------------------------------
CSGCGALDomain2D::~CSGCGALDomain2D()
{
}
//-----------------------------------------------------------------------------
CSGCGALDomain2D::CSGCGALDomain2D(const CSGGeometry *geometry)
: impl(new CSGCGALDomain2DImpl)
{

  if (geometry->dim() != 2)
    dolfin::dolfin_error("CSGCGALDomain2D.cpp",
                         "Creating polygonal domain",
                         "Geometry has dimension %d, expected 2", geometry->dim());


  switch (geometry->getType()) 
  {
    case CSGGeometry::Union:
    {
      const CSGUnion *u = dynamic_cast<const CSGUnion*>(geometry);
      dolfin_assert(u);

      CSGCGALDomain2D a(u->_g0.get());
      CSGCGALDomain2D b(u->_g1.get());

      impl.swap(a.impl);
      impl->polygon_set.join(b.impl->polygon_set);    
      break;
    }
    case CSGGeometry::Intersection:
    {
      const CSGIntersection* u = dynamic_cast<const CSGIntersection*>(geometry);
      dolfin_assert(u);

      CSGCGALDomain2D a(u->_g0.get());
      CSGCGALDomain2D b(u->_g1.get());
      
      impl.swap(a.impl);
      impl->polygon_set.intersection(b.impl->polygon_set);
      break;
    }
    case CSGGeometry::Difference:
    {
      const CSGDifference* u = dynamic_cast<const CSGDifference*>(geometry);
      dolfin_assert(u);
      CSGCGALDomain2D a(u->_g0.get());
      CSGCGALDomain2D b(u->_g1.get());
      
      impl.swap(a.impl);
      impl->polygon_set.difference(b.impl->polygon_set);
      break;
    }
    case CSGGeometry::Translation :
    {
      const CSGTranslation* t = dynamic_cast<const CSGTranslation*>(geometry);
      dolfin_assert(t);
      CSGCGALDomain2D a(t->g.get());
      Exact_Kernel::Aff_transformation_2 translation(CGAL::TRANSLATION, Vector_2(t->t.x(), t->t.y()));
      std::unique_ptr<CSGCGALDomain2DImpl> transformed = do_transformation(a.impl->polygon_set, translation);
      impl.swap(transformed);
      break;
    }
    case CSGGeometry::Scaling :
    {
      const CSGScaling* t = dynamic_cast<const CSGScaling*>(geometry);
      dolfin_assert(t);
      CSGCGALDomain2D a(t->g.get());
      Exact_Kernel::Aff_transformation_2 tr(CGAL::IDENTITY);

      // Translate if requested
      if (t->translate)
        tr = Exact_Kernel::Aff_transformation_2 (CGAL::TRANSLATION,
                                                 Vector_2(-t->c.x(), -t->c.y())) * tr;

      // Do the scaling
      tr = Exact_Kernel::Aff_transformation_2(CGAL::SCALING, t->s) * tr;

      if (t->translate)
        tr = Exact_Kernel::Aff_transformation_2(CGAL::TRANSLATION,
                                                Vector_2(t->c.x(), t->c.y())) * tr;

      std::unique_ptr<CSGCGALDomain2DImpl> transformed = do_transformation(a.impl->polygon_set,
                                                                           tr);
      impl.swap(transformed);
      break;
    }
    case CSGGeometry::Rotation :
    {
      const CSGRotation* t = dynamic_cast<const CSGRotation*>(geometry);
      dolfin_assert(t);
      CSGCGALDomain2D a(t->g.get());
      Exact_Kernel::Aff_transformation_2 tr(CGAL::IDENTITY);

      // Translate if requested
      if (t->translate)
        tr = Exact_Kernel::Aff_transformation_2 (CGAL::TRANSLATION,
                                                 Vector_2(-t->c.x(), -t->c.y())) * tr;

      // Do the rotation
      tr = Exact_Kernel::Aff_transformation_2(CGAL::ROTATION, sin(t->theta), cos(t->theta)) * tr;

      if (t->translate)
        tr = Exact_Kernel::Aff_transformation_2(CGAL::TRANSLATION,
                                                Vector_2(t->c.x(), t->c.y())) * tr;

      std::unique_ptr<CSGCGALDomain2DImpl> transformed = do_transformation(a.impl->polygon_set,
                                                                           tr);
      impl.swap(transformed);
      break;
    }
    case CSGGeometry::Circle:
    {
      const Circle* c = dynamic_cast<const Circle*>(geometry);
      dolfin_assert(c);
      impl->polygon_set.insert(make_circle(c));
      break;
    }
    case CSGGeometry::Ellipse:
    {
      const Ellipse* c = dynamic_cast<const Ellipse*>(geometry);
      dolfin_assert(c);
      impl->polygon_set.insert(make_ellipse(c));
      break;
    }
    case CSGGeometry::Rectangle:
    {
      const Rectangle* r = dynamic_cast<const Rectangle*>(geometry);
      dolfin_assert(r);
      impl->polygon_set.insert(make_rectangle(r));
      break;
    }
    case CSGGeometry::Polygon:
    {
      const Polygon* p = dynamic_cast<const Polygon*>(geometry);
      dolfin_assert(p);
      impl->polygon_set.insert(make_polygon(p));
      break;
    }
    default:
      dolfin::dolfin_error("CSGCGALMeshGenerator2D.cpp",
                           "converting geometry to cgal polyhedron",
                           "Unhandled primitive type");
  }
}
//-----------------------------------------------------------------------------
CSGCGALDomain2D::CSGCGALDomain2D(const CSGCGALDomain2D &other)
 : impl(new CSGCGALDomain2DImpl(other.impl->polygon_set))
{
}
//-----------------------------------------------------------------------------
CSGCGALDomain2D &CSGCGALDomain2D::operator=(const CSGCGALDomain2D &other)
{
  std::unique_ptr<CSGCGALDomain2DImpl> tmp(new CSGCGALDomain2DImpl(other.impl->polygon_set));
  
  impl.swap(tmp);

  return *this;
}
//-----------------------------------------------------------------------------
double CSGCGALDomain2D::compute_boundingcircle_radius() const
{
  std::list<Polygon_with_holes_2> polygon_list;
  impl->polygon_set.polygons_with_holes(std::back_inserter(polygon_list));

  std::vector<Point_2> points;

  for (std::list<Polygon_with_holes_2>::const_iterator pit = polygon_list.begin();
       pit != polygon_list.end(); ++pit)
    for (Polygon_2::Vertex_const_iterator vit = pit->outer_boundary().vertices_begin(); 
         vit != pit->outer_boundary().vertices_end(); ++vit)
      points.push_back(*vit);

  Min_circle min_circle (points.begin(),
                         points.end(),
                         true); //randomize point order

  return sqrt(CGAL::to_double(min_circle.circle().squared_radius()));
}
//-----------------------------------------------------------------------------
std::size_t CSGCGALDomain2D::num_polygons() const
{
  return impl->polygon_set.number_of_polygons_with_holes();
}
//-----------------------------------------------------------------------------
std::vector<dolfin::Point> CSGCGALDomain2D::get_outer_polygon(std::size_t i) const
{
  std::vector<Polygon_with_holes_2> polygon_list;
  impl->polygon_set.polygons_with_holes(std::back_inserter(polygon_list));

  const Polygon_2& polygon = polygon_list[i].outer_boundary();

  std::vector<dolfin::Point> res;
  res.reserve(polygon.size());
  for (std::size_t j = 0; j < polygon.size(); j++)
  {
    const Point_2& p = polygon.vertex(j);
    res.push_back(dolfin::Point(CGAL::to_double(p.x()), CGAL::to_double(p.y())));
  }

  return std::move(res);
}
//-----------------------------------------------------------------------------
void CSGCGALDomain2D::join_inplace(const CSGCGALDomain2D& other)
{
  impl->polygon_set.join(other.impl->polygon_set);
}
//-----------------------------------------------------------------------------
void CSGCGALDomain2D::difference_inplace(const CSGCGALDomain2D& other)
{
  impl->polygon_set.difference(other.impl->polygon_set);
}
//-----------------------------------------------------------------------------
void CSGCGALDomain2D::intersect_inplace(const CSGCGALDomain2D &other)
{
  impl->polygon_set.intersection(other.impl->polygon_set);
}
//-----------------------------------------------------------------------------
bool CSGCGALDomain2D::point_in_domain(dolfin::Point p) const
{
  const Point_2 p_(p.x(), p.y());
  return impl->polygon_set.oriented_side(p_) == CGAL::ON_POSITIVE_SIDE;
}
//-----------------------------------------------------------------------------
std::string CSGCGALDomain2D::str(bool verbose) const
{
  std::stringstream ss;
  const std::size_t num_polygons = impl->polygon_set.number_of_polygons_with_holes();
  ss << "<Polygonal domain with " << num_polygons
     << " outer polygon" << (num_polygons != 1 ? "s" : "") << std::endl;

  if (verbose)
  {
    std::list<Polygon_with_holes_2> polygon_list;
    impl->polygon_set.polygons_with_holes(std::back_inserter(polygon_list));
    for (const Polygon_with_holes_2& p : polygon_list)
    {
      ss << "  Polygon " << std::endl;
      const Polygon_2& bdr = p.outer_boundary();
      ss << "[" << bdr.size() << "] ";
      for (Polygon_2::Vertex_const_iterator vit = bdr.vertices_begin();
           vit != bdr.vertices_end(); vit++)
      {
        ss << "(" << CGAL::to_double(vit->x()) << ", " << CGAL::to_double(vit->y()) << ") ";
      }
      ss << std::endl;
    }
  }

  ss << ">";
  return ss.str();
}
//-----------------------------------------------------------------------------
static inline void add_simple_polygon(std::map<Point_2, std::size_t>& vertices,
                                      std::set<std::pair<std::size_t, std::size_t>>& segments,
                                      std::size_t& num_vertices,
                                      const Polygon_2& p,
                                      double truncate_tolerance)
{
  // NOTE: For now this function assumes that segment do not intersect except
  // exactly in the endpoints.
  // TODO: Make it more flexible and search for endpoints with a tolerance

  typedef std::map<Point_2, std::size_t>::iterator Iterator;

  // std::cout << "Adding simple polygon" << std::endl;

  std::size_t offset = 0;
  while (p.edge(offset).squared_length() < truncate_tolerance)
  {
    offset++;
    dolfin_assert(offset < p.size());
  }

  Point_2 prev = p.vertex(offset);
  offset++;

  for (std::size_t i = 0; i < p.size(); i++)
  {
    Point_2 current = p.vertex((i+offset)%p.size());
    Segment_2 s(prev, current);

    if (s.squared_length() < truncate_tolerance)
    {
      // std::cout << "Short edge (" << s << ") skipped" << std::endl;

      // Insert both points into the vertex map, pointing to the same vertex

      // Check if vertex exists
      Iterator sit = vertices.find(s.source());
      if (sit != vertices.end())
      {
        // source point exists in vertex_map. Insert the target point with the same vertex number
        const std::size_t vertex_number = sit->second;;

        /* std::pair<Iterator, bool> i = */ vertices.insert(std::make_pair(s.target(), vertex_number));
        //dolfin_assert(!i.second);
      }
      else
      {
        Iterator tit = vertices.find(s.target());
        if (tit != vertices.end())
        {
          // target points exists in vertex_map. Insert the source point with the same vertex number
          const std::size_t vertex_number = tit->second;
          /* std::pair<Iterator, bool> i = */ vertices.insert(std::make_pair(s.source(), vertex_number));
          //dolfin_assert(i.second);
        }
        else
        {
          // No points exists in the vertex_map.
          // Insert both
          const std::size_t vertex_number = num_vertices;
          /* std::pair<Iterator, bool> i = */ vertices.insert(std::make_pair(s.source(), vertex_number));
          /* std::pair<Iterator, bool> j = */ vertices.insert(std::make_pair(s.target(), vertex_number));
          num_vertices++;
        }
      }
    }
    else
    {
      // std::cout << "  Segment: " << s << " (" << sqrt(CGAL::to_double(s.squared_length())) << ")" << std::endl;

      Iterator sit = vertices.find(s.source());
      if (sit == vertices.end())
      {
        std::pair<Iterator, bool> i = vertices.insert(std::make_pair(s.source(), num_vertices));
        dolfin_assert(i.second);
        sit = i.first;
        num_vertices++;
      }
      // std::cout << "Point " << sit->first << " is " << sit->second << std::endl;

      Iterator tit = vertices.find(s.target());
      if (tit == vertices.end())
      {
        /* const std::size_t vertex_number = vertices.size(); */
        std::pair<Iterator, bool> i = vertices.insert(std::make_pair(s.target(), num_vertices));
        dolfin_assert(i.second);
        tit = i.first;
        num_vertices++;
      }
      // std::cout << "Point " << tit->first << " is " << tit->second << std::endl;

      if (segments.count(std::make_pair(tit->second, sit->second)) == 0)
        segments.insert(std::make_pair(sit->second, tit->second));

      prev = current;
    }
  }
}
//-----------------------------------------------------------------------------
std::pair<std::vector<dolfin::Point>, std::vector<std::pair<std::size_t, std::size_t>>>
  CSGCGALDomain2D::compute_pslg(const std::vector<std::pair<std::size_t, CSGCGALDomain2D>>& domains,
                   double truncate_tolerance)
{
  // std::cout << "compute_pslg(), truncate_tolerance " << truncate_tolerance << std::endl;

  std::size_t num_vertices = 0;
  std::map<Point_2, std::size_t> vertices;
  std::set<std::pair<std::size_t, std::size_t>> segments;

  for (const std::pair<std::size_t, CSGCGALDomain2D>& domain : domains)
  {
    // std::cout << "PSLG: Adding domain: " << domain.first << std::endl;
    const Polygon_set_2& p = domain.second.impl->polygon_set;

    std::list<Polygon_with_holes_2> polygon_list;
    p.polygons_with_holes(std::back_inserter(polygon_list));

    for (const Polygon_with_holes_2& pwh : polygon_list)
    {
      // std::cout << "  Adding polygon" << std::endl;
      add_simple_polygon(vertices,
                         segments,
                         num_vertices,
                         pwh.outer_boundary(),
                         truncate_tolerance);

      // Add holes
      Hole_const_iterator hit;
      for (hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit)
      {
        add_simple_polygon(vertices,
                           segments,
                           num_vertices,
                           *hit,
                           truncate_tolerance);
      }
    }
  }

  std::vector<dolfin::Point> v(num_vertices);
  // std::set<std::size_t> v_inserted;
  for (const std::pair<Point_2, std::size_t>& vertex : vertices)
  {
    v[vertex.second] = dolfin::Point(CGAL::to_double(vertex.first.x()), CGAL::to_double(vertex.first.y()));
  }

  std::vector<std::pair<std::size_t, std::size_t>> s(segments.begin(), segments.end());

  return std::make_pair(std::move(v), std::move(s));
}
//-----------------------------------------------------------------------------
}
