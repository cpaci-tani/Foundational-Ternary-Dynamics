// FTD-0707: repair the L=33 rest preparation in the existing four-coordinate
// symmetry sector, then qualify the complete reciprocal fixed point.

#define FTD_0704_EMBEDDED
#include "test_connected_dressed_matter_high_speed_preflight.cpp"
#undef FTD_0704_EMBEDDED

namespace {

constexpr char rest33_protocol_sha256[] =
    "0E1C61DDE059B8693DB68438CA17E056B39146278804BB35349DEA6FB5827FB0";
constexpr char rest33_parent_protocol_sha256[] =
    "D07F8CE10B43D209A3C2EAA6AA9A316B12192CE2CF072612935E0F8451FE8FA7";
constexpr int rest33_dof = 4;
constexpr int rest33_ticks = 8;
constexpr double rest33_gradient_step = 2e-5;
constexpr double rest33_hessian_step = 2e-4;
using Rest33Parameters = std::array<double, rest33_dof>;
using Rest33Matrix = std::array<std::array<double, rest33_dof>, rest33_dof>;

struct Rest33Evaluation {
  bool valid = false;
  double energy = INFINITY;
  double gauss = INFINITY;
  ftd::eft::ConnectedMooreBlockState state;
  explicit Rest33Evaluation(int L = 0) : state(L) {}
};

struct Rest33Derivatives {
  bool valid = false;
  Rest33Parameters gradient{};
  Rest33Matrix hessian{};
  Rest33Matrix vectors{};
  Rest33Parameters eigenvalues{};
  double gradient_inf = INFINITY;
  double minimum_eigenvalue = -INFINITY;
};

struct Rest33Iteration {
  int iteration = 0;
  double energy = INFINITY;
  double gradient = INFINITY;
  double minimum_eigenvalue = -INFINITY;
  double step_inf = INFINITY;
  double accepted_scale = 0.0;
};

struct Rest33Tick {
  int tick = 0;
  int hops = 0;
  double state = INFINITY;
  double center = INFINITY;
  double energy = INFINITY;
  double common = INFINITY;
};

struct Rest33Summary {
  bool parent = false;
  bool normalization = false;
  bool evaluations = false;
  bool optimization = false;
  bool one_step = false;
  bool forward = false;
  bool reverse = false;
  bool covariance = false;
  int evaluations_count = 0;
  int accepted_steps = 0;
  Rest33Parameters parameters{};
  double starting_energy = INFINITY;
  double final_energy = INFINITY;
  double final_gradient = INFINITY;
  double minimum_eigenvalue = -INFINITY;
  double maximum_impulse = INFINITY;
  double one_step_state = INFINITY;
  double one_step_momentum = INFINITY;
  double maximum_state = 0.0;
  double maximum_center = 0.0;
  double maximum_energy = 0.0;
  double maximum_common = 0.0;
  double recovery = INFINITY;
  double covariance_residual = INFINITY;
  std::string verdict = "L33_SYMMETRY_REST_REFINEMENT_EXECUTION_INVALID";
  ftd::eft::ConnectedMooreBlockState refined;
  std::vector<Rest33Iteration> iterations;
  std::vector<Rest33Tick> ticks;
};

bool rest33_parent_fingerprint() {
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0706/ftd_0706_complete_moving_dressing_relative_orbit_v1.json";
  std::ifstream input(path, std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(input)), {});
  return bytes.find(rest33_parent_protocol_sha256) != std::string::npos
      && bytes.find("MOVING_DRESSING_RELATIVE_ORBIT_EXECUTION_INVALID")
          != std::string::npos;
}

double rest33_max_component(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y), std::abs(value.z)});
}

double rest33_parameter_inf(const Rest33Parameters& values) {
  double result = 0.0;
  for (double value : values) result = std::max(result, std::abs(value));
  return result;
}

ftd::eft::ConnectedMooreBlockState rest33_geometry(
    const ftd::eft::ConnectedMooreBlockState& reference,
    const Rest33Parameters& parameters) {
  auto result = reference;
  const Vec3 c = center(reference);
  for (std::size_t i = 0; i < result.constituents.size(); ++i) {
    Vec3 x = position(reference.constituents[i]);
    const Vec3 d = x - c;
    const bool outer = std::abs(d.x) > 1.0;
    x.x += std::copysign(parameters[outer ? 0 : 1], d.x);
    const double transverse = parameters[outer ? 2 : 3];
    x.y += std::copysign(transverse, d.y);
    x.z += std::copysign(transverse, d.z);
    result.constituents[i] = preflight_point_at(x, preflight_volume);
    result.constituents[i].momentum = {};
  }
  return result;
}

Rest33Evaluation rest33_evaluate(
    const ftd::eft::ConnectedMooreBlockState& reference,
    const Rest33Parameters& parameters, double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options, int& count) {
  Rest33Evaluation result(preflight_volume);
  ++count;
  const auto dressed = ftd::eft::redress_connected_moore_block_with_fibre_limit(
      rest33_geometry(reference, parameters), 8, 1e-13, 4096);
  if (!dressed.valid) return result;
  result.valid = true;
  result.state = dressed.state;
  result.gauss = dressed.gauss_residual;
  result.energy = preflight_energy(result.state, beta, options);
  result.valid = std::isfinite(result.energy) && result.gauss <= 1e-12;
  return result;
}

void rest33_jacobi(Rest33Matrix matrix, Rest33Matrix& vectors,
                   Rest33Parameters& values) {
  for (int i = 0; i < rest33_dof; ++i) vectors[i][i] = 1.0;
  for (int iteration = 0; iteration < 96; ++iteration) {
    int p = 0, q = 1;
    double largest = std::abs(matrix[p][q]);
    for (int i = 0; i < rest33_dof; ++i)
      for (int j = i + 1; j < rest33_dof; ++j)
        if (std::abs(matrix[i][j]) > largest) {
          largest = std::abs(matrix[i][j]); p = i; q = j;
        }
    if (largest < 1e-13) break;
    const double angle = 0.5*std::atan2(
        2.0*matrix[p][q], matrix[q][q]-matrix[p][p]);
    const double c = std::cos(angle), s = std::sin(angle);
    for (int k = 0; k < rest33_dof; ++k) if (k != p && k != q) {
      const double kp = matrix[k][p], kq = matrix[k][q];
      matrix[k][p] = matrix[p][k] = c*kp-s*kq;
      matrix[k][q] = matrix[q][k] = s*kp+c*kq;
    }
    const double pp = matrix[p][p], qq = matrix[q][q], pq = matrix[p][q];
    matrix[p][p] = c*c*pp-2.0*c*s*pq+s*s*qq;
    matrix[q][q] = s*s*pp+2.0*c*s*pq+c*c*qq;
    matrix[p][q] = matrix[q][p] = 0.0;
    for (int k = 0; k < rest33_dof; ++k) {
      const double kp = vectors[k][p], kq = vectors[k][q];
      vectors[k][p] = c*kp-s*kq;
      vectors[k][q] = s*kp+c*kq;
    }
  }
  for (int i = 0; i < rest33_dof; ++i) values[i] = matrix[i][i];
}

Rest33Derivatives rest33_derivatives(
    const ftd::eft::ConnectedMooreBlockState& reference,
    const Rest33Parameters& parameters, double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options, int& count) {
  Rest33Derivatives result;
  const auto center_eval = rest33_evaluate(
      reference, parameters, beta, options, count);
  if (!center_eval.valid) return result;
  for (int i = 0; i < rest33_dof; ++i) {
    auto plus = parameters, minus = parameters;
    plus[i] += rest33_gradient_step;
    minus[i] -= rest33_gradient_step;
    const auto ep = rest33_evaluate(reference, plus, beta, options, count);
    const auto em = rest33_evaluate(reference, minus, beta, options, count);
    if (!ep.valid || !em.valid) return result;
    result.gradient[i] = (ep.energy-em.energy)/(2.0*rest33_gradient_step);

    plus = parameters; minus = parameters;
    plus[i] += rest33_hessian_step;
    minus[i] -= rest33_hessian_step;
    const auto hp = rest33_evaluate(reference, plus, beta, options, count);
    const auto hm = rest33_evaluate(reference, minus, beta, options, count);
    if (!hp.valid || !hm.valid) return result;
    result.hessian[i][i] = (hp.energy-2.0*center_eval.energy+hm.energy)
        /(rest33_hessian_step*rest33_hessian_step);
  }
  for (int i = 0; i < rest33_dof; ++i) {
    for (int j = i + 1; j < rest33_dof; ++j) {
      auto pp = parameters, pm = parameters, mp = parameters, mm = parameters;
      pp[i] += rest33_hessian_step; pp[j] += rest33_hessian_step;
      pm[i] += rest33_hessian_step; pm[j] -= rest33_hessian_step;
      mp[i] -= rest33_hessian_step; mp[j] += rest33_hessian_step;
      mm[i] -= rest33_hessian_step; mm[j] -= rest33_hessian_step;
      const auto epp = rest33_evaluate(reference, pp, beta, options, count);
      const auto epm = rest33_evaluate(reference, pm, beta, options, count);
      const auto emp = rest33_evaluate(reference, mp, beta, options, count);
      const auto emm = rest33_evaluate(reference, mm, beta, options, count);
      if (!epp.valid || !epm.valid || !emp.valid || !emm.valid) return result;
      result.hessian[i][j] = result.hessian[j][i] =
          (epp.energy-epm.energy-emp.energy+emm.energy)
          /(4.0*rest33_hessian_step*rest33_hessian_step);
    }
  }
  rest33_jacobi(result.hessian, result.vectors, result.eigenvalues);
  result.gradient_inf = rest33_parameter_inf(result.gradient);
  result.minimum_eigenvalue = *std::min_element(
      result.eigenvalues.begin(), result.eigenvalues.end());
  result.valid = std::isfinite(result.gradient_inf)
      && std::isfinite(result.minimum_eigenvalue);
  return result;
}

Rest33Parameters rest33_newton_step(const Rest33Derivatives& derivatives) {
  Rest33Parameters projected{}, step{};
  for (int mode = 0; mode < rest33_dof; ++mode)
    for (int i = 0; i < rest33_dof; ++i)
      projected[mode] += derivatives.vectors[i][mode]
          * derivatives.gradient[i];
  for (int mode = 0; mode < rest33_dof; ++mode)
    projected[mode] /= std::max(1e-6, derivatives.eigenvalues[mode]);
  for (int i = 0; i < rest33_dof; ++i)
    for (int mode = 0; mode < rest33_dof; ++mode)
      step[i] -= derivatives.vectors[i][mode]*projected[mode];
  return step;
}

ftd::eft::ConnectedMooreBlockState rest33_translate(
    const ftd::eft::ConnectedMooreBlockState& source, int dx) {
  auto result = source;
  const int L = source.electric.L;
  for (auto& point : result.constituents)
    point.anchor.x = preflight_wrap(point.anchor.x+dx, L);
  for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
      const int from = source.electric.index(x,y,z);
      const int to = result.electric.index(x+dx,y,z);
      result.electric.x[to]=source.electric.x[from];
      result.electric.y[to]=source.electric.y[from];
      result.electric.z[to]=source.electric.z[from];
      result.magnetic_half.x[to]=source.magnetic_half.x[from];
      result.magnetic_half.y[to]=source.magnetic_half.y[from];
      result.magnetic_half.z[to]=source.magnetic_half.z[from];
    }
  return result;
}

void rest33_run(Rest33Summary& summary,
                const ftd::eft::ConnectedMooreBlockState& reference,
                double beta,
                const ftd::eft::ConnectedMooreBlockOptions& options) {
  Rest33Parameters parameters{};
  auto current = rest33_evaluate(
      reference, parameters, beta, options, summary.evaluations_count);
  if (!current.valid) return;
  summary.starting_energy = current.energy;
  summary.evaluations = true;
  for (int iteration = 0; iteration < 8; ++iteration) {
    const auto derivatives = rest33_derivatives(
        reference, parameters, beta, options, summary.evaluations_count);
    if (!derivatives.valid) { summary.evaluations = false; return; }
    Rest33Iteration row;
    row.iteration = iteration;
    row.energy = current.energy;
    row.gradient = derivatives.gradient_inf;
    row.minimum_eigenvalue = derivatives.minimum_eigenvalue;
    if (derivatives.gradient_inf <= 1e-9) {
      row.step_inf = 0.0;
      summary.iterations.push_back(row);
      break;
    }
    const auto step = rest33_newton_step(derivatives);
    row.step_inf = rest33_parameter_inf(step);
    bool accepted = false;
    for (int backtrack = 0; backtrack <= 10; ++backtrack) {
      const double scale = std::ldexp(1.0, -backtrack);
      auto trial = parameters;
      for (int i = 0; i < rest33_dof; ++i) trial[i] += scale*step[i];
      const auto candidate = rest33_evaluate(
          reference, trial, beta, options, summary.evaluations_count);
      if (candidate.valid && candidate.energy < current.energy) {
        parameters = trial;
        current = candidate;
        row.accepted_scale = scale;
        ++summary.accepted_steps;
        accepted = true;
        break;
      }
    }
    summary.iterations.push_back(row);
    if (!accepted) break;
  }

  const auto final_derivatives = rest33_derivatives(
      reference, parameters, beta, options, summary.evaluations_count);
  if (!final_derivatives.valid || !current.valid) {
    summary.evaluations = false;
    return;
  }
  summary.parameters = parameters;
  summary.final_energy = current.energy;
  summary.final_gradient = final_derivatives.gradient_inf;
  summary.minimum_eigenvalue = final_derivatives.minimum_eigenvalue;
  summary.refined = current.state;
  summary.optimization = summary.final_gradient <= 1e-9
      && summary.minimum_eigenvalue > 1e-6
      && summary.final_energy < summary.starting_energy;

  const auto first = ftd::eft::solve_connected_moore_block_forward(
      summary.refined, options);
  summary.maximum_impulse = 0.0;
  if (first.valid && first.common_action_gates_pass) {
    for (const auto& impulse : first.total_impulses)
      summary.maximum_impulse = std::max(
          summary.maximum_impulse, rest33_max_component(impulse));
    summary.one_step_state =
        ftd::eft::connected_moore_block_state_max_difference(
            summary.refined, first.later);
    summary.one_step_momentum = rest33_max_component(
        preflight_total_momentum(first.later));
  }
  summary.one_step = first.valid && first.common_action_gates_pass
      && common_residual(first) <= 1e-10
      && summary.maximum_impulse <= 1e-9
      && summary.one_step_state <= 1e-9
      && summary.one_step_momentum <= 1e-9;

  const double energy0 = preflight_energy(summary.refined, beta, options);
  const Vec3 center0 = center(summary.refined);
  auto state = summary.refined;
  summary.forward = true;
  ftd::eft::ConnectedMooreBlockSolveCache forward_cache;
  for (int tick = 1; tick <= rest33_ticks && summary.forward; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        state, options, &forward_cache);
    const double common = common_residual(step);
    if (!step.valid || !step.common_action_gates_pass || common > 1e-10) {
      summary.forward = false; break;
    }
    state = step.later;
    Rest33Tick row;
    row.tick = tick;
    row.hops = step.site_hops;
    row.state = ftd::eft::connected_moore_block_state_max_difference(
        summary.refined, state);
    row.center = (center(state)-center0).mag();
    row.energy = std::abs(preflight_energy(state,beta,options)-energy0);
    row.common = common;
    summary.maximum_state = std::max(summary.maximum_state, row.state);
    summary.maximum_center = std::max(summary.maximum_center, row.center);
    summary.maximum_energy = std::max(summary.maximum_energy, row.energy);
    summary.maximum_common = std::max(summary.maximum_common, row.common);
    summary.ticks.push_back(row);
  }
  summary.forward = summary.forward && summary.ticks.size() == rest33_ticks;
  summary.reverse = summary.forward;
  ftd::eft::ConnectedMooreBlockSolveCache reverse_cache;
  for (int tick = 0; tick < rest33_ticks && summary.reverse; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_reverse(
        state, options, &reverse_cache);
    const double common = common_residual(step);
    if (!step.valid || !step.common_action_gates_pass || common > 1e-10) {
      summary.reverse = false; break;
    }
    state = step.earlier;
    summary.maximum_common = std::max(summary.maximum_common, common);
  }
  if (summary.reverse) summary.recovery =
      ftd::eft::connected_moore_block_state_max_difference(
          summary.refined, state);

  const auto shifted = rest33_translate(summary.refined, 3);
  const auto shifted_step = ftd::eft::solve_connected_moore_block_forward(
      shifted, options);
  if (first.valid && shifted_step.valid && first.common_action_gates_pass
      && shifted_step.common_action_gates_pass) {
    summary.covariance_residual =
        ftd::eft::connected_moore_block_state_max_difference(
            shifted_step.later, rest33_translate(first.later,3));
  }
  summary.covariance = shifted_step.valid
      && shifted_step.common_action_gates_pass
      && common_residual(shifted_step) <= 1e-10
      && summary.covariance_residual <= 1e-9;

  const bool repeated = summary.one_step && summary.forward && summary.reverse
      && summary.maximum_state <= 1e-8
      && summary.maximum_center <= 1e-10
      && summary.maximum_energy <= 1e-10
      && summary.maximum_common <= 1e-10
      && summary.recovery <= 1e-9;
  if (!summary.evaluations || !first.valid || !shifted_step.valid
      || !summary.covariance) {
    summary.verdict = "L33_SYMMETRY_REST_REFINEMENT_EXECUTION_INVALID";
  } else if (summary.optimization && repeated) {
    summary.verdict = "L33_SYMMETRY_REST_FIXED_POINT_CONSTRUCTIVE";
  } else {
    summary.verdict = "L33_REST_REQUIRES_FULL_COORDINATE_REFINEMENT";
  }
}

void rest33_write(const Rest33Summary& summary) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results/ftd_0707";
  std::filesystem::create_directories(directory);
  std::ofstream json(directory / "ftd_0707_l33_symmetry_rest_refinement_v1.json");
  json << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0707\",\n"
       << "  \"protocol_sha256\": \"" << rest33_protocol_sha256 << "\",\n"
       << "  \"parent_protocol_sha256\": \""
       << rest33_parent_protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"volume\": " << preflight_volume << ",\n"
       << "  \"evaluations_pass\": " << summary.evaluations << ",\n"
       << "  \"optimization_pass\": " << summary.optimization << ",\n"
       << "  \"one_step_pass\": " << summary.one_step << ",\n"
       << "  \"forward_pass\": " << summary.forward << ",\n"
       << "  \"reverse_pass\": " << summary.reverse << ",\n"
       << "  \"covariance_pass\": " << summary.covariance << ",\n"
       << "  \"evaluations\": " << summary.evaluations_count << ",\n"
       << "  \"accepted_steps\": " << summary.accepted_steps << ",\n"
       << "  \"starting_energy\": " << summary.starting_energy << ",\n"
       << "  \"final_energy\": " << summary.final_energy << ",\n"
       << "  \"final_gradient\": " << summary.final_gradient << ",\n"
       << "  \"minimum_eigenvalue\": " << summary.minimum_eigenvalue << ",\n"
       << "  \"maximum_impulse\": " << summary.maximum_impulse << ",\n"
       << "  \"one_step_state\": " << summary.one_step_state << ",\n"
       << "  \"one_step_momentum\": " << summary.one_step_momentum << ",\n"
       << "  \"maximum_state\": " << summary.maximum_state << ",\n"
       << "  \"maximum_center\": " << summary.maximum_center << ",\n"
       << "  \"maximum_energy\": " << summary.maximum_energy << ",\n"
       << "  \"maximum_common\": " << summary.maximum_common << ",\n"
       << "  \"recovery\": " << summary.recovery << ",\n"
       << "  \"covariance_residual\": " << summary.covariance_residual
       << "\n}\n";

  std::ofstream iterations(directory /
      "ftd_0707_l33_symmetry_rest_refinement_iterations_v1.csv");
  iterations << "ftd_id,iteration,energy,gradient,min_eigenvalue,step_inf,accepted_scale\n";
  for (const auto& row : summary.iterations)
    iterations << std::setprecision(17) << "FTD-0707," << row.iteration << ','
        << row.energy << ',' << row.gradient << ',' << row.minimum_eigenvalue
        << ',' << row.step_inf << ',' << row.accepted_scale << '\n';

  std::ofstream state(directory /
      "ftd_0707_l33_symmetry_rest_refinement_state_v1.csv");
  state << "ftd_id,particle,charge,x,y,z,px,py,pz\n";
  for (std::size_t i = 0; i < summary.refined.constituents.size(); ++i) {
    const Vec3 x = position(summary.refined.constituents[i]);
    const Vec3 p = summary.refined.constituents[i].momentum;
    state << std::setprecision(17) << "FTD-0707," << i << ','
          << summary.refined.charges[i] << ',' << x.x << ',' << x.y << ','
          << x.z << ',' << p.x << ',' << p.y << ',' << p.z << '\n';
  }

  std::ofstream ticks(directory /
      "ftd_0707_l33_symmetry_rest_refinement_ticks_v1.csv");
  ticks << "ftd_id,tick,hops,state,center,energy,common\n";
  for (const auto& row : summary.ticks)
    ticks << std::setprecision(17) << "FTD-0707," << row.tick << ','
          << row.hops << ',' << row.state << ',' << row.center << ','
          << row.energy << ',' << row.common << '\n';
}

}  // namespace

int main() {
  Rest33Summary summary;
  summary.parent = rest33_parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  summary.normalization = normalization.valid;
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  options.use_sparse_local_current = true;
  options.use_local_residual_evaluation = true;
  const auto reference = preflight_reference();
  if (summary.parent && summary.normalization
      && reference.electric.L == preflight_volume) {
    rest33_run(summary, reference,
        normalization.mapped_field_work_coefficient, options);
  }
  rest33_write(summary);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << rest33_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "evaluations=" << summary.evaluations_count
            << " accepted=" << summary.accepted_steps
            << " energy=(" << summary.starting_energy << ','
            << summary.final_energy << ") gradient=" << summary.final_gradient
            << " min_eigen=" << summary.minimum_eigenvalue << '\n'
            << "one_step=" << summary.one_step
            << " impulse=" << summary.maximum_impulse
            << " state=" << summary.one_step_state
            << " momentum=" << summary.one_step_momentum << '\n'
            << "forward=" << summary.forward
            << " reverse=" << summary.reverse
            << " max_state=" << summary.maximum_state
            << " center=" << summary.maximum_center
            << " energy=" << summary.maximum_energy
            << " common=" << summary.maximum_common
            << " recovery=" << summary.recovery
            << " covariance=" << summary.covariance_residual << '\n';
  return summary.verdict ==
      "L33_SYMMETRY_REST_REFINEMENT_EXECUTION_INVALID" ? 1 : 0;
}

