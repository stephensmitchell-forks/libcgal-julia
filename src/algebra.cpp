#include <CGAL/number_utils.h>

#include <jlcxx/module.hpp>

#include "macros.hpp"
#include "kernel.hpp"

using NT = K::FT;

void wrap_algebra(jlcxx::Module& cgal) {
  // missing functions that involve EuclideanRing concept:
  // div, div_mod, mod
  OVERRIDE_BASE(cgal,);
  CGAL_GLOBAL_FUNCTION(NT, abs, const NT&);
  CGAL_GLOBAL_FUNCTION(CGAL::Sign, sign, const NT&);
  CGAL_GLOBAL_FUNCTION(NT, sqrt, const NT&);
  UNSET_OVERRIDE(cgal,);
  CGAL_GLOBAL_FUNCTION(CGAL::Comparison_result, compare, const NT&, const NT&);
  CGAL_GLOBAL_FUNCTION(NT, integral_division, const NT&, const NT&);
  CGAL_GLOBAL_FUNCTION(NT, inverse, const NT&);
  CGAL_GLOBAL_FUNCTION(bool, is_negative, const NT&);
  CGAL_GLOBAL_FUNCTION(bool, is_positive, const NT&);
  CGAL_GLOBAL_FUNCTION(bool, is_square, const NT&);
  CGAL_GLOBAL_FUNCTION(bool, is_square, const NT&, NT&);
  CGAL_GLOBAL_FUNCTION(bool, is_zero, const NT&);
  // no matching method for given NT
  //GLOBAL_FUNCTION(void, simplify, const NT&);
  CGAL_GLOBAL_FUNCTION(double, to_double, const NT&);
  cgal.method("to_interval", [](const NT& nt) {
      auto pair = CGAL::to_interval(nt);
      return std::make_tuple(pair.first, pair.second);
  });
  CGAL_GLOBAL_FUNCTION(NT, unit_part, const NT&);
}
