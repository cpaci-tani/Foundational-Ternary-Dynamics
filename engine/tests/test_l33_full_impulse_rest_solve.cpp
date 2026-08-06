// FTD-0708: solve the complete 48-coordinate rest residual at L=33 using the
// actual one-tick common-action impulses as the force vector.

#define FTD_0704_EMBEDDED
#include "test_connected_dressed_matter_high_speed_preflight.cpp"
#undef FTD_0704_EMBEDDED

namespace {

constexpr char fullrest_protocol_sha256[] =
    "D978E8920D8121CA2FC91F3E6B4F68353B98E7B6285B4A82304511EE4177D007";
constexpr char fullrest_parent_protocol_sha256[] =
    "0E1C61DDE059B8693DB68438CA17E056B39146278804BB35349DEA6FB5827FB0";
constexpr int fullrest_dof = 3*count;
constexpr int fullrest_ticks = 8;
constexpr double fullrest_jacobian_step = 2e-5;
using FullRestVector = std::array<double, fullrest_dof>;
using FullRestMatrix = std::array<std::array<double, fullrest_dof>, fullrest_dof>;

struct FullRestEvaluation {
  bool valid = false;
  double residual = INFINITY;
  double common = INFINITY;
  double energy = INFINITY;
  FullRestVector impulses{};
  ftd::eft::ConnectedMooreBlockState state;
  ftd::eft::ConnectedMooreBlockState later;
  explicit FullRestEvaluation(int L = 0) : state(L), later(L) {}
};

struct FullRestIteration {
  int iteration = 0;
  double residual = INFINITY;
  double step = INFINITY;
  double accepted_scale = 0.0;
  double minimum_pivot = INFINITY;
  int evaluations = 0;
};

struct FullRestTick {
  int tick = 0;
  int hops = 0;
  double state = INFINITY;
  double center = INFINITY;
  double energy = INFINITY;
  double common = INFINITY;
};

struct FullRestSummary {
  bool parent = false;
  bool normalization = false;
  bool evaluations = false;
  bool linear_algebra = true;
  bool root = false;
  bool one_step = false;
  bool forward = false;
  bool reverse = false;
  bool covariance = false;
  int evaluations_count = 0;
  int accepted_steps = 0;
  int total_hops = 0;
  double starting_residual = INFINITY;
  double final_residual = INFINITY;
  double maximum_displacement = 0.0;
  double one_step_state = INFINITY;
  double one_step_momentum = INFINITY;
  double maximum_state = 0.0;
  double maximum_center = 0.0;
  double maximum_energy = 0.0;
  double maximum_common = 0.0;
  double recovery = INFINITY;
  double covariance_residual = INFINITY;
  FullRestVector displacement{};
  ftd::eft::ConnectedMooreBlockState refined;
  std::vector<FullRestIteration> iterations;
  std::vector<FullRestTick> ticks;
  std::string verdict = "L33_FULL_IMPULSE_REST_SOLVE_EXECUTION_INVALID";
};

bool fullrest_parent_fingerprint() {
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0707/ftd_0707_l33_symmetry_rest_refinement_v1.json";
  std::ifstream input(path, std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(input)), {});
  return bytes.find(fullrest_parent_protocol_sha256) != std::string::npos
      && bytes.find("L33_REST_REQUIRES_FULL_COORDINATE_REFINEMENT")
          != std::string::npos;
}

double fullrest_max_component(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y), std::abs(value.z)});
}

double fullrest_norm(const FullRestVector& values) {
  double result = 0.0;
  for (double value : values) result = std::max(result, std::abs(value));
  return result;
}

ftd::eft::ConnectedMooreBlockState fullrest_geometry(
    const ftd::eft::ConnectedMooreBlockState& reference,
    const FullRestVector& displacement, bool& sector) {
  auto result = reference;
  sector = true;
  for (int particle = 0; particle < count; ++particle) {
    Vec3 x = position(reference.constituents[particle]);
    x.x += displacement[3*particle];
    x.y += displacement[3*particle+1];
    x.z += displacement[3*particle+2];
    auto point = preflight_point_at(x, preflight_volume);
    sector = sector && point.anchor == reference.constituents[particle].anchor;
    point.momentum = {};
    result.constituents[particle] = point;
  }
  return result;
}

FullRestEvaluation fullrest_evaluate(
    const ftd::eft::ConnectedMooreBlockState& reference,
    const FullRestVector& displacement, double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options, int& count_eval) {
  FullRestEvaluation result(preflight_volume);
  ++count_eval;
  bool sector = false;
  const auto geometry = fullrest_geometry(reference, displacement, sector);
  if (!sector || fullrest_norm(displacement) > 0.05) return result;
  const auto dressed = ftd::eft::redress_connected_moore_block_with_fibre_limit(
      geometry, 8, 1e-13, 4096);
  if (!dressed.valid) return result;
  const auto step = ftd::eft::solve_connected_moore_block_forward(
      dressed.state, options);
  if (!step.valid || !step.common_action_gates_pass
      || step.total_impulses.size() != count) return result;
  result.common = common_residual(step);
  result.energy = std::abs(preflight_energy(step.later,beta,options)
      -preflight_energy(dressed.state,beta,options));
  for (int particle = 0; particle < count; ++particle) {
    result.impulses[3*particle] = step.total_impulses[particle].x;
    result.impulses[3*particle+1] = step.total_impulses[particle].y;
    result.impulses[3*particle+2] = step.total_impulses[particle].z;
  }
  result.residual = fullrest_norm(result.impulses);
  result.state = dressed.state;
  result.later = step.later;
  result.valid = result.common <= 1e-10 && result.energy <= 1e-10
      && std::isfinite(result.residual);
  return result;
}

bool fullrest_solve_linear(FullRestMatrix matrix, FullRestVector rhs,
                           FullRestVector& solution,
                           double& minimum_pivot) {
  minimum_pivot = INFINITY;
  for (int column = 0; column < fullrest_dof; ++column) {
    int pivot = column;
    for (int row = column+1; row < fullrest_dof; ++row)
      if (std::abs(matrix[row][column])
          > std::abs(matrix[pivot][column])) pivot = row;
    const double value = std::abs(matrix[pivot][column]);
    minimum_pivot = std::min(minimum_pivot, value);
    if (value <= 1e-10 || !std::isfinite(value)) return false;
    if (pivot != column) {
      std::swap(matrix[pivot], matrix[column]);
      std::swap(rhs[pivot], rhs[column]);
    }
    for (int row = column+1; row < fullrest_dof; ++row) {
      const double factor = matrix[row][column]/matrix[column][column];
      for (int k = column; k < fullrest_dof; ++k)
        matrix[row][k] -= factor*matrix[column][k];
      rhs[row] -= factor*rhs[column];
    }
  }
  for (int row = fullrest_dof-1; row >= 0; --row) {
    double value = rhs[row];
    for (int k = row+1; k < fullrest_dof; ++k)
      value -= matrix[row][k]*solution[k];
    solution[row] = value/matrix[row][row];
  }
  return std::all_of(solution.begin(), solution.end(),
      [](double value) { return std::isfinite(value); });
}

ftd::eft::ConnectedMooreBlockState fullrest_translate(
    const ftd::eft::ConnectedMooreBlockState& source, int dx) {
  auto result = source;
  const int L = source.electric.L;
  for (auto& point : result.constituents)
    point.anchor.x = preflight_wrap(point.anchor.x+dx,L);
  for (int x=0;x<L;++x) for(int y=0;y<L;++y) for(int z=0;z<L;++z) {
    const int from=source.electric.index(x,y,z);
    const int to=result.electric.index(x+dx,y,z);
    result.electric.x[to]=source.electric.x[from];
    result.electric.y[to]=source.electric.y[from];
    result.electric.z[to]=source.electric.z[from];
    result.magnetic_half.x[to]=source.magnetic_half.x[from];
    result.magnetic_half.y[to]=source.magnetic_half.y[from];
    result.magnetic_half.z[to]=source.magnetic_half.z[from];
  }
  return result;
}

void fullrest_run(FullRestSummary& summary,
                  const ftd::eft::ConnectedMooreBlockState& reference,
                  double beta,
                  const ftd::eft::ConnectedMooreBlockOptions& options) {
  FullRestVector displacement{};
  auto current = fullrest_evaluate(
      reference, displacement, beta, options, summary.evaluations_count);
  if (!current.valid) return;
  summary.evaluations = true;
  summary.starting_residual = current.residual;

  for (int iteration = 0; iteration < 6 && current.residual > 1e-9;
       ++iteration) {
    FullRestMatrix jacobian{};
    const int before = summary.evaluations_count;
    for (int column = 0; column < fullrest_dof; ++column) {
      auto plus = displacement, minus = displacement;
      plus[column] += fullrest_jacobian_step;
      minus[column] -= fullrest_jacobian_step;
      const auto ep = fullrest_evaluate(
          reference, plus, beta, options, summary.evaluations_count);
      const auto em = fullrest_evaluate(
          reference, minus, beta, options, summary.evaluations_count);
      if (!ep.valid || !em.valid) { summary.evaluations = false; return; }
      for (int row = 0; row < fullrest_dof; ++row)
        jacobian[row][column] = (ep.impulses[row]-em.impulses[row])
            /(2.0*fullrest_jacobian_step);
    }
    FullRestVector rhs{}, step{};
    for (int i=0;i<fullrest_dof;++i) rhs[i]=-current.impulses[i];
    FullRestIteration record;
    record.iteration=iteration;
    record.residual=current.residual;
    record.evaluations=summary.evaluations_count-before;
    if (!fullrest_solve_linear(jacobian,rhs,step,record.minimum_pivot)) {
      summary.linear_algebra=false;
      summary.iterations.push_back(record);
      return;
    }
    record.step=fullrest_norm(step);
    bool accepted=false;
    for(int backtrack=0;backtrack<=10;++backtrack) {
      const double scale=std::ldexp(1.0,-backtrack);
      auto trial=displacement;
      for(int i=0;i<fullrest_dof;++i)trial[i]+=scale*step[i];
      const auto candidate=fullrest_evaluate(
          reference,trial,beta,options,summary.evaluations_count);
      if(candidate.valid&&candidate.residual<current.residual) {
        displacement=trial;current=candidate;record.accepted_scale=scale;
        ++summary.accepted_steps;accepted=true;break;
      }
    }
    summary.iterations.push_back(record);
    if(!accepted)break;
  }

  summary.displacement=displacement;
  summary.maximum_displacement=fullrest_norm(displacement);
  summary.final_residual=current.residual;
  summary.refined=current.state;
  summary.root=current.residual<=1e-9;
  summary.one_step_state=ftd::eft::connected_moore_block_state_max_difference(
      current.state,current.later);
  summary.one_step_momentum=fullrest_max_component(
      preflight_total_momentum(current.later));
  summary.one_step=summary.root&&summary.one_step_state<=1e-9
      &&summary.one_step_momentum<=1e-9;

  const double energy0=preflight_energy(current.state,beta,options);
  const Vec3 center0=center(current.state);
  auto state=current.state;
  summary.forward=true;
  ftd::eft::ConnectedMooreBlockSolveCache forward_cache;
  for(int tick=1;tick<=fullrest_ticks&&summary.forward;++tick) {
    const auto step=ftd::eft::solve_connected_moore_block_forward(
        state,options,&forward_cache);
    const double common=common_residual(step);
    if(!step.valid||!step.common_action_gates_pass||common>1e-10) {
      summary.forward=false;break;
    }
    state=step.later;
    FullRestTick row;
    row.tick=tick;row.hops=step.site_hops;
    row.state=ftd::eft::connected_moore_block_state_max_difference(
        current.state,state);
    row.center=(center(state)-center0).mag();
    row.energy=std::abs(preflight_energy(state,beta,options)-energy0);
    row.common=common;
    summary.total_hops+=row.hops;
    summary.maximum_state=std::max(summary.maximum_state,row.state);
    summary.maximum_center=std::max(summary.maximum_center,row.center);
    summary.maximum_energy=std::max(summary.maximum_energy,row.energy);
    summary.maximum_common=std::max(summary.maximum_common,row.common);
    summary.ticks.push_back(row);
  }
  summary.forward=summary.forward&&summary.ticks.size()==fullrest_ticks;
  summary.reverse=summary.forward;
  ftd::eft::ConnectedMooreBlockSolveCache reverse_cache;
  for(int tick=0;tick<fullrest_ticks&&summary.reverse;++tick) {
    const auto step=ftd::eft::solve_connected_moore_block_reverse(
        state,options,&reverse_cache);
    const double common=common_residual(step);
    if(!step.valid||!step.common_action_gates_pass||common>1e-10) {
      summary.reverse=false;break;
    }
    state=step.earlier;
    summary.maximum_common=std::max(summary.maximum_common,common);
  }
  if(summary.reverse)summary.recovery=
      ftd::eft::connected_moore_block_state_max_difference(current.state,state);

  const auto shifted=fullrest_translate(current.state,3);
  const auto shifted_step=ftd::eft::solve_connected_moore_block_forward(
      shifted,options);
  if(shifted_step.valid&&shifted_step.common_action_gates_pass) {
    summary.covariance_residual=
        ftd::eft::connected_moore_block_state_max_difference(
            shifted_step.later,fullrest_translate(current.later,3));
  }
  summary.covariance=shifted_step.valid&&shifted_step.common_action_gates_pass
      &&common_residual(shifted_step)<=1e-10
      &&summary.covariance_residual<=1e-9;

  const bool fixed=summary.one_step&&summary.forward&&summary.reverse
      &&summary.total_hops==0&&summary.maximum_state<=1e-8
      &&summary.maximum_center<=1e-10&&summary.maximum_energy<=1e-10
      &&summary.maximum_common<=1e-10&&summary.recovery<=1e-9;
  if(!summary.evaluations||!summary.linear_algebra||!summary.covariance) {
    summary.verdict="L33_FULL_IMPULSE_REST_SOLVE_EXECUTION_INVALID";
  } else if(fixed) {
    summary.verdict="L33_FULL_IMPULSE_REST_FIXED_POINT_CONSTRUCTIVE";
  } else {
    summary.verdict="L33_EXISTING_STATE_HAS_NO_NEARBY_FULL_IMPULSE_REST_ROOT";
  }
}

void fullrest_write(const FullRestSummary& summary) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results/ftd_0708";
  std::filesystem::create_directories(directory);
  std::ofstream json(directory/"ftd_0708_l33_full_impulse_rest_solve_v1.json");
  json<<std::setprecision(17)
      <<"{\n  \"ftd_id\": \"FTD-0708\",\n"
      <<"  \"protocol_sha256\": \""<<fullrest_protocol_sha256<<"\",\n"
      <<"  \"parent_protocol_sha256\": \""<<fullrest_parent_protocol_sha256<<"\",\n"
      <<"  \"verdict\": \""<<summary.verdict<<"\",\n"
      <<"  \"production_changed\": false,\n"
      <<"  \"volume\": "<<preflight_volume<<",\n"
      <<"  \"evaluations_pass\": "<<summary.evaluations<<",\n"
      <<"  \"linear_algebra_pass\": "<<summary.linear_algebra<<",\n"
      <<"  \"root_pass\": "<<summary.root<<",\n"
      <<"  \"one_step_pass\": "<<summary.one_step<<",\n"
      <<"  \"forward_pass\": "<<summary.forward<<",\n"
      <<"  \"reverse_pass\": "<<summary.reverse<<",\n"
      <<"  \"covariance_pass\": "<<summary.covariance<<",\n"
      <<"  \"evaluations\": "<<summary.evaluations_count<<",\n"
      <<"  \"accepted_steps\": "<<summary.accepted_steps<<",\n"
      <<"  \"starting_residual\": "<<summary.starting_residual<<",\n"
      <<"  \"final_residual\": "<<summary.final_residual<<",\n"
      <<"  \"maximum_displacement\": "<<summary.maximum_displacement<<",\n"
      <<"  \"one_step_state\": "<<summary.one_step_state<<",\n"
      <<"  \"one_step_momentum\": "<<summary.one_step_momentum<<",\n"
      <<"  \"total_hops\": "<<summary.total_hops<<",\n"
      <<"  \"maximum_state\": "<<summary.maximum_state<<",\n"
      <<"  \"maximum_center\": "<<summary.maximum_center<<",\n"
      <<"  \"maximum_energy\": "<<summary.maximum_energy<<",\n"
      <<"  \"maximum_common\": "<<summary.maximum_common<<",\n"
      <<"  \"recovery\": "<<summary.recovery<<",\n"
      <<"  \"covariance_residual\": "<<summary.covariance_residual<<"\n}\n";
  std::ofstream iterations(directory/
      "ftd_0708_l33_full_impulse_rest_solve_iterations_v1.csv");
  iterations<<"ftd_id,iteration,residual,step,accepted_scale,minimum_pivot,evaluations\n";
  for(const auto&row:summary.iterations)iterations<<std::setprecision(17)
      <<"FTD-0708,"<<row.iteration<<','<<row.residual<<','<<row.step<<','
      <<row.accepted_scale<<','<<row.minimum_pivot<<','<<row.evaluations<<'\n';
  std::ofstream state(directory/
      "ftd_0708_l33_full_impulse_rest_solve_state_v1.csv");
  state<<"ftd_id,particle,charge,x,y,z,dx,dy,dz\n";
  for(int i=0;i<count;++i){const Vec3 x=position(summary.refined.constituents[i]);
    state<<std::setprecision(17)<<"FTD-0708,"<<i<<','<<summary.refined.charges[i]
        <<','<<x.x<<','<<x.y<<','<<x.z<<','<<summary.displacement[3*i]<<','
        <<summary.displacement[3*i+1]<<','<<summary.displacement[3*i+2]<<'\n';}
  std::ofstream ticks(directory/
      "ftd_0708_l33_full_impulse_rest_solve_ticks_v1.csv");
  ticks<<"ftd_id,tick,hops,state,center,energy,common\n";
  for(const auto&row:summary.ticks)ticks<<std::setprecision(17)<<"FTD-0708,"
      <<row.tick<<','<<row.hops<<','<<row.state<<','<<row.center<<','
      <<row.energy<<','<<row.common<<'\n';
}

}  // namespace

int main(){
  FullRestSummary summary;
  summary.parent=fullrest_parent_fingerprint();
  const auto normalization=ftd::eft::measure_face_flux_normalization();
  summary.normalization=normalization.valid;
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart=true;
  options.use_sparse_local_current=true;
  options.use_local_residual_evaluation=true;
  const auto reference=preflight_reference();
  if(summary.parent&&summary.normalization
      &&reference.electric.L==preflight_volume)
    fullrest_run(summary,reference,
        normalization.mapped_field_work_coefficient,options);
  fullrest_write(summary);
  std::cout<<std::setprecision(17)
      <<"protocol_sha256="<<fullrest_protocol_sha256<<'\n'
      <<"verdict="<<summary.verdict<<'\n'
      <<"evaluations="<<summary.evaluations_count
      <<" accepted="<<summary.accepted_steps
      <<" residual=("<<summary.starting_residual<<','<<summary.final_residual
      <<") displacement="<<summary.maximum_displacement<<'\n'
      <<"root="<<summary.root<<" one_step="<<summary.one_step
      <<" state="<<summary.one_step_state
      <<" momentum="<<summary.one_step_momentum<<'\n'
      <<"forward="<<summary.forward<<" reverse="<<summary.reverse
      <<" hops="<<summary.total_hops<<" max_state="<<summary.maximum_state
      <<" center="<<summary.maximum_center<<" energy="<<summary.maximum_energy
      <<" common="<<summary.maximum_common<<" recovery="<<summary.recovery
      <<" covariance="<<summary.covariance_residual<<'\n';
  return summary.verdict=="L33_FULL_IMPULSE_REST_SOLVE_EXECUTION_INVALID"?1:0;
}

