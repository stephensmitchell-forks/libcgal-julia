#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/variant/apply_visitor.hpp>

#include <CGAL/Circular_kernel_2/Intersection_traits.h>
#include <CGAL/Kernel/global_functions.h>

#include <jlcxx/module.hpp>

#include <julia.h>

#include "kernel.hpp"
#include "kernel_conversion.hpp"
#include "macros.hpp"

#define DO_INTERSECT(T1, T2) \
  cgal.SPFUNC(, do_intersect, T1, T2); \
  cgal.SPFUNC(, do_intersect, T2, T1)
#define DO_INTERSECT_SELF(T) \
  cgal.SPFUNC(, do_intersect, T, T)

#define CK_DO_INTERSECT_CONVERT(T1, T2, C1, C2) \
  cgal.PFUNC(, do_intersect, ck_do_intersect, T1, T2, C1, C2); \
  cgal.PFUNC(, do_intersect, ck_do_intersect, T2, T1, C2, C1)
#define CK_DO_INTERSECT(T1, T2) CK_DO_INTERSECT_CONVERT(T1, T2, CK::T1, CK::T2)
#define CK_DO_INTERSECT_SELF(T) \
  cgal.PFUNC(, do_intersect, ck_do_intersect, T, T, CK::T, CK::T)

#define INTERSECTION(T1, T2) \
  cgal.SPFUNC(, intersection, T1, T2); \
  cgal.SPFUNC(, intersection, T2, T1)
#define INTERSECTION_SELF(T) cgal.SPFUNC(, intersection, T, T)

#define CK_INTERSECTION(T1, T2) \
  cgal.PFUNC(, intersection, ck_intersection, T1, T2, CK::T1, CK::T2); \
  cgal.PFUNC(, intersection, ck_intersection, T2, T1, CK::T2, CK::T1)
#define CK_INTERSECTION_SELF(T) \
  cgal.PFUNC(, intersection, ck_intersection, T, T, CK::T, CK::T)

#define CK_INTERSECTION_CONVERT(T1, T2, C1, C2) \
  cgal.PFUNC(, intersection, ck_intersection, T1, T2, C1, C2); \
  cgal.PFUNC(, intersection, ck_intersection, T2, T1, C2, C1)

#define SQUARED_DISTANCE_2(T) \
  cgal.SPFUNC(, squared_distance, T, Point_2); \
  cgal.SPFUNC(, squared_distance, T, Line_2); \
  cgal.SPFUNC(, squared_distance, T, Ray_2); \
  cgal.SPFUNC(, squared_distance, T, Segment_2); \
  cgal.SPFUNC(, squared_distance, T, Triangle_2)

#define SQUARED_DISTANCE_3(T) \
  cgal.SPFUNC(, squared_distance, T, Point_3); \
  cgal.SPFUNC(, squared_distance, T, Line_3); \
  cgal.SPFUNC(, squared_distance, T, Ray_3); \
  cgal.SPFUNC(, squared_distance, T, Segment_3); \
  cgal.SPFUNC(, squared_distance, T, Plane_3)

typedef Circular_kernel CK;

struct Intersection_visitor {
  typedef jl_value_t* result_type;

  template<typename T>
  inline
  result_type
  operator()(const T& t) const {
    return jlcxx::box<T>(t);
  }

  inline
  template<typename... TS>
  result_type
  operator()(const boost::variant<TS...>& v) const {
    return boost::apply_visitor(*this, v);
  }

  template<typename T>
  result_type operator()(const std::vector<T>& ts) const {
    if (ts.empty()) {
      return jl_nothing;
    }

    const std::size_t sz = ts.size();
    result_type first = (*this)(ts[0]);

    if (sz == 1) {
      return first;
    }

    jl_value_t* atype = jl_apply_array_type(jl_typeof(first), 1);
    jl_array_t* ja = jl_alloc_array_1d(atype, sz);

    JL_GC_PUSH1(&ja);
    for (std::size_t i = 0; i < sz; ++i) {
      jl_arrayset(ja, (*this)(ts[i]), i);
    }
    JL_GC_POP();

    return (result_type)ja;
  }

  // Circular results
  inline
  result_type
  operator()(const std::pair<CK::Circular_arc_point_2, unsigned>& p) const {
    return jlcxx::box<Point_2>(To_linear<CK::Circular_arc_point_2>()(p.first));
  }

  inline
  result_type
  operator()(const CK::Circle_2& c) const {
    return jlcxx::box<Circle_2>(To_linear<CK::Circle_2>()(c));
  }

  result_type
  operator()(const CK::Circle_2& cc) const {
    return jlcxx::box<Circle_2>(To_linear<CK::Circle_2>()(cc));
  }
};

template<typename T1, typename T2>
inline
bool
do_intersect(const T1& t1, const T2& t2) {
  return CGAL::do_intersect(t1, t2);
}

template<typename T1, typename T2, typename C1, typename C2>
inline
bool
ck_do_intersect(const T1& t1, const T2& t2) {
  C1 c1 = To_circular<C1>()(t1);
  C2 c2 = To_circular<C2>()(t2);
  return CGAL::do_intersect(c1, c2);
}

template<typename T1, typename T2>
inline
jl_value_t*
intersection(const T1& t1, const T2& t2) {
  auto result = CGAL::intersection(t1, t2);
  return result ?
    boost::apply_visitor(Intersection_visitor(), *result) :
    jl_nothing;
}

template<typename T1, typename T2, typename C1, typename C2>
inline
jl_value_t*
ck_intersection(const T1& t1, const T2& t2) {
  typedef typename CGAL::CK2_Intersection_traits<CK, C1, C2>::type ResultT;

  C1 c1 = To_circular<C1>()(t1);
  C2 c2 = To_circular<C2>()(t2);
  std::vector<ResultT> res;
  CGAL::intersection(c1, c2, std::back_inserter(res));
  return boost::apply_visitor(Intersection_visitor(),
                              boost::variant<std::vector<ResultT>>(res));
}

template<typename T1, typename T2>
inline
FT
squared_distance(const T1& t1, const T2& t2) {
  return CGAL::squared_distance(t1, t2);
}

void wrap_global_kernel_functions(jlcxx::Module& cgal) {
  OVERRIDE_BASE(cgal,);
  CGAL_GLOBAL_FUNCTION(CGAL::Angle, angle, const Vector_2&, const Vector_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Angle, angle, const Point_2&,  const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Angle, angle, const Point_2&,  const Point_2&, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Angle, angle, const Vector_3&, const Vector_3&);
  CGAL_GLOBAL_FUNCTION(CGAL::Angle, angle, const Point_3&,  const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(CGAL::Angle, angle, const Point_3&,  const Point_3&, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(CGAL::Angle, angle, const Point_3&,  const Point_3&, const Point_3&, const Vector_3&);
  UNSET_OVERRIDE(cgal,);

  CGAL_GLOBAL_FUNCTION(FT, approximate_angle, const Point_3&, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(FT, approximate_angle, const Vector_3&, const Vector_3&);

  CGAL_GLOBAL_FUNCTION(FT, approximate_dihedral_angle, const Point_3&, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(FT, area, const Point_2&, const Point_2&, const Point_2&);

  CGAL_GLOBAL_FUNCTION(bool, are_ordered_along_line, const Point_2&, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(bool, are_ordered_along_line, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(bool, are_strictly_ordered_along_line, const Point_2&, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(bool, are_strictly_ordered_along_line, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(Point_2, barycenter, const Point_2&, const FT&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(Point_2, barycenter, const Point_2&, const FT&, const Point_2&, const FT&);
  CGAL_GLOBAL_FUNCTION(Point_2, barycenter, const Point_2&, const FT&, const Point_2&, const FT&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(Point_2, barycenter, const Point_2&, const FT&, const Point_2&, const FT&, const Point_2&, const FT&);
  CGAL_GLOBAL_FUNCTION(Point_2, barycenter, const Point_2&, const FT&, const Point_2&, const FT&, const Point_2&, const FT&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(Point_2, barycenter, const Point_2&, const FT&, const Point_2&, const FT&, const Point_2&, const FT&, const Point_2&, const FT&);
  CGAL_GLOBAL_FUNCTION(Point_3, barycenter, const Point_3&, const FT&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(Point_3, barycenter, const Point_3&, const FT&, const Point_3&, const FT&);
  CGAL_GLOBAL_FUNCTION(Point_3, barycenter, const Point_3&, const FT&, const Point_3&, const FT&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(Point_3, barycenter, const Point_3&, const FT&, const Point_3&, const FT&, const Point_3&, const FT&);
  CGAL_GLOBAL_FUNCTION(Point_3, barycenter, const Point_3&, const FT&, const Point_3&, const FT&, const Point_3&, const FT&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(Point_3, barycenter, const Point_3&, const FT&, const Point_3&, const FT&, const Point_3&, const FT&, const Point_3&, const FT&);

  CGAL_GLOBAL_FUNCTION(Line_2, bisector, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(Line_2, bisector, const Line_2&,  const Line_2&);
  CGAL_GLOBAL_FUNCTION(Plane_3, bisector, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(Plane_3, bisector, const Plane_3&,  const Plane_3&);

  CGAL_GLOBAL_FUNCTION(Point_2, centroid, const Point_2&, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(Point_2, centroid, const Point_2&, const Point_2&, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(Point_2, centroid, const Triangle_2&);
  CGAL_GLOBAL_FUNCTION(Point_3, centroid, const Point_3&, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(Point_3, centroid, const Point_3&, const Point_3&, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(Point_3, centroid, const Triangle_3&);
  CGAL_GLOBAL_FUNCTION(Point_3, centroid, const Tetrahedron_3&);

  CGAL_GLOBAL_FUNCTION(Point_2, circumcenter, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(Point_2, circumcenter, const Point_2&, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(Point_2, circumcenter, const Triangle_2&);
  CGAL_GLOBAL_FUNCTION(Point_3, circumcenter, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(Point_3, circumcenter, const Point_3&, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(Point_3, circumcenter, const Point_3&, const Point_3&, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(Point_3, circumcenter, const Triangle_3&);
  CGAL_GLOBAL_FUNCTION(Point_3, circumcenter, const Tetrahedron_3&);

  CGAL_GLOBAL_FUNCTION(bool, collinear_are_ordered_along_line, const Point_2&, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(bool, collinear_are_ordered_along_line, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(bool, collinear_are_strictly_ordered_along_line, const Point_2&, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(bool, collinear_are_strictly_ordered_along_line, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(bool, collinear, const Point_2&, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(bool, collinear, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_dihedral_angle, const Point_3&, const Point_3&, const Point_3&, const Point_3&, const FT&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_dihedral_angle, const Point_3&, const Point_3&, const Point_3&, const Point_3&, const Point_3&, const Point_3&, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_dihedral_angle, const Vector_3&, const Vector_3&, const Vector_3&, const FT&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_dihedral_angle, const Vector_3&, const Vector_3&, const Vector_3&, const Vector_3&, const Vector_3&, const Vector_3&);

  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_distance_to_point, const Point_2&, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_distance_to_point, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_lexicographically, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_lexicographically, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_signed_distance_to_line, const Line_2&,  const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_signed_distance_to_line, const Point_2&, const Point_2&, const Point_2&, const Point_2&);

  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_signed_distance_to_plane, const Plane_3&,  const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_signed_distance_to_plane, const Point_3&, const Point_3&, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_slope, const Line_2&,    const Line_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_slope, const Segment_2&, const Segment_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_slope, const Point_3&, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_squared_distance, const Point_2&, const Point_2&, const FT&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_squared_distance, const Point_3&, const Point_3&, const FT&);

  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_squared_radius, const Point_3&, const FT&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_squared_radius, const Point_3&, const Point_3&, const FT&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_squared_radius, const Point_3&, const Point_3&, const Point_3&, const FT&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_squared_radius, const Point_3&, const Point_3&, const Point_3&, const Point_3&, const FT&);

  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_x, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_x, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_x, const Point_2&, const Line_2&, const Line_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_x, const Line_2&,  const Line_2&, const Line_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_x, const Line_2&,  const Line_2&, const Line_2&, const Line_2&);

  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_xy, const Point_2&, const Point_2&);

  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_x_at_y, const Point_2&, const Line_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_x_at_y, const Point_2&, const Line_2&, const Line_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_x_at_y, const Line_2&,  const Line_2&, const Line_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_x_at_y, const Line_2&,  const Line_2&, const Line_2&, const Line_2&);

  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_y_at_x, const Point_2&, const Line_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_y_at_x, const Point_2&, const Line_2&, const Line_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_y_at_x, const Line_2&,  const Line_2&, const Line_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_y_at_x, const Line_2&,  const Line_2&, const Line_2&, const Line_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_y_at_x, const Point_2&, const Segment_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_y_at_x, const Point_2&, const Segment_2&, const Segment_2&);

  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_y, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_y, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_y, const Point_2&, const Line_2&, const Line_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_y, const Line_2&,  const Line_2&, const Line_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_y, const Line_2&,  const Line_2&, const Line_2&, const Line_2&);

  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_xyz, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_z, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare_yx, const Point_2&, const Point_2&);

  CGAL_GLOBAL_FUNCTION(bool, coplanar, const Point_3&, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(CGAL::Orientation, coplanar_orientation, const Point_3&, const Point_3&, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(CGAL::Orientation, coplanar_orientation, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(CGAL::Bounded_side, coplanar_side_of_bounded_circle, const Point_3&, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(Vector_3, cross_product, const Vector_3&, const Vector_3&);

  CGAL_GLOBAL_FUNCTION(FT, determinant, const Vector_2&, const Vector_2&);
  CGAL_GLOBAL_FUNCTION(FT, determinant, const Vector_3&, const Vector_3&, const Vector_3&);

  DO_INTERSECT(Bbox_2, Circle_2);
  DO_INTERSECT(Bbox_2, Line_2);
  DO_INTERSECT(Bbox_2, Point_2);
  DO_INTERSECT(Bbox_2, Ray_2);
  DO_INTERSECT_SELF(Circle_2);
  DO_INTERSECT(Circle_2, Iso_rectangle_2);
  DO_INTERSECT(Circle_2, Line_2);
  DO_INTERSECT(Circle_2, Point_2);
  DO_INTERSECT_SELF(Iso_rectangle_2);
  DO_INTERSECT(Iso_rectangle_2, Line_2);
  DO_INTERSECT(Iso_rectangle_2, Point_2);
  DO_INTERSECT(Iso_rectangle_2, Ray_2);
  DO_INTERSECT(Iso_rectangle_2, Segment_2);
  DO_INTERSECT(Iso_rectangle_2, Triangle_2);
  DO_INTERSECT_SELF(Line_2);
  DO_INTERSECT(Line_2, Point_2);
  DO_INTERSECT(Line_2, Ray_2);
  DO_INTERSECT(Line_2, Segment_2);
  DO_INTERSECT(Line_2, Triangle_2);
  DO_INTERSECT_SELF(Point_2);
  DO_INTERSECT(Point_2, Ray_2);
  DO_INTERSECT(Point_2, Segment_2);
  DO_INTERSECT(Point_2, Triangle_2);
  DO_INTERSECT_SELF(Ray_2);
  DO_INTERSECT(Ray_2, Segment_2);
  DO_INTERSECT(Ray_2, Triangle_2);
  DO_INTERSECT_SELF(Segment_2);
  DO_INTERSECT(Segment_2, Triangle_2);
  DO_INTERSECT_SELF(Triangle_2);

  CK_DO_INTERSECT(Circle_2, Circular_arc_2);
  CK_DO_INTERSECT_CONVERT(Circle_2, Segment_2, CK::Circle_2, CK::Line_arc_2);
  CK_DO_INTERSECT_SELF(Circular_arc_2);
  CK_DO_INTERSECT(Circular_arc_2, Line_2);
  CK_DO_INTERSECT_CONVERT(Circular_arc_2, Segment_2, CK::Circular_arc_2, CK::Line_arc_2);

  DO_INTERSECT_SELF(Bbox_3);
  DO_INTERSECT(Bbox_3, Iso_cuboid_3);
  DO_INTERSECT(Bbox_3, Line_3);
  DO_INTERSECT(Bbox_3, Plane_3);
  DO_INTERSECT(Bbox_3, Point_3);
  DO_INTERSECT(Bbox_3, Ray_3);
  DO_INTERSECT(Bbox_3, Segment_3);
  DO_INTERSECT(Bbox_3, Sphere_3);
  DO_INTERSECT(Bbox_3, Tetrahedron_3);
  DO_INTERSECT(Bbox_3, Triangle_3);
  DO_INTERSECT_SELF(Iso_cuboid_3);
  DO_INTERSECT(Iso_cuboid_3, Line_3);
  DO_INTERSECT(Iso_cuboid_3, Plane_3);
  DO_INTERSECT(Iso_cuboid_3, Point_3);
  DO_INTERSECT(Iso_cuboid_3, Ray_3);
  DO_INTERSECT(Iso_cuboid_3, Segment_3);
  DO_INTERSECT(Iso_cuboid_3, Sphere_3);
  DO_INTERSECT(Iso_cuboid_3, Tetrahedron_3);
  DO_INTERSECT(Iso_cuboid_3, Triangle_3);
  DO_INTERSECT_SELF(Line_3);
  DO_INTERSECT(Line_3, Plane_3);
  DO_INTERSECT(Line_3, Point_3);
  DO_INTERSECT(Line_3, Ray_3);
  DO_INTERSECT(Line_3, Segment_3);
  DO_INTERSECT(Line_3, Sphere_3);
  DO_INTERSECT(Line_3, Tetrahedron_3);
  DO_INTERSECT(Line_3, Triangle_3);
  DO_INTERSECT_SELF(Plane_3);
  DO_INTERSECT(Plane_3, Point_3);
  DO_INTERSECT(Plane_3, Ray_3);
  DO_INTERSECT(Plane_3, Segment_3);
  DO_INTERSECT(Plane_3, Sphere_3);
  DO_INTERSECT(Plane_3, Tetrahedron_3);
  DO_INTERSECT(Plane_3, Triangle_3);
  DO_INTERSECT_SELF(Point_3);
  DO_INTERSECT(Point_3, Ray_3);
  DO_INTERSECT(Point_3, Segment_3);
  DO_INTERSECT(Point_3, Sphere_3);
  DO_INTERSECT(Point_3, Tetrahedron_3);
  DO_INTERSECT(Point_3, Triangle_3);
  DO_INTERSECT_SELF(Ray_3);
  DO_INTERSECT(Ray_3, Segment_3);
  DO_INTERSECT(Ray_3, Sphere_3);
  DO_INTERSECT(Ray_3, Tetrahedron_3);
  DO_INTERSECT(Ray_3, Triangle_3);
  DO_INTERSECT_SELF(Segment_3);
  DO_INTERSECT(Segment_3, Sphere_3);
  DO_INTERSECT(Segment_3, Tetrahedron_3);
  DO_INTERSECT(Segment_3, Triangle_3);
  DO_INTERSECT_SELF(Sphere_3);
  DO_INTERSECT(Sphere_3, Tetrahedron_3);
  DO_INTERSECT(Sphere_3, Triangle_3);
  DO_INTERSECT_SELF(Tetrahedron_3);
  DO_INTERSECT(Tetrahedron_3, Triangle_3);
  DO_INTERSECT_SELF(Triangle_3);

  CGAL_GLOBAL_FUNCTION(bool, has_larger_distance_to_point, const Point_2&, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(bool, has_larger_distance_to_point, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(bool, has_larger_signed_distance_to_line, const Line_2&,  const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(bool, has_larger_signed_distance_to_line, const Point_2&, const Point_2&, const Point_2&, const Point_2&);

  CGAL_GLOBAL_FUNCTION(bool, has_larger_signed_distance_to_plane, const Plane_3&,  const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(bool, has_larger_signed_distance_to_plane, const Point_3&, const Point_3&, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(bool, has_smaller_distance_to_point, const Point_2&, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(bool, has_smaller_distance_to_point, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(bool, has_smaller_signed_distance_to_line, const Line_2&,  const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(bool, has_smaller_signed_distance_to_line, const Point_2&, const Point_2&, const Point_2&, const Point_2&);

  CGAL_GLOBAL_FUNCTION(bool, has_smaller_signed_distance_to_plane, const Plane_3&,  const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(bool, has_smaller_signed_distance_to_plane, const Point_3&, const Point_3&, const Point_3&, const Point_3&, const Point_3&);

  INTERSECTION_SELF(Iso_rectangle_2);
  INTERSECTION(Iso_rectangle_2, Line_2);
  INTERSECTION(Iso_rectangle_2, Ray_2);
  INTERSECTION(Iso_rectangle_2, Segment_2);
  INTERSECTION(Iso_rectangle_2, Triangle_2);
  INTERSECTION_SELF(Line_2);
  INTERSECTION(Line_2, Ray_2);
  INTERSECTION(Line_2, Segment_2);
  INTERSECTION(Line_2, Triangle_2);
  INTERSECTION_SELF(Ray_2);
  INTERSECTION(Ray_2, Segment_2);
  INTERSECTION(Ray_2, Triangle_2);
  INTERSECTION_SELF(Segment_2);
  INTERSECTION(Segment_2, Triangle_2);
  INTERSECTION_SELF(Triangle_2);

  CK_INTERSECTION_SELF(Circle_2);
  CK_INTERSECTION(Circle_2, Circular_arc_2);
  CK_INTERSECTION(Circle_2, Line_2);
  CK_INTERSECTION_CONVERT(Circle_2, Segment_2, CK::Circle_2, CK::Line_arc_2);
  CK_INTERSECTION_SELF(Circular_arc_2);
  CK_INTERSECTION(Circular_arc_2, Line_2);
  CK_INTERSECTION_CONVERT(Circular_arc_2, Segment_2, CK::Circular_arc_2, CK::Line_arc_2);

  INTERSECTION_SELF(Point_3);
  INTERSECTION(Point_3, Line_3);
  INTERSECTION(Point_3, Plane_3);
  INTERSECTION(Point_3, Ray_3);
  INTERSECTION(Point_3, Segment_3);
  INTERSECTION(Point_3, Sphere_3);
  INTERSECTION(Point_3, Triangle_3);
  INTERSECTION_SELF(Line_3);
  INTERSECTION(Line_3, Plane_3);
  INTERSECTION(Line_3, Ray_3);
  INTERSECTION(Line_3, Segment_3);
  INTERSECTION(Line_3, Triangle_3);
  INTERSECTION_SELF(Plane_3);
  INTERSECTION(Plane_3, Ray_3);
  INTERSECTION(Plane_3, Segment_3);
  INTERSECTION(Plane_3, Sphere_3);
  INTERSECTION(Plane_3, Triangle_3);
  INTERSECTION_SELF(Ray_3);
  INTERSECTION(Ray_3, Triangle_3);
  INTERSECTION_SELF(Segment_3);
  INTERSECTION(Segment_3, Triangle_3);
  INTERSECTION_SELF(Sphere_3);
  INTERSECTION_SELF(Triangle_3);

  CGAL_GLOBAL_FUNCTION(FT, l_infinity_distance, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(FT, l_infinity_distance, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(bool, left_turn, const Point_2&, const Point_2&, const Point_2&);

  CGAL_GLOBAL_FUNCTION(bool, lexicographically_xy_larger, const Point_2&, const Point_2&);

  CGAL_GLOBAL_FUNCTION(bool, lexicographically_xy_larger_or_equal, const Point_2&, const Point_2&);

  CGAL_GLOBAL_FUNCTION(bool, lexicographically_xy_smaller, const Point_2&, const Point_2&);

  CGAL_GLOBAL_FUNCTION(bool, lexicographically_xy_smaller_or_equal, const Point_2&, const Point_2&);

  CGAL_GLOBAL_FUNCTION(bool, lexicographically_xyz_smaller, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(bool, lexicographically_xyz_smaller_or_equal, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(Point_2, max_vertex, const Iso_rectangle_2&);
  CGAL_GLOBAL_FUNCTION(Point_3, max_vertex, const Iso_cuboid_3&);

  CGAL_GLOBAL_FUNCTION(Point_2, midpoint, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(Point_3, midpoint, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(Point_2, min_vertex, const Iso_rectangle_2&);
  CGAL_GLOBAL_FUNCTION(Point_3, min_vertex, const Iso_cuboid_3&);

  CGAL_GLOBAL_FUNCTION(Vector_3, normal, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(CGAL::Orientation, orientation, const Point_2&,  const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Orientation, orientation, const Vector_2&, const Vector_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Orientation, orientation, const Point_3&,  const Point_3&, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(CGAL::Orientation, orientation, const Vector_3&, const Vector_3&, const Vector_3&);

  CGAL_GLOBAL_FUNCTION(Vector_3, orthogonal_vector, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(bool, parallel, const Line_2&, const Line_2&);
  CGAL_GLOBAL_FUNCTION(bool, parallel, const Ray_2&, const Ray_2&);
  CGAL_GLOBAL_FUNCTION(bool, parallel, const Segment_2&, const Segment_2&);
  CGAL_GLOBAL_FUNCTION(bool, parallel, const Line_3&, const Line_3&);
  CGAL_GLOBAL_FUNCTION(bool, parallel, const Plane_3&, const Plane_3&);
  CGAL_GLOBAL_FUNCTION(bool, parallel, const Ray_3&, const Ray_3&);
  CGAL_GLOBAL_FUNCTION(bool, parallel, const Segment_3&, const Segment_3&);

  CGAL_GLOBAL_FUNCTION(Plane_3, radical_plane, const Sphere_3&, const Sphere_3&);

  CGAL_GLOBAL_FUNCTION(Line_2, radical_line, const Circle_2&, const Circle_2&);

  CGAL_GLOBAL_FUNCTION(void, rational_rotation_approximation, const RT&, const RT&, RT&, RT&, RT&, const RT&, const RT&);

  CGAL_GLOBAL_FUNCTION(bool, right_turn, const Point_2&, const Point_2&, const Point_2&);

  CGAL_GLOBAL_FUNCTION(FT, scalar_product, const Vector_2&, const Vector_2&);
  CGAL_GLOBAL_FUNCTION(FT, scalar_product, const Vector_3&, const Vector_3&);

  CGAL_GLOBAL_FUNCTION(CGAL::Bounded_side, side_of_bounded_circle, const Point_2&, const Point_2&, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(CGAL::Bounded_side, side_of_bounded_circle, const Point_2&, const Point_2&, const Point_2&);

  CGAL_GLOBAL_FUNCTION(CGAL::Bounded_side, side_of_bounded_sphere, const Point_3&, const Point_3&, const Point_3&, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(CGAL::Bounded_side, side_of_bounded_sphere, const Point_3&, const Point_3&, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(CGAL::Bounded_side, side_of_bounded_sphere, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(CGAL::Oriented_side, side_of_oriented_circle, const Point_2&, const Point_2&, const Point_2&, const Point_2&);

  CGAL_GLOBAL_FUNCTION(CGAL::Oriented_side, side_of_oriented_sphere, const Point_3&, const Point_3&, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(FT, squared_area, const Point_3&, const Point_3&, const Point_3&);

  SQUARED_DISTANCE_2(Point_2);
  SQUARED_DISTANCE_2(Line_2);
  SQUARED_DISTANCE_2(Ray_2);
  SQUARED_DISTANCE_2(Segment_2);
  SQUARED_DISTANCE_2(Triangle_2);

  SQUARED_DISTANCE_3(Point_3);
  SQUARED_DISTANCE_3(Line_3);
  SQUARED_DISTANCE_3(Ray_3);
  SQUARED_DISTANCE_3(Segment_3);
  SQUARED_DISTANCE_3(Plane_3);
  cgal.SPFUNC(, squared_distance, const Point_3&, const Triangle_3&);
  cgal.SPFUNC(, squared_distance, const Triangle_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(FT, squared_radius, const Point_2&, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(FT, squared_radius, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(FT, squared_radius, const Point_2&);
  CGAL_GLOBAL_FUNCTION(FT, squared_radius, const Point_3&, const Point_3&, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(FT, squared_radius, const Point_3&, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(FT, squared_radius, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(FT, squared_radius, const Point_3&);

  CGAL_GLOBAL_FUNCTION(Vector_3, unit_normal, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(FT, volume, const Point_3&, const Point_3&, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(bool, x_equal, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(bool, x_equal, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(bool, y_equal, const Point_2&, const Point_2&);
  CGAL_GLOBAL_FUNCTION(bool, y_equal, const Point_3&, const Point_3&);
  CGAL_GLOBAL_FUNCTION(bool, z_equal, const Point_3&, const Point_3&);

  CGAL_GLOBAL_FUNCTION(bool, do_overlap, const Bbox_2&, const Bbox_2&);
  CGAL_GLOBAL_FUNCTION(bool, do_overlap, const Bbox_3&, const Bbox_3&);
}

#undef DO_INTERSECT
#undef DO_INTERSECT_SELF

#undef CK_DO_INTERSECT_CONVERT
#undef CK_DO_INTERSECT
#undef CK_DO_INTERSECT_SELF

#undef INTERSECTION
#undef INTERSECTION_SELF

#undef CK_INTERSECTION_CONVERT
#undef CK_INTERSECTION
#undef CK_INTERSECTION_SELF

#undef SQUARED_DISTANCE
