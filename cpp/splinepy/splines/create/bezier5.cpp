#ifdef SPLINEPY_MORE

#include <splinepy/splines/create/create_bezier.hpp>

namespace splinepy::splines::create {

/// dynamic creation of templated Bezier
std::shared_ptr<splinepy::splines::SplinepyBase>
CreateBezier5(const int dim, const int* degrees, const double* control_points) {
  switch (dim) {
  case 1:
    return std::make_shared<Bezier<5, 1>>(degrees, control_points);
  case 2:
    return std::make_shared<Bezier<5, 2>>(degrees, control_points);
  case 3:
    return std::make_shared<Bezier<5, 3>>(degrees, control_points);
  case 4:
    return std::make_shared<Bezier<5, 4>>(degrees, control_points);
  case 5:
    return std::make_shared<Bezier<5, 5>>(degrees, control_points);
  case 6:
    return std::make_shared<Bezier<5, 6>>(degrees, control_points);
  case 7:
    return std::make_shared<Bezier<5, 7>>(degrees, control_points);
  case 8:
    return std::make_shared<Bezier<5, 8>>(degrees, control_points);
  case 9:
    return std::make_shared<Bezier<5, 9>>(degrees, control_points);
  case 10:
    return std::make_shared<Bezier<5, 10>>(degrees, control_points);
  default:
    splinepy::utils::PrintAndThrowError(
        "Something went wrong during CreateBezier. Please help us by writing "
        "an issue about this case at [ github.com/tataratat/splinepy ]");
    break;
  }
  splinepy::utils::PrintAndThrowError(
      "Something went very wrong during CreateBezier. Please help us by "
      "writing "
      "an issue about this case at [ github.com/tataratat/splinepy ]");
  // make compiler happy
  return std::shared_ptr<SplinepyBase>{};
}

} // namespace splinepy::splines::create

#endif
