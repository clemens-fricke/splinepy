#include <memory>
#include <vector>

// pybind11
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <splinepy/splines/splinepy_base.hpp>
#include <splinepy/utils/print.hpp>
#include <splinepy/utils/grid_points.hpp>

namespace splinepy::py {

namespace py = pybind11;

using namespace splinelib::sources;

/// True interface to python  
class PySpline {
public:
  using CoreSpline_ = typename std::shared_ptr<splinepy::splines::SplinepyBase>;

  CoreSpline_ c_spline_;
  int para_dim_;
  int dim_;

  // ctor
  PySpline() = default;
  PySpline(const py::kwargs& kwargs) {
    NewCore(kwargs);
  }

  /// Creates a corresponding spline based on kwargs
  /// similar to previous update_c()
  void NewCore(const py::kwargs& kwargs) {
    // parse kwargs
    double* degrees_ptr = nullptr;
    std::vector<std::vector<double>> knot_vectors;
    std::vector<std::vector<double>>* knot_vectors_ptr = nullptr;
    double* control_points_ptr = nullptr;
    double* weights_ptr = nullptr;
    int para_dim, dim;

    // get degrees and set para_dim
    auto d_array = py::cast<py::array_t<double>>(kwargs["degrees"]);
    degrees_ptr = static_cast<double*>(d_array.request().ptr);
    para_dim = d_array.shape(0);
    knot_vectors.reserve(para_dim);

    // get cps and set dim
    auto cp_array = py::cast<py::array_t<double>>(kwargs["control_points"]);
    control_points_ptr = static_cast<double*>(cp_array.request().ptr);
    dim = cp_array.shape(1);

    // maybe, get knot_vectors
    if (kwargs.contains("knot_vectors")) {
      for (py::handle kv : kwargs["knot_vectors"]) {
        std::vector<double> knot_vector;
        // cast to array_t, as python side will try to use tracked array anyways
        auto kv_array = py::cast<py::array_t<double>>(kv);
        knot_vector.reserve(kv_array.size());
        for (py::handle k : kv) {
          knot_vector.push_back(k.cast<double>());
        }
        knot_vectors.push_back(std::move(knot_vector));
      }
      knot_vectors_ptr = &knot_vectors;
    } 

    // maybe, get weights
    if (kwargs.contains("weights")) {
      auto w_array = py::cast<py::array_t<double>>(kwargs["weights"]);
      weights_ptr = static_cast<double*>(w_array.request().ptr);
    }

    // new assign
    c_spline_ = splinepy::splines::SplinepyBase::Create(
        para_dim,
        dim,
        degrees_ptr,
        knot_vectors_ptr,
        control_points_ptr,
        weights_ptr
    );
    para_dim_ = c_spline_->ParaDim();
    dim_ = c_spline_->Dim();
  }

  py::dict CurrentProperties() {
    
  }

  py::array_t<double> Evaluate(py::array_t<double> queries, int nthreads) {
    // prepare output
    const int n_queries = queries.shape(0);
    py::array_t<double> evaluated(n_queries * dim_);
    double* evaluated_ptr = static_cast<double*>(evaluated.request().ptr);

    // prepare vectorized evaluate queries
    double* queries_ptr = static_cast<double*>(queries.request().ptr);
    auto evaluate = [&](int begin, int end) {
      for (int i{begin}; i < end; ++i) {
        c_spline_->RawPtrEvaluate(
            &queries_ptr[i * para_dim_],
            &evaluated_ptr[i * dim_]
        );
      }
    };

    splinepy::utils::NThreadExecution(evaluate, n_queries, nthreads);

    evaluated.resize({n_queries, dim_});
    return evaluated;
  }

  /// Sample wraps evaluate to allow nthread executions
  /// Requires RawPtrParametricBounds 
  py::array_t<double> Sample(py::array_t<int> resolutions, int nthreads) {
    // get sampling bounds
    std::vector<double> bounds(para_dim_ * 2);
    double* bounds_ptr = bounds.data();
    c_spline_->RawPtrParametricBounds(bounds_ptr);
    // prepare sampler
    auto grid = splinepy::utils::RawPtrGridPoints(
        para_dim_,
        bounds_ptr,
        static_cast<int*>(resolutions.request().ptr));
    // prepare output
    const int n_sampled = grid.Size();
    py::array_t<double> sampled(n_sampled * dim_);
    double* sampled_ptr = static_cast<double*>(sampled.request().ptr);

    // wrap evaluate
    auto sample = [&](int begin, int end) {
      std::vector<double> query(para_dim_);
      double* query_ptr = query.data();
      for (int i{begin}; i < end; ++i) {
        grid.IdToGridPoint(i, query_ptr);
        c_spline_->RawPtrEvaluate(
            &query[0],
            &sampled_ptr[i * dim_]
        );
      }
    };

    splinepy::utils::NThreadExecution(sample, n_sampled, nthreads);

    sampled.resize({n_sampled, dim_});
    return sampled;
  }

};

} // namespace splinepy::py

void add_spline_pyclass(py::module& m, const char* class_name) {
  py::class_<splinepy::py::PySpline> klasse(m, class_name);

  klasse.def(py::init<>())
      .def(py::init<py::kwargs>()) // doc here?
      .def("evaluate",
           &splinepy::py::PySpline::Evaluate,
           py::arg("queries"),
           py::arg("nthreads")=1)
      .def("sample",
           &splinepy::py::PySpline::Sample,
           py::arg("resolutions"),
           py::arg("nthreads")=1)

      ;
}
