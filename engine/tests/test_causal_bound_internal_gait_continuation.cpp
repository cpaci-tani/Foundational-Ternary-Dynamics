// FTD-0713: continue the FTD-0712 internal gait under causal-speed and graph
// bounds after removing only the auxiliary 0.05 coordinate cap.

#define FTD_0712_EMBEDDED
#include "test_resonant_internal_gait_cancellation.cpp"
#undef FTD_0712_EMBEDDED

#include <limits>

namespace {

constexpr char causalgait_protocol_sha256[] =
    "901F2F2FDACEB47D62ED57EE0E4E114B1C4C29C6DF7F8188EA39E86F3DC724BF";
constexpr char causalgait_parent_sha256[] =
    "47BC6C8897FFFC0C983FDA6BB73910C6FADE87206544DE74BF63E7F52E344852";
constexpr double causalgait_parent_residual = 1.4076158628294926e-5;

struct CausalGaitSummary {
  bool parent=false,reconstruction=false,modes=false,state=false;
  bool crosscheck=false,evaluations=false,linear_algebra=true;
  bool root=false,conjugacy=false,covariance=false;
  int evaluation_count=0,accepted_steps=0;
  double starting_residual=INFINITY,starting_null_norm=INFINITY;
  double final_residual=INFINITY,final_null_norm=INFINITY;
  double maximum_displacement=INFINITY,maximum_speed=INFINITY;
  double edge_deformation=INFINITY,center_residual=INFINITY;
  double continuity_residual=INFINITY,causal_excess=INFINITY;
  double conjugacy_residual=INFINITY,covariance_residual=INFINITY;
  GaitVariables variables{};
  std::vector<GaitIteration> iterations;
  std::string verdict="CAUSAL_INTERNAL_GAIT_CONTINUATION_EXECUTION_INVALID";
};

bool causalgait_load(GaitVariables&variables) {
  const auto path=std::filesystem::path(__FILE__).parent_path().parent_path()/
      "results/ftd_0712/ftd_0712_resonant_internal_gait_state_v1.csv";
  std::ifstream input(path);std::string line;std::getline(input,line);int loaded=0;
  while(std::getline(input,line)) {
    std::stringstream row(line);std::array<std::string,4> fields;
    for(auto&field:fields)std::getline(row,field,',');
    const int particle=std::stoi(fields[0]);
    if(particle<0||particle>=count)return false;
    if(particle<count-1) {
      variables[3*particle]=std::stod(fields[1]);
      variables[3*particle+1]=std::stod(fields[2]);
      variables[3*particle+2]=std::stod(fields[3]);
    }
    ++loaded;
  }
  if(loaded!=count)return false;
  const auto delta=gait_deltas(variables);
  return delta[count-1].x==0.04999707526108492
      &&delta[count-1].y==0.0045553482183017489
      &&delta[count-1].z==0.0045553482146791736;
}

void causalgait_run(CausalGaitSummary&summary,
                    const ftd::eft::ConnectedMooreBlockState&reference,
                    const std::array<GaitMode,gait_modes>&modes) {
  auto variables=summary.variables;
  const double unlimited=std::numeric_limits<double>::infinity();
  auto current=gait_evaluate(reference,modes,variables,
      summary.evaluation_count,unlimited);
  if(!current.valid)return;
  summary.evaluations=true;summary.starting_residual=current.residual;
  summary.starting_null_norm=current.full_null_norm;
  summary.crosscheck=std::abs(current.residual-causalgait_parent_residual)<=1e-12;
  if(!summary.crosscheck)return;
  for(int iteration=0;iteration<8&&current.residual>1e-10;++iteration) {
    GaitJacobian jacobian{};const int before=summary.evaluation_count;
    for(int column=0;column<gait_variable_dof;++column) {
      auto plus=variables,minus=variables;
      plus[column]+=gait_fd_step;minus[column]-=gait_fd_step;
      const auto ep=gait_evaluate(reference,modes,plus,
          summary.evaluation_count,unlimited);
      const auto em=gait_evaluate(reference,modes,minus,
          summary.evaluation_count,unlimited);
      if(!ep.valid||!em.valid){summary.evaluations=false;return;}
      for(int row=0;row<gait_residual_dof;++row)
        jacobian[row][column]=(ep.values[row]-em.values[row])/(2.0*gait_fd_step);
    }
    GaitGram gram{};GaitResidual rhs{},dual{};
    for(int row=0;row<gait_residual_dof;++row) {
      rhs[row]=-current.values[row];
      for(int other=0;other<gait_residual_dof;++other)
        for(int column=0;column<gait_variable_dof;++column)
          gram[row][other]+=jacobian[row][column]*jacobian[other][column];
    }
    GaitIteration record;record.iteration=iteration;record.residual=current.residual;
    record.evaluations=summary.evaluation_count-before;
    if(!gait_solve_gram(gram,rhs,dual,record.minimum_pivot)) {
      summary.linear_algebra=false;summary.iterations.push_back(record);return;
    }
    GaitVariables step{};
    for(int column=0;column<gait_variable_dof;++column)
      for(int row=0;row<gait_residual_dof;++row)
        step[column]+=jacobian[row][column]*dual[row];
    record.step=gait_variable_norm(step);bool accepted=false;
    for(int backtrack=0;backtrack<=10;++backtrack) {
      const double scale=std::ldexp(1.0,-backtrack);auto trial=variables;
      for(int i=0;i<gait_variable_dof;++i)trial[i]+=scale*step[i];
      const auto candidate=gait_evaluate(reference,modes,trial,
          summary.evaluation_count,unlimited);
      if(candidate.valid&&candidate.residual<current.residual) {
        variables=trial;current=candidate;record.scale=scale;
        ++summary.accepted_steps;accepted=true;break;
      }
    }
    summary.iterations.push_back(record);if(!accepted)break;
  }
  summary.variables=variables;summary.final_residual=current.residual;
  summary.final_null_norm=current.full_null_norm;
  summary.maximum_displacement=current.displacement;
  summary.maximum_speed=current.speed;summary.edge_deformation=current.edge_deformation;
  summary.center_residual=current.center;summary.continuity_residual=current.continuity;
  summary.causal_excess=current.causal;summary.conjugacy_residual=current.conjugacy;
  summary.conjugacy=current.conjugacy<=1e-10;

  auto shifted=reference;for(auto&point:shifted.constituents)
    point.anchor.x=preflight_wrap(point.anchor.x+3,shifted.electric.L);
  for(int x=0;x<reference.electric.L;++x)for(int y=0;y<reference.electric.L;++y)
    for(int z=0;z<reference.electric.L;++z) {
      const int from=reference.electric.index(x,y,z),to=shifted.electric.index(x+3,y,z);
      shifted.electric.x[to]=reference.electric.x[from];
      shifted.electric.y[to]=reference.electric.y[from];
      shifted.electric.z[to]=reference.electric.z[from];
      shifted.magnetic_half.x[to]=reference.magnetic_half.x[from];
      shifted.magnetic_half.y[to]=reference.magnetic_half.y[from];
      shifted.magnetic_half.z[to]=reference.magnetic_half.z[from];
    }
  const auto shifted_eval=gait_evaluate(shifted,modes,variables,
      summary.evaluation_count,unlimited);
  if(shifted_eval.valid) {
    summary.covariance_residual=0.0;
    for(int mode=0;mode<gait_modes;++mode) {
      const Complex phase=std::exp(Complex(0,-3.0*modes[mode].k.x));
      for(int basis=0;basis<gait_nullity;++basis)
        summary.covariance_residual=std::max(summary.covariance_residual,
            std::abs(shifted_eval.projections[mode][basis]
              -phase*current.projections[mode][basis]));
    }
    summary.covariance=summary.covariance_residual<=1e-10;
  }
  summary.root=current.valid&&current.residual<=1e-10
      &&current.full_null_norm<=1e-10&&current.center<=1e-14
      &&current.speed<=ftd::C_SPEED+1e-12&&current.edge_deformation<=0.10;
}

void causalgait_classify(CausalGaitSummary&summary) {
  const bool execution=summary.parent&&summary.reconstruction&&summary.modes
      &&summary.state&&summary.crosscheck&&summary.evaluations
      &&summary.linear_algebra&&summary.conjugacy
      &&std::isfinite(summary.covariance_residual);
  if(!execution)summary.verdict=
      "CAUSAL_INTERNAL_GAIT_CONTINUATION_EXECUTION_INVALID";
  else if(summary.root&&summary.covariance)summary.verdict=
      "CAUSAL_INTERNAL_GAIT_CANCELLATION_CONSTRUCTIVE";
  else summary.verdict=
      "CAUSAL_OR_EDGE_BOUND_PREVENTS_RESONANCE_CANCELLATION";
}

void causalgait_write(const CausalGaitSummary&summary) {
  const auto directory=std::filesystem::path(__FILE__).parent_path().parent_path()/
      "results/ftd_0713";std::filesystem::create_directories(directory);
  std::ofstream json(directory/"ftd_0713_causal_bound_internal_gait_continuation_v1.json");
  json<<std::setprecision(17)<<"{\n  \"ftd_id\": \"FTD-0713\",\n"
      <<"  \"protocol_sha256\": \""<<causalgait_protocol_sha256<<"\",\n"
      <<"  \"parent_protocol_sha256\": \""<<causalgait_parent_sha256<<"\",\n"
      <<"  \"verdict\": \""<<summary.verdict<<"\",\n"
      <<"  \"production_changed\": false,\n  \"volume\": "<<preflight_volume<<",\n"
      <<"  \"parent_pass\": "<<summary.parent<<",\n"
      <<"  \"reconstruction_pass\": "<<summary.reconstruction<<",\n"
      <<"  \"mode_algebra_pass\": "<<summary.modes<<",\n"
      <<"  \"state_load_pass\": "<<summary.state<<",\n"
      <<"  \"parent_crosscheck_pass\": "<<summary.crosscheck<<",\n"
      <<"  \"evaluation_pass\": "<<summary.evaluations<<",\n"
      <<"  \"linear_algebra_pass\": "<<summary.linear_algebra<<",\n"
      <<"  \"root_pass\": "<<summary.root<<",\n"
      <<"  \"conjugacy_pass\": "<<summary.conjugacy<<",\n"
      <<"  \"covariance_pass\": "<<summary.covariance<<",\n"
      <<"  \"evaluations\": "<<summary.evaluation_count<<",\n"
      <<"  \"accepted_steps\": "<<summary.accepted_steps<<",\n"
      <<"  \"starting_residual\": "<<summary.starting_residual<<",\n"
      <<"  \"starting_null_norm\": "<<summary.starting_null_norm<<",\n"
      <<"  \"final_residual\": "<<summary.final_residual<<",\n"
      <<"  \"final_null_norm\": "<<summary.final_null_norm<<",\n"
      <<"  \"maximum_displacement\": "<<summary.maximum_displacement<<",\n"
      <<"  \"maximum_speed\": "<<summary.maximum_speed<<",\n"
      <<"  \"edge_deformation\": "<<summary.edge_deformation<<",\n"
      <<"  \"center_residual\": "<<summary.center_residual<<",\n"
      <<"  \"continuity_residual\": "<<summary.continuity_residual<<",\n"
      <<"  \"causal_excess\": "<<summary.causal_excess<<",\n"
      <<"  \"conjugacy_residual\": "<<summary.conjugacy_residual<<",\n"
      <<"  \"covariance_residual\": "<<summary.covariance_residual<<"\n}\n";
  std::ofstream iter(directory/"ftd_0713_causal_bound_internal_gait_iterations_v1.csv");
  iter<<"iteration,residual,step,accepted_scale,minimum_pivot,evaluations\n";
  for(const auto&row:summary.iterations)iter<<row.iteration<<','<<std::setprecision(17)
      <<row.residual<<','<<row.step<<','<<row.scale<<','<<row.minimum_pivot<<','
      <<row.evaluations<<'\n';
  std::ofstream state(directory/"ftd_0713_causal_bound_internal_gait_state_v1.csv");
  state<<"particle,dx,dy,dz\n";const auto delta=gait_deltas(summary.variables);
  for(int particle=0;particle<count;++particle)state<<particle<<','
      <<std::setprecision(17)<<delta[particle].x<<','<<delta[particle].y<<','
      <<delta[particle].z<<'\n';
}

} // namespace

int main() {
  CausalGaitSummary summary;const auto results=std::filesystem::path(__FILE__)
      .parent_path().parent_path()/"results";
  summary.parent=gait_parent_fingerprint(
      results/"ftd_0712/ftd_0712_resonant_internal_gait_cancellation_v1.json",
      causalgait_parent_sha256,"BOUNDED_INTERNAL_GAIT_CANNOT_CANCEL_LOCKED_RESONANCE");
  auto reference=gait_reference(summary.reconstruction);
  const auto modes=gait_make_modes(summary.modes);
  summary.state=causalgait_load(summary.variables);
  if(summary.parent&&summary.reconstruction&&summary.modes&&summary.state)
    causalgait_run(summary,reference,modes);
  causalgait_classify(summary);causalgait_write(summary);
  std::cout<<std::setprecision(17)<<"protocol_sha256="<<causalgait_protocol_sha256<<'\n'
      <<"verdict="<<summary.verdict<<'\n'
      <<"residual="<<summary.starting_residual<<" -> "<<summary.final_residual
      <<" null="<<summary.starting_null_norm<<" -> "<<summary.final_null_norm<<'\n'
      <<"steps="<<summary.accepted_steps<<" evaluations="<<summary.evaluation_count
      <<" displacement="<<summary.maximum_displacement
      <<" speed="<<summary.maximum_speed<<" edge="<<summary.edge_deformation<<'\n'
      <<"conjugacy="<<summary.conjugacy_residual
      <<" covariance="<<summary.covariance_residual<<'\n';
  return summary.verdict==
      "CAUSAL_INTERNAL_GAIT_CONTINUATION_EXECUTION_INVALID"?1:0;
}
