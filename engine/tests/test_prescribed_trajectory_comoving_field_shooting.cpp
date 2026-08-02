// FTD-0710: solve the complete matched field in the co-moving frame of the
// prescribed two-tick rigid translation, then replay the unchanged reciprocal
// matter action without retuning.

#define FTD_0704_EMBEDDED
#include "test_connected_dressed_matter_high_speed_preflight.cpp"
#undef FTD_0704_EMBEDDED

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr char cofield_protocol_sha256[] =
    "82E52438F5483C5C3A427B31D9B068314778B804C2320EEBFFCA1EA6EE593A4B";
constexpr char cofield_parent_rest_sha256[] =
    "D978E8920D8121CA2FC91F3E6B4F68353B98E7B6285B4A82304511EE4177D007";
constexpr char cofield_parent_orbit_sha256[] =
    "14AE617CE7D5EA4F4617FAB667F34CFE339309512B2D9E2D1BE97C946D47A74E";
constexpr int cofield_restart = 48;
constexpr int cofield_max_iterations = 480;
constexpr double cofield_relative_tolerance = 1e-11;
constexpr double cofield_absolute_tolerance = 1e-11;

struct CoFieldPair {
  ftd::eft::MatchedFaceFlux electric;
  ftd::eft::MatchedEdgeField magnetic;
  explicit CoFieldPair(int L = 0) : electric(L), magnetic(L) {}
};

struct CoFieldCurrents {
  bool valid = false;
  double continuity = INFINITY;
  double causal = INFINITY;
  ftd::eft::MatchedFaceFlux first;
  ftd::eft::MatchedFaceFlux second;
  explicit CoFieldCurrents(int L = 0) : first(L), second(L) {}
};

struct CoFieldGmres {
  bool finite = false;
  bool converged = false;
  bool breakdown = false;
  int iterations = 0;
  double initial_l2 = INFINITY;
  double final_l2 = INFINITY;
  std::vector<double> solution;
  std::vector<double> residual_history;
};

struct CoFieldReplay {
  bool attempted = false;
  bool forward = false;
  bool reverse = false;
  bool covariance_evaluated = false;
  int hops = 0;
  double common = 0.0;
  double energy = 0.0;
  double inverse = INFINITY;
  double covariance = INFINITY;
  double position = INFINITY;
  double momentum = INFINITY;
  double electric = INFINITY;
  double magnetic = INFINITY;
  double complete = INFINITY;
};

struct CoFieldSummary {
  bool parent_rest = false;
  bool parent_orbit = false;
  bool reconstruction = false;
  bool currents = false;
  bool algebra = false;
  bool field_evaluated = false;
  bool field_pass = false;
  bool field_covariance_evaluated = false;
  int dof = 0;
  double current_continuity = INFINITY;
  double current_causal = INFINITY;
  double initial_field_l2 = INFINITY;
  double final_field_l2 = INFINITY;
  double electric_residual = INFINITY;
  double magnetic_residual = INFINITY;
  double complete_field_residual = INFINITY;
  double gauss_before = INFINITY;
  double gauss_after = INFINITY;
  double harmonic_mean = INFINITY;
  double field_covariance = INFINITY;
  double correction_maximum = INFINITY;
  std::vector<double> field_rhs;
  CoFieldGmres gmres;
  CoFieldReplay replay;
  std::string verdict =
      "PRESCRIBED_TRAJECTORY_COMOVING_FIELD_EXECUTION_INVALID";
};

std::size_t cofield_volume(int L) {
  return static_cast<std::size_t>(L)*L*L;
}

bool cofield_parent_fingerprint(const std::filesystem::path& path,
                                const char* protocol,
                                const char* verdict) {
  std::ifstream input(path, std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(input)), {});
  return bytes.find(protocol) != std::string::npos
      && bytes.find(verdict) != std::string::npos;
}

double cofield_max_component(const Vec3& value) {
  return std::max({std::abs(value.x),std::abs(value.y),std::abs(value.z)});
}

ftd::eft::ConnectedMooreBlockState cofield_reference(bool& valid) {
  valid = false;
  auto geometry = preflight_reference();
  if (geometry.electric.L != preflight_volume)
    return ftd::eft::ConnectedMooreBlockState(0);
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0708/ftd_0708_l33_full_impulse_rest_solve_state_v1.csv";
  std::ifstream input(path);
  std::string line;
  std::getline(input,line);
  int loaded = 0;
  while (std::getline(input,line)) {
    std::stringstream row(line);
    std::array<std::string,9> fields;
    for (auto& field : fields) std::getline(row,field,',');
    if (fields[0] != "FTD-0708") continue;
    const int particle = std::stoi(fields[1]);
    if (particle < 0 || particle >= count
        || std::stoi(fields[2]) != geometry.charges[particle])
      return ftd::eft::ConnectedMooreBlockState(0);
    const Vec3 x{std::stod(fields[3]),std::stod(fields[4]),
                 std::stod(fields[5])};
    geometry.constituents[particle] =
        preflight_point_at(x,preflight_volume);
    geometry.constituents[particle].momentum = {};
    ++loaded;
  }
  if (loaded != count) return ftd::eft::ConnectedMooreBlockState(0);
  const auto dressed = ftd::eft::redress_connected_moore_block_with_fibre_limit(
      geometry,8,1e-13,4096);
  valid = dressed.valid;
  return dressed.valid ? dressed.state
                       : ftd::eft::ConnectedMooreBlockState{};
}

void cofield_add_segment(ftd::eft::MatchedFaceFlux& target,
                         const ftd::eft::QuadraticCoatFaceCurrent& segment) {
  for (const auto& entry : segment.sparse_current) {
    const std::size_t i = static_cast<std::size_t>(target.index(
        entry.face.x,entry.face.y,entry.face.z));
    auto& component = entry.axis == 0 ? target.x
        : (entry.axis == 1 ? target.y : target.z);
    component[i] += entry.value;
  }
}

CoFieldCurrents cofield_make_currents(
    const ftd::eft::ConnectedMooreBlockState& geometry) {
  const int L = geometry.electric.L;
  CoFieldCurrents result(L);
  result.valid = geometry.constituents.size() == geometry.charges.size();
  result.continuity = 0.0;
  result.causal = 0.0;
  for (std::size_t a = 0; a < geometry.constituents.size(); ++a) {
    const Vec3 x0 = position(geometry.constituents[a]);
    const Vec3 x1 = x0+Vec3{0.5,0.0,0.0};
    const Vec3 x2 = x0+Vec3{1.0,0.0,0.0};
    const auto first = ftd::eft::make_quadratic_coat_face_current(
        L,x0,x1,geometry.charges[a],false);
    const auto second = ftd::eft::make_quadratic_coat_face_current(
        L,x1,x2,geometry.charges[a],false);
    result.valid = result.valid && first.valid && second.valid;
    result.continuity = std::max({result.continuity,
        first.continuity_residual,second.continuity_residual});
    result.causal = std::max({result.causal,
        first.causal_excess,second.causal_excess});
    if (first.valid) cofield_add_segment(result.first,first);
    if (second.valid) cofield_add_segment(result.second,second);
  }
  result.valid = result.valid && result.continuity <= 1e-12
      && result.causal <= 1e-12;
  return result;
}

template <typename Field>
void cofield_add_scaled(Field& target, const Field& source, double scale) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += scale*source.x[i];
    target.y[i] += scale*source.y[i];
    target.z[i] += scale*source.z[i];
  }
}

template <typename Field>
Field cofield_translate_field(const Field& source, int dx) {
  Field result(source.L);
  const int L = source.L;
  for (int x=0;x<L;++x) for(int y=0;y<L;++y) for(int z=0;z<L;++z) {
    const int from=source.index(x,y,z),to=result.index(x+dx,y,z);
    result.x[to]=source.x[from];
    result.y[to]=source.y[from];
    result.z[to]=source.z[from];
  }
  return result;
}

CoFieldPair cofield_translate(const CoFieldPair& source, int dx) {
  CoFieldPair result(source.electric.L);
  result.electric=cofield_translate_field(source.electric,dx);
  result.magnetic=cofield_translate_field(source.magnetic,dx);
  return result;
}

CoFieldPair cofield_two_ticks(const CoFieldPair& initial,
                              const CoFieldCurrents* currents) {
  const double lambda = ftd::C_SPEED;
  CoFieldPair state = initial;
  const std::array<const ftd::eft::MatchedFaceFlux*,2> current{{
      currents ? &currents->first : nullptr,
      currents ? &currents->second : nullptr}};
  for (int tick=0;tick<2;++tick) {
    const auto curl_t = ftd::eft::matched_curl_adjoint(state.electric);
    cofield_add_scaled(state.magnetic,curl_t,-lambda);
    const auto curl = ftd::eft::matched_curl(state.magnetic);
    cofield_add_scaled(state.electric,curl,+lambda);
    if (current[tick]) cofield_add_scaled(state.electric,*current[tick],-1.0);
  }
  return state;
}

std::vector<double> cofield_pack(const CoFieldPair& field) {
  const std::size_t V=field.electric.x.size();
  std::vector<double> result(6*V);
  const std::array<const std::vector<double>*,6> parts{{
      &field.electric.x,&field.electric.y,&field.electric.z,
      &field.magnetic.x,&field.magnetic.y,&field.magnetic.z}};
  for (std::size_t part=0;part<parts.size();++part)
    std::copy(parts[part]->begin(),parts[part]->end(),
              result.begin()+static_cast<std::ptrdiff_t>(part*V));
  return result;
}

CoFieldPair cofield_unpack(const std::vector<double>& values, int L) {
  CoFieldPair result(L);
  const std::size_t V=cofield_volume(L);
  if (values.size()!=6*V) return CoFieldPair(0);
  const std::array<std::vector<double>*,6> parts{{
      &result.electric.x,&result.electric.y,&result.electric.z,
      &result.magnetic.x,&result.magnetic.y,&result.magnetic.z}};
  for (std::size_t part=0;part<parts.size();++part)
    std::copy(values.begin()+static_cast<std::ptrdiff_t>(part*V),
              values.begin()+static_cast<std::ptrdiff_t>((part+1)*V),
              parts[part]->begin());
  return result;
}

std::vector<double> cofield_linear_operator(const std::vector<double>& values,
                                             int L) {
  const auto input=cofield_unpack(values,L);
  auto output=cofield_translate(cofield_two_ticks(input,nullptr),-1);
  auto packed=cofield_pack(output);
  for(std::size_t i=0;i<packed.size();++i)packed[i]-=values[i];
  return packed;
}

long double cofield_dot(const std::vector<double>& a,
                        const std::vector<double>& b) {
  long double result=0.0L;
  for(std::size_t i=0;i<a.size();++i)
    result+=static_cast<long double>(a[i])*b[i];
  return result;
}

double cofield_l2(const std::vector<double>& values) {
  return std::sqrt(static_cast<double>(cofield_dot(values,values)));
}

double cofield_infinity(const std::vector<double>& values) {
  double result=0.0;
  for(double value:values)result=std::max(result,std::abs(value));
  return result;
}

bool cofield_finite(const std::vector<double>& values) {
  return std::all_of(values.begin(),values.end(),
      [](double value){return std::isfinite(value);});
}

CoFieldGmres cofield_gmres(const std::vector<double>& b,int L) {
  CoFieldGmres out;
  const std::size_t n=b.size();
  out.solution.assign(n,0.0);
  out.initial_l2=cofield_l2(b);
  const double tolerance=std::max(cofield_absolute_tolerance,
      cofield_relative_tolerance*out.initial_l2);
  out.residual_history.push_back(out.initial_l2);
  if(!std::isfinite(out.initial_l2)){return out;}
  if(out.initial_l2<=tolerance){
    out.finite=true;out.converged=true;out.final_l2=out.initial_l2;return out;
  }
  while(out.iterations<cofield_max_iterations&&!out.converged) {
    auto Ax=cofield_linear_operator(out.solution,L);
    std::vector<double> residual(n);
    for(std::size_t i=0;i<n;++i)residual[i]=b[i]-Ax[i];
    const double beta=cofield_l2(residual);
    if(!std::isfinite(beta))return out;
    if(beta<=tolerance){out.converged=true;break;}
    const int cycle=std::min(cofield_restart,
        cofield_max_iterations-out.iterations);
    std::vector<std::vector<double>> basis;
    basis.reserve(static_cast<std::size_t>(cycle+1));
    for(double&value:residual)value/=beta;
    basis.push_back(std::move(residual));
    std::vector<std::vector<double>> h(static_cast<std::size_t>(cycle+1),
        std::vector<double>(static_cast<std::size_t>(cycle),0.0));
    std::vector<double> cs(cycle,0.0),sn(cycle,0.0),g(cycle+1,0.0);
    g[0]=beta;
    int used=0;
    for(int column=0;column<cycle;++column) {
      auto w=cofield_linear_operator(basis[column],L);
      if(!cofield_finite(w))return out;
      for(int pass=0;pass<2;++pass)
        for(int row=0;row<=column;++row) {
          const double projection=static_cast<double>(cofield_dot(w,basis[row]));
          h[row][column]+=projection;
          for(std::size_t i=0;i<n;++i)w[i]-=projection*basis[row][i];
        }
      h[column+1][column]=cofield_l2(w);
      if(h[column+1][column]>1e-15) {
        for(double&value:w)value/=h[column+1][column];
        basis.push_back(std::move(w));
      } else {
        basis.push_back(std::vector<double>(n,0.0));
        out.breakdown=true;
      }
      for(int row=0;row<column;++row) {
        const double first=cs[row]*h[row][column]+sn[row]*h[row+1][column];
        const double second=-sn[row]*h[row][column]+cs[row]*h[row+1][column];
        h[row][column]=first;h[row+1][column]=second;
      }
      const double diagonal=std::hypot(h[column][column],h[column+1][column]);
      if(diagonal<=1e-30||!std::isfinite(diagonal))return out;
      cs[column]=h[column][column]/diagonal;
      sn[column]=h[column+1][column]/diagonal;
      h[column][column]=diagonal;h[column+1][column]=0.0;
      const double first=cs[column]*g[column]+sn[column]*g[column+1];
      g[column+1]=-sn[column]*g[column]+cs[column]*g[column+1];
      g[column]=first;
      ++out.iterations;++used;
      out.residual_history.push_back(std::abs(g[column+1]));
      if(std::abs(g[column+1])<=tolerance||out.breakdown)break;
    }
    std::vector<double> y(static_cast<std::size_t>(used),0.0);
    for(int row=used-1;row>=0;--row) {
      double value=g[row];
      for(int column=row+1;column<used;++column)value-=h[row][column]*y[column];
      if(std::abs(h[row][row])<=1e-30)return out;
      y[row]=value/h[row][row];
    }
    for(int column=0;column<used;++column)
      for(std::size_t i=0;i<n;++i)
        out.solution[i]+=y[column]*basis[column][i];
    if(!cofield_finite(out.solution))return out;
    auto check=cofield_linear_operator(out.solution,L);
    for(std::size_t i=0;i<n;++i)check[i]=b[i]-check[i];
    out.final_l2=cofield_l2(check);
    out.residual_history.push_back(out.final_l2);
    out.converged=out.final_l2<=tolerance;
    if(out.breakdown&&!out.converged)break;
  }
  auto check=cofield_linear_operator(out.solution,L);
  for(std::size_t i=0;i<n;++i)check[i]=b[i]-check[i];
  out.final_l2=cofield_l2(check);
  out.finite=std::isfinite(out.final_l2)&&cofield_finite(out.solution);
  out.converged=out.finite&&out.final_l2<=std::max(
      cofield_absolute_tolerance,cofield_relative_tolerance*out.initial_l2);
  return out;
}

std::vector<double> cofield_density(
    const ftd::eft::ConnectedMooreBlockState& state,double shift) {
  const int L=state.electric.L;
  std::vector<double> density(cofield_volume(L),0.0);
  for(std::size_t a=0;a<state.constituents.size();++a) {
    const auto coat=ftd::eft::make_quadratic_polarity_coat(
        position(state.constituents[a])+Vec3{shift,0,0},state.charges[a]);
    if(!coat.valid)return {};
    for(std::size_t item=0;item<coat.weight_count;++item) {
      const auto&entry=coat.weights[item];
      density[static_cast<std::size_t>(state.electric.index(
          entry.site.x,entry.site.y,entry.site.z))]+=entry.weight;
    }
  }
  return density;
}

double cofield_harmonic_mean(const CoFieldPair& field) {
  double result=0.0;
  const std::array<const std::vector<double>*,6> parts{{
      &field.electric.x,&field.electric.y,&field.electric.z,
      &field.magnetic.x,&field.magnetic.y,&field.magnetic.z}};
  for(const auto*part:parts) {
    const long double sum=std::accumulate(part->begin(),part->end(),0.0L);
    result=std::max(result,std::abs(static_cast<double>(sum/part->size())));
  }
  return result;
}

ftd::eft::ConnectedMooreBlockState cofield_translate_state(
    const ftd::eft::ConnectedMooreBlockState& source,int dx) {
  auto result=source;
  for(auto&point:result.constituents)
    point.anchor.x=preflight_wrap(point.anchor.x+dx,source.electric.L);
  result.electric=cofield_translate_field(source.electric,dx);
  result.magnetic_half=cofield_translate_field(source.magnetic_half,dx);
  return result;
}

double cofield_position_difference(
    const ftd::eft::ConnectedMooreBlockState&a,
    const ftd::eft::ConnectedMooreBlockState&b) {
  if(a.constituents.size()!=b.constituents.size())return INFINITY;
  double result=0.0,L=a.electric.L;
  for(std::size_t i=0;i<a.constituents.size();++i) {
    Vec3 d=position(a.constituents[i])-position(b.constituents[i]);
    d.x-=std::round(d.x/L)*L;d.y-=std::round(d.y/L)*L;
    d.z-=std::round(d.z/L)*L;
    result=std::max(result,cofield_max_component(d));
  }
  return result;
}

double cofield_momentum_difference(
    const ftd::eft::ConnectedMooreBlockState&a,
    const ftd::eft::ConnectedMooreBlockState&b) {
  if(a.constituents.size()!=b.constituents.size())return INFINITY;
  double result=0.0;
  for(std::size_t i=0;i<a.constituents.size();++i)
    result=std::max(result,cofield_max_component(
        a.constituents[i].momentum-b.constituents[i].momentum));
  return result;
}

CoFieldReplay cofield_replay(
    const ftd::eft::ConnectedMooreBlockState& initial,double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options) {
  CoFieldReplay result;result.attempted=true;
  const double energy0=preflight_energy(initial,beta,options);
  auto state=initial;
  ftd::eft::ConnectedMooreBlockSolveCache forward_cache,reverse_cache;
  result.forward=true;
  for(int tick=0;tick<2;++tick) {
    const auto step=ftd::eft::solve_connected_moore_block_forward(
        state,options,&forward_cache);
    const double common=common_residual(step);
    if(!step.valid||!step.common_action_gates_pass||common>1e-10) {
      result.forward=false;break;
    }
    state=step.later;result.hops+=step.site_hops;
    result.common=std::max(result.common,common);
    result.energy=std::max(result.energy,
        std::abs(preflight_energy(state,beta,options)-energy0));
  }
  if(!result.forward)return result;
  const auto target=cofield_translate_state(initial,1);
  result.position=cofield_position_difference(state,target);
  result.momentum=cofield_momentum_difference(state,target);
  result.electric=ftd::eft::matched_face_max_difference(
      state.electric,target.electric);
  result.magnetic=ftd::eft::matched_edge_max_difference(
      state.magnetic_half,target.magnetic_half);
  result.complete=ftd::eft::connected_moore_block_state_max_difference(
      state,target);
  result.reverse=true;
  for(int tick=0;tick<2;++tick) {
    const auto step=ftd::eft::solve_connected_moore_block_reverse(
        state,options,&reverse_cache);
    const double common=common_residual(step);
    if(!step.valid||!step.common_action_gates_pass||common>1e-10) {
      result.reverse=false;break;
    }
    state=step.earlier;
    result.common=std::max(result.common,common);
    result.energy=std::max(result.energy,
        std::abs(preflight_energy(state,beta,options)-energy0));
  }
  if(result.reverse)result.inverse=
      ftd::eft::connected_moore_block_state_max_difference(initial,state);
  return result;
}

void cofield_run(CoFieldSummary& summary,
                 const ftd::eft::ConnectedMooreBlockState& reference,
                 double beta,
                 const ftd::eft::ConnectedMooreBlockOptions& options) {
  const int L=reference.electric.L;
  const auto currents=cofield_make_currents(reference);
  summary.currents=currents.valid;
  summary.current_continuity=currents.continuity;
  summary.current_causal=currents.causal;
  if(!currents.valid)return;

  CoFieldPair base(L);base.electric=reference.electric;
  const auto affine=cofield_translate(cofield_two_ticks(base,&currents),-1);
  auto b=cofield_pack(affine),base_vector=cofield_pack(base);
  for(std::size_t i=0;i<b.size();++i)b[i]=-(b[i]-base_vector[i]);
  summary.dof=static_cast<int>(b.size());
  summary.field_rhs=b;
  summary.gmres=cofield_gmres(b,L);
  summary.algebra=summary.gmres.finite;
  summary.initial_field_l2=summary.gmres.initial_l2;
  summary.final_field_l2=summary.gmres.final_l2;
  if(!summary.algebra)return;

  const auto correction=cofield_unpack(summary.gmres.solution,L);
  CoFieldPair solved=base;
  cofield_add_scaled(solved.electric,correction.electric,1.0);
  cofield_add_scaled(solved.magnetic,correction.magnetic,1.0);
  summary.correction_maximum=cofield_infinity(summary.gmres.solution);
  const auto evolved=cofield_two_ticks(solved,&currents);
  const auto relative=cofield_translate(evolved,-1);
  summary.electric_residual=ftd::eft::matched_face_max_difference(
      relative.electric,solved.electric);
  summary.magnetic_residual=ftd::eft::matched_edge_max_difference(
      relative.magnetic,solved.magnetic);
  summary.complete_field_residual=std::max(
      summary.electric_residual,summary.magnetic_residual);
  const auto rho0=cofield_density(reference,0.0);
  const auto rho2=cofield_density(reference,1.0);
  if(rho0.empty()||rho2.empty())return;
  summary.gauss_before=ftd::eft::max_fractional_gauss_residual(
      solved.electric,rho0);
  summary.gauss_after=ftd::eft::max_fractional_gauss_residual(
      evolved.electric,rho2);
  summary.harmonic_mean=cofield_harmonic_mean(solved);
  summary.field_evaluated=std::isfinite(summary.complete_field_residual)
      &&std::isfinite(summary.gauss_before)&&std::isfinite(summary.gauss_after)
      &&std::isfinite(summary.harmonic_mean);

  auto shifted_reference=cofield_translate_state(reference,3);
  const auto shifted_currents=cofield_make_currents(shifted_reference);
  const auto shifted_solved=cofield_translate(solved,3);
  if(shifted_currents.valid) {
    const auto shifted_relative=cofield_translate(
        cofield_two_ticks(shifted_solved,&shifted_currents),-1);
    const auto expected=cofield_translate(relative,3);
    summary.field_covariance=std::max(
        ftd::eft::matched_face_max_difference(
            shifted_relative.electric,expected.electric),
        ftd::eft::matched_edge_max_difference(
            shifted_relative.magnetic,expected.magnetic));
    summary.field_covariance_evaluated=std::isfinite(summary.field_covariance);
  }
  summary.field_pass=summary.gmres.converged&&summary.field_evaluated
      &&summary.field_covariance_evaluated
      &&summary.electric_residual<=1e-9&&summary.magnetic_residual<=1e-9
      &&summary.gauss_before<=1e-10&&summary.gauss_after<=1e-10
      &&summary.harmonic_mean<=1e-12&&summary.field_covariance<=1e-9;
  if(!summary.field_pass)return;

  auto moving=reference;moving.electric=solved.electric;
  moving.magnetic_half=solved.magnetic;
  const Vec3 momentum=ftd::eft::production_flat_momentum({0.5,0,0});
  for(auto&point:moving.constituents)point.momentum=momentum;
  summary.replay=cofield_replay(moving,beta,options);
  if(!summary.replay.forward)return;
  auto shifted_moving=cofield_translate_state(moving,3);
  const auto shifted_replay=cofield_replay(shifted_moving,beta,options);
  if(shifted_replay.forward) {
    summary.replay.covariance=std::max({
        std::abs(shifted_replay.position-summary.replay.position),
        std::abs(shifted_replay.momentum-summary.replay.momentum),
        std::abs(shifted_replay.electric-summary.replay.electric),
        std::abs(shifted_replay.magnetic-summary.replay.magnetic),
        std::abs(shifted_replay.complete-summary.replay.complete)});
    summary.replay.covariance_evaluated=std::isfinite(
        summary.replay.covariance);
  }
}

void cofield_classify(CoFieldSummary& summary) {
  const bool base_valid=summary.parent_rest&&summary.parent_orbit
      &&summary.reconstruction&&summary.currents&&summary.algebra
      &&summary.field_evaluated&&summary.field_covariance_evaluated;
  if(!base_valid) {
    summary.verdict="PRESCRIBED_TRAJECTORY_COMOVING_FIELD_EXECUTION_INVALID";
    return;
  }
  if(!summary.field_pass) {
    summary.verdict="PRESCRIBED_TRAJECTORY_FIELD_SHOOTING_NOT_RESOLVED";
    return;
  }
  const bool replay_evaluated=summary.replay.attempted&&summary.replay.forward
      &&summary.replay.reverse&&summary.replay.covariance_evaluated
      &&std::isfinite(summary.replay.inverse);
  if(!replay_evaluated) {
    summary.verdict="PRESCRIBED_TRAJECTORY_COMOVING_FIELD_EXECUTION_INVALID";
    return;
  }
  const bool reciprocal_gates=summary.replay.common<=1e-10
      &&summary.replay.energy<=1e-10&&summary.replay.inverse<=1e-9
      &&summary.replay.covariance<=1e-9;
  if(reciprocal_gates&&summary.replay.complete<=1e-9)
    summary.verdict=
        "PRESCRIBED_TRAJECTORY_COMPLETE_RELATIVE_ORBIT_CANDIDATE";
  else
    summary.verdict=
        "COMOVING_FIELD_SOLVED_RIGID_MATTER_NOT_SELF_CONSISTENT";
}

void cofield_write(const CoFieldSummary& summary) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results/ftd_0710";
  std::filesystem::create_directories(directory);
  const auto write_number=[](std::ostream& output,double value) {
    if(std::isfinite(value))output<<value;
    else output<<"null";
  };
  std::ofstream json(directory/
      "ftd_0710_prescribed_trajectory_comoving_field_shooting_v1.json");
  json<<std::setprecision(17)<<"{\n  \"ftd_id\": \"FTD-0710\",\n"
      <<"  \"protocol_sha256\": \""<<cofield_protocol_sha256<<"\",\n"
      <<"  \"parent_rest_protocol_sha256\": \""
      <<cofield_parent_rest_sha256<<"\",\n"
      <<"  \"parent_orbit_protocol_sha256\": \""
      <<cofield_parent_orbit_sha256<<"\",\n"
      <<"  \"verdict\": \""<<summary.verdict<<"\",\n"
      <<"  \"production_changed\": false,\n"
      <<"  \"volume\": "<<preflight_volume<<",\n"
      <<"  \"field_dof\": "<<summary.dof<<",\n"
      <<"  \"parent_rest_pass\": "<<summary.parent_rest<<",\n"
      <<"  \"parent_orbit_pass\": "<<summary.parent_orbit<<",\n"
      <<"  \"reconstruction_pass\": "<<summary.reconstruction<<",\n"
      <<"  \"current_pass\": "<<summary.currents<<",\n"
      <<"  \"algebra_pass\": "<<summary.algebra<<",\n"
      <<"  \"gmres_converged\": "<<summary.gmres.converged<<",\n"
      <<"  \"gmres_breakdown\": "<<summary.gmres.breakdown<<",\n"
      <<"  \"gmres_iterations\": "<<summary.gmres.iterations<<",\n"
      <<"  \"field_pass\": "<<summary.field_pass<<",\n"
      <<"  \"current_continuity_residual\": "
      <<summary.current_continuity<<",\n"
      <<"  \"current_causal_excess\": "<<summary.current_causal<<",\n"
      <<"  \"initial_field_l2_residual\": "
      <<summary.initial_field_l2<<",\n"
      <<"  \"final_field_l2_residual\": "
      <<summary.final_field_l2<<",\n"
      <<"  \"electric_residual\": "<<summary.electric_residual<<",\n"
      <<"  \"magnetic_residual\": "<<summary.magnetic_residual<<",\n"
      <<"  \"complete_field_residual\": "
      <<summary.complete_field_residual<<",\n"
      <<"  \"gauss_before_residual\": "<<summary.gauss_before<<",\n"
      <<"  \"gauss_after_residual\": "<<summary.gauss_after<<",\n"
      <<"  \"harmonic_mean_residual\": "<<summary.harmonic_mean<<",\n"
      <<"  \"field_covariance_residual\": "
      <<summary.field_covariance<<",\n"
      <<"  \"correction_maximum\": "<<summary.correction_maximum<<",\n"
      <<"  \"reciprocal_attempted\": "<<summary.replay.attempted<<",\n"
      <<"  \"reciprocal_forward_pass\": "<<summary.replay.forward<<",\n"
      <<"  \"reciprocal_reverse_pass\": "<<summary.replay.reverse<<",\n"
      <<"  \"reciprocal_hops\": "<<summary.replay.hops<<",\n"
      <<"  \"reciprocal_common_residual\": "<<summary.replay.common<<",\n"
      <<"  \"reciprocal_energy_drift\": "<<summary.replay.energy<<",\n"
      <<"  \"reciprocal_inverse_residual\": ";
  write_number(json,summary.replay.inverse);json<<",\n"
      <<"  \"reciprocal_covariance_residual\": ";
  write_number(json,summary.replay.covariance);json<<",\n"
      <<"  \"position_residual\": ";
  write_number(json,summary.replay.position);json<<",\n"
      <<"  \"momentum_residual\": ";
  write_number(json,summary.replay.momentum);json<<",\n"
      <<"  \"reciprocal_electric_residual\": ";
  write_number(json,summary.replay.electric);json<<",\n"
      <<"  \"reciprocal_magnetic_residual\": ";
  write_number(json,summary.replay.magnetic);json<<",\n"
      <<"  \"complete_relative_orbit_residual\": ";
  write_number(json,summary.replay.complete);json<<"\n}\n";
  std::ofstream csv(directory/
      "ftd_0710_prescribed_trajectory_comoving_field_gmres_v1.csv");
  csv<<"sample,residual_l2\n";
  for(std::size_t i=0;i<summary.gmres.residual_history.size();++i)
    csv<<i<<','<<std::setprecision(17)<<summary.gmres.residual_history[i]<<'\n';
  std::ofstream rhs(directory/
      "ftd_0710_prescribed_trajectory_comoving_field_rhs_v1.csv");
  rhs<<"x,y,z,rhs_electric_x,rhs_electric_y,rhs_electric_z,"
      <<"rhs_magnetic_x,rhs_magnetic_y,rhs_magnetic_z,"
      <<"gmres_electric_x,gmres_electric_y,gmres_electric_z,"
      <<"gmres_magnetic_x,gmres_magnetic_y,gmres_magnetic_z\n";
  const std::size_t V=cofield_volume(preflight_volume);
  if(summary.field_rhs.size()==6*V)
    for(int x=0;x<preflight_volume;++x)
      for(int y=0;y<preflight_volume;++y)
        for(int z=0;z<preflight_volume;++z) {
          const std::size_t i=static_cast<std::size_t>(
              (x*preflight_volume+y)*preflight_volume+z);
          rhs<<x<<','<<y<<','<<z;
          for(int component=0;component<6;++component)
            rhs<<','<<std::setprecision(17)
               <<summary.field_rhs[static_cast<std::size_t>(component)*V+i];
          for(int component=0;component<6;++component)
            rhs<<','<<std::setprecision(17)
               <<summary.gmres.solution[
                   static_cast<std::size_t>(component)*V+i];
          rhs<<'\n';
        }
}

} // namespace

int main() {
  CoFieldSummary summary;
  const auto results=std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results";
  summary.parent_rest=cofield_parent_fingerprint(
      results/"ftd_0708/ftd_0708_l33_full_impulse_rest_solve_v1.json",
      cofield_parent_rest_sha256,
      "L33_FULL_IMPULSE_REST_FIXED_POINT_CONSTRUCTIVE");
  summary.parent_orbit=cofield_parent_fingerprint(
      results/"ftd_0709/ftd_0709_rest_qualified_moving_dressing_relative_orbit_v1.json",
      cofield_parent_orbit_sha256,
      "REST_QUALIFIED_CORE_TRANSLATES_WITHOUT_COMPLETE_MOVING_DRESSING");
  auto reference=cofield_reference(summary.reconstruction);
  const auto normalization=ftd::eft::measure_face_flux_normalization();
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart=true;
  options.use_sparse_local_current=true;
  options.use_local_residual_evaluation=true;
  if(summary.parent_rest&&summary.parent_orbit&&summary.reconstruction
      &&normalization.valid)
    cofield_run(summary,reference,
        normalization.mapped_field_work_coefficient,options);
  cofield_classify(summary);
  cofield_write(summary);
  std::cout<<std::setprecision(17)
      <<"protocol_sha256="<<cofield_protocol_sha256<<'\n'
      <<"verdict="<<summary.verdict<<'\n'
      <<"dof="<<summary.dof<<" gmres="<<summary.gmres.converged
      <<" iterations="<<summary.gmres.iterations
      <<" l2="<<summary.initial_field_l2<<" -> "
      <<summary.final_field_l2<<'\n'
      <<"field electric="<<summary.electric_residual
      <<" magnetic="<<summary.magnetic_residual
      <<" gauss="<<summary.gauss_before<<","<<summary.gauss_after
      <<" covariance="<<summary.field_covariance<<'\n'
      <<"replay complete="<<summary.replay.complete
      <<" position="<<summary.replay.position
      <<" momentum="<<summary.replay.momentum
      <<" inverse="<<summary.replay.inverse<<'\n';
  return summary.verdict==
      "PRESCRIBED_TRAJECTORY_COMOVING_FIELD_EXECUTION_INVALID"?1:0;
}
