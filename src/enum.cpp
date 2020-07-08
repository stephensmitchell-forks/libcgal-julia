#include <CGAL/enum.h>

#include <jlcxx/module.hpp>

#include "macros.hpp"

#define CGAL_CONST(N)   cgal.set_const(#N, CGAL::N)
#define CGAL_ENUM(E, N) cgal.add_bits<CGAL::E>(#N, jlcxx::julia_type("CppEnum"))
#define CGAL_SENUM(E)   CGAL_ENUM(E, E)

void wrap_enum(jlcxx::Module& cgal) {
  CGAL_SENUM(Sign);
  OVERRIDE_BASE(cgal,);
  /* cgal.UNARY_OP(-, CGAL::Sign); */
  /* cgal.BINARY_OP(CGAL::Sign, *, CGAL::Sign); */
  UNSET_OVERRIDE(cgal,);
  /* cgal.SPFUNC(CGAL, opposite, CGAL::Sign); */
  // Sign
  CGAL_CONST(NEGATIVE ); CGAL_CONST(ZERO    ); CGAL_CONST(POSITIVE  );
  CGAL_CONST(COLLINEAR); CGAL_CONST(COPLANAR); CGAL_CONST(DEGENERATE);
  // Orientation
  CGAL_CONST(RIGHT_TURN); CGAL_CONST(LEFT_TURN);
  CGAL_CONST(CLOCKWISE ); CGAL_CONST(COUNTERCLOCKWISE);
  // Oriented_side
  CGAL_CONST(ON_NEGATIVE_SIDE); CGAL_CONST(ON_ORIENTED_BOUNDARY); CGAL_CONST(ON_POSITIVE_SIDE);
  // Comparison_result
  CGAL_CONST(SMALLER); CGAL_CONST(EQUAL); CGAL_CONST(LARGER);

  CGAL_ENUM(Bounded_side, BoundedSide);
  /* CGAL_UNAMBIG_FUNC(CGAL::Bounded_side, opposite, CGAL::Bounded_side); */
  CGAL_CONST(ON_UNBOUNDED_SIDE); CGAL_CONST(ON_BOUNDARY); CGAL_CONST(ON_BOUNDED_SIDE);

  CGAL_SENUM(Angle);
  /* CGAL_UNAMBIG_FUNC(CGAL::Angle, opposite, CGAL::Angle); */
  CGAL_CONST(OBTUSE); CGAL_CONST(RIGHT); CGAL_CONST(ACUTE);

  CGAL_ENUM(Box_parameter_space_2, BoxParameterSpace2);
  CGAL_CONST(LEFT_BOUNDARY  ); CGAL_CONST(RIGHT_BOUNDARY);
  CGAL_CONST(BOTTOM_BOUNDARY); CGAL_CONST(TOP_BOUNDARY  );
  CGAL_CONST(INTERIOR       ); CGAL_CONST(EXTERIOR      );
}

#undef CGAL_CONST
#undef CGAL_ENUM
#undef CGAL_SENUM
