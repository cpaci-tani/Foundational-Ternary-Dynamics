// FTD-0712: vary only the internal midpoint gait of the qualified composite
// and test exact cancellation of the eight body-diagonal co-moving null modes.

#define FTD_0704_EMBEDDED
#include "test_connected_dressed_matter_high_speed_preflight.cpp"
#undef FTD_0704_EMBEDDED

#include <array>
#include <complex>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr char gait_protocol_sha256[] =
    "47BC6C8897FFFC0C983FDA6BB73910C6FADE87206544DE74BF63E7F52E344852";
constexpr char gait_parent_rest_sha256[] =
    "D978E8920D8121CA2FC91F3E6B4F68353B98E7B6285B4A82304511EE4177D007";
constexpr char gait_parent_spectral_sha256[] =
    "BD9B05437801F23A7A773F8B455E447BB32CDAD01C91962D2C1E743422921E5F";
constexpr int gait_modes = 4;
constexpr int gait_nullity = 2;
constexpr int gait_residual_dof = 2*gait_modes*gait_nullity;
constexpr int gait_variable_dof = 3*(count-1);
constexpr double gait_fd_step = 1e-5;
constexpr double gait_parent_null_norm = 4.6345148020027714e-4;

using Complex = std::complex<double>;
using CVector = std::array<Complex,6>;
using CMatrix = std::array<CVector,6>;
using GaitResidual = std::array<double,gait_residual_dof>;
using GaitVariables = std::array<double,gait_variable_dof>;
using GaitJacobian =
    std::array<std::array<double,gait_variable_dof>,gait_residual_dof>;
using GaitGram =
    std::array<std::array<double,gait_residual_dof>,gait_residual_dof>;

struct GaitMode {
  Vec3 k{};
  CMatrix curl{};
  CMatrix curl_t{};
  CMatrix relative{};
  std::array<CVector,gait_nullity> left_null{};
};

struct GaitEvaluation {
  bool valid=false;
  double residual=INFINITY;
  double full_null_norm=INFINITY;
  double continuity=INFINITY;
  double causal=INFINITY;
  double center=INFINITY;
  double speed=INFINITY;
  double displacement=INFINITY;
  double edge_deformation=INFINITY;
  double conjugacy=INFINITY;
  GaitResidual values{};
  std::array<CVector,gait_modes> rhs{};
  std::array<std::array<Complex,gait_nullity>,gait_modes> projections{};
  std::array<Vec3,count> delta{};
};

struct GaitIteration {
  int iteration=0;
  int evaluations=0;
  double residual=INFINITY;
  double step=INFINITY;
  double scale=0.0;
  double minimum_pivot=INFINITY;
};

struct GaitSummary {
  bool parent_rest=false,parent_spectral=false,reconstruction=false;
  bool modes=false,rigid_crosscheck=false,evaluations=false;
  bool linear_algebra=true,root=false,conjugacy=false,covariance=false;
  int evaluation_count=0,accepted_steps=0;
  double rigid_null_norm=INFINITY,final_null_norm=INFINITY;
  double final_residual=INFINITY,maximum_displacement=INFINITY;
  double maximum_speed=INFINITY,edge_deformation=INFINITY;
  double center_residual=INFINITY,continuity_residual=INFINITY;
  double causal_excess=INFINITY,conjugacy_residual=INFINITY;
  double covariance_residual=INFINITY;
  GaitVariables variables{};
  std::vector<GaitIteration> iterations;
  std::string verdict="RESONANT_INTERNAL_GAIT_CANCELLATION_EXECUTION_INVALID";
};

bool gait_parent_fingerprint(const std::filesystem::path&path,
                             const char*protocol,const char*verdict) {
  std::ifstream input(path,std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(input)),{});
  return bytes.find(protocol)!=std::string::npos
      &&bytes.find(verdict)!=std::string::npos;
}

ftd::eft::ConnectedMooreBlockState gait_reference(bool&valid) {
  valid=false;auto geometry=preflight_reference();
  if(geometry.electric.L!=preflight_volume)
    return ftd::eft::ConnectedMooreBlockState(0);
  const auto path=std::filesystem::path(__FILE__).parent_path().parent_path()/
      "results/ftd_0708/ftd_0708_l33_full_impulse_rest_solve_state_v1.csv";
  std::ifstream input(path);std::string line;std::getline(input,line);int loaded=0;
  while(std::getline(input,line)) {
    std::stringstream row(line);std::array<std::string,9> fields;
    for(auto&field:fields)std::getline(row,field,',');
    if(fields[0]!="FTD-0708")continue;
    const int particle=std::stoi(fields[1]);
    if(particle<0||particle>=count
        ||std::stoi(fields[2])!=geometry.charges[particle])
      return ftd::eft::ConnectedMooreBlockState(0);
    const Vec3 x{std::stod(fields[3]),std::stod(fields[4]),std::stod(fields[5])};
    geometry.constituents[particle]=preflight_point_at(x,preflight_volume);
    geometry.constituents[particle].momentum={};++loaded;
  }
  if(loaded!=count)return ftd::eft::ConnectedMooreBlockState(0);
  const auto dressed=ftd::eft::redress_connected_moore_block_with_fibre_limit(
      geometry,8,1e-13,4096);
  valid=dressed.valid;return dressed.valid?dressed.state:
      ftd::eft::ConnectedMooreBlockState{};
}

CMatrix gait_identity() {
  CMatrix result{};for(int i=0;i<6;++i)result[i][i]=1.0;return result;
}

CMatrix gait_multiply(const CMatrix&a,const CMatrix&b) {
  CMatrix result{};
  for(int i=0;i<6;++i)for(int j=0;j<6;++j)
    for(int k=0;k<6;++k)result[i][j]+=a[i][k]*b[k][j];
  return result;
}

CVector gait_multiply(const CMatrix&a,const CVector&b) {
  CVector result{};
  for(int i=0;i<6;++i)for(int j=0;j<6;++j)result[i]+=a[i][j]*b[j];
  return result;
}

CMatrix gait_adjoint(const CMatrix&a) {
  CMatrix result{};
  for(int i=0;i<6;++i)for(int j=0;j<6;++j)
    result[i][j]=std::conj(a[j][i]);
  return result;
}

Complex gait_inner(const CVector&a,const CVector&b) {
  Complex result{};for(int i=0;i<6;++i)result+=std::conj(a[i])*b[i];
  return result;
}

double gait_norm(const CVector&a) {
  return std::sqrt(std::max(0.0,gait_inner(a,a).real()));
}

std::vector<CVector> gait_nullspace(CMatrix matrix) {
  constexpr double tolerance=1e-10;
  std::array<int,6> pivot_columns{};pivot_columns.fill(-1);
  std::array<bool,6> pivoted{};int rank=0;
  for(int column=0;column<6&&rank<6;++column) {
    int pivot=rank;
    for(int row=rank+1;row<6;++row)
      if(std::abs(matrix[row][column])>std::abs(matrix[pivot][column]))pivot=row;
    if(std::abs(matrix[pivot][column])<=tolerance)continue;
    std::swap(matrix[pivot],matrix[rank]);
    const Complex diagonal=matrix[rank][column];
    for(int j=0;j<6;++j)matrix[rank][j]/=diagonal;
    for(int row=0;row<6;++row)if(row!=rank) {
      const Complex factor=matrix[row][column];
      for(int j=0;j<6;++j)matrix[row][j]-=factor*matrix[rank][j];
    }
    pivot_columns[rank]=column;pivoted[column]=true;++rank;
  }
  std::vector<CVector> result;
  for(int free_column=0;free_column<6;++free_column)if(!pivoted[free_column]) {
    CVector value{};value[free_column]=1.0;
    for(int row=rank-1;row>=0;--row)
      value[pivot_columns[row]]=-matrix[row][free_column];
    for(const auto&prior:result) {
      const Complex projection=gait_inner(prior,value);
      for(int i=0;i<6;++i)value[i]-=projection*prior[i];
    }
    const double length=gait_norm(value);
    if(length<=tolerance)return {};
    for(auto&entry:value)entry/=length;
    result.push_back(value);
  }
  return result;
}

GaitMode gait_make_mode(int sy,int sz) {
  GaitMode mode;
  mode.k={2.0*ftd::PI/3.0,sy*2.0*ftd::PI/3.0,sz*2.0*ftd::PI/3.0};
  const std::array<Complex,3>d{{
      1.0-std::exp(Complex(0,-mode.k.x)),
      1.0-std::exp(Complex(0,-mode.k.y)),
      1.0-std::exp(Complex(0,-mode.k.z))}};
  mode.curl[0][1]=-d[2];mode.curl[0][2]=d[1];
  mode.curl[1][0]=d[2];mode.curl[1][2]=-d[0];
  mode.curl[2][0]=-d[1];mode.curl[2][1]=d[0];
  mode.curl_t=gait_adjoint(mode.curl);
  CMatrix update=gait_identity();
  const auto cct=gait_multiply(mode.curl,mode.curl_t);
  for(int i=0;i<3;++i)for(int j=0;j<3;++j) {
    update[i][j]=(i==j?1.0:0.0)-ftd::C_SPEED*ftd::C_SPEED*cct[i][j];
    update[i][j+3]=ftd::C_SPEED*mode.curl[i][j];
    update[i+3][j]=-ftd::C_SPEED*mode.curl_t[i][j];
    update[i+3][j+3]=(i==j?1.0:0.0);
  }
  mode.relative=gait_multiply(update,update);
  const Complex phase=std::exp(Complex(0,mode.k.x));
  for(int i=0;i<6;++i)for(int j=0;j<6;++j)
    mode.relative[i][j]=phase*mode.relative[i][j]-(i==j?1.0:0.0);
  const auto null=gait_nullspace(gait_adjoint(mode.relative));
  if(null.size()==gait_nullity)for(int i=0;i<gait_nullity;++i)
    mode.left_null[i]=null[i];
  return mode;
}

std::array<GaitMode,gait_modes> gait_make_modes(bool&valid) {
  std::array<GaitMode,gait_modes> result{{
      gait_make_mode(+1,+1),gait_make_mode(+1,-1),
      gait_make_mode(-1,+1),gait_make_mode(-1,-1)}};
  valid=true;
  for(const auto&mode:result)for(const auto&basis:mode.left_null)
    valid=valid&&std::abs(gait_norm(basis)-1.0)<=1e-10;
  return result;
}

template<typename Field>
CVector gait_field_fourier(const Field&electric,
                           const ftd::eft::MatchedEdgeField&magnetic,
                           const Vec3&k) {
  CVector result{};const int L=electric.L;
  const double normalization=1.0/std::sqrt(static_cast<double>(L)*L*L);
  for(int x=0;x<L;++x)for(int y=0;y<L;++y)for(int z=0;z<L;++z) {
    const int i=electric.index(x,y,z);
    const Complex phase=normalization*std::exp(Complex(0,
        -(k.x*x+k.y*y+k.z*z)));
    result[0]+=electric.x[i]*phase;result[1]+=electric.y[i]*phase;
    result[2]+=electric.z[i]*phase;result[3]+=magnetic.x[i]*phase;
    result[4]+=magnetic.y[i]*phase;result[5]+=magnetic.z[i]*phase;
  }
  return result;
}

CVector gait_current_fourier(
    const std::vector<ftd::eft::QuadraticCoatFaceCurrent>&segments,
    const Vec3&k,int L) {
  CVector result{};
  const double normalization=1.0/std::sqrt(static_cast<double>(L)*L*L);
  for(const auto&segment:segments)for(const auto&entry:segment.sparse_current) {
    const int x=preflight_wrap(entry.face.x,L),y=preflight_wrap(entry.face.y,L);
    const int z=preflight_wrap(entry.face.z,L);
    const Complex phase=normalization*std::exp(Complex(0,
        -(k.x*x+k.y*y+k.z*z)));
    result[entry.axis]+=entry.value*phase;
  }
  return result;
}

std::array<Vec3,count> gait_deltas(const GaitVariables&variables) {
  std::array<Vec3,count> result{};
  for(int particle=0;particle<count-1;++particle) {
    result[particle]={variables[3*particle],variables[3*particle+1],
                      variables[3*particle+2]};
    result[count-1]-=result[particle];
  }
  return result;
}

double gait_edge_deformation(
    const ftd::eft::ConnectedMooreBlockState&reference,
    const std::array<Vec3,count>&delta) {
  double result=0.0;
  for(const auto&edge:reference.edges) {
    const Vec3 d0=position(reference.constituents[edge.second])
        -position(reference.constituents[edge.first]);
    const Vec3 d1=d0+delta[edge.second]-delta[edge.first];
    const double length0=d0.mag(),length1=d1.mag();
    if(!(length0>0.0))return INFINITY;
    result=std::max(result,std::abs(length1/length0-1.0));
  }
  return result;
}

GaitEvaluation gait_evaluate(
    const ftd::eft::ConnectedMooreBlockState&reference,
    const std::array<GaitMode,gait_modes>&modes,
    const GaitVariables&variables,int&evaluation_count,
    double displacement_limit=0.05) {
  ++evaluation_count;GaitEvaluation result;result.delta=gait_deltas(variables);
  Vec3 sum{};result.displacement=0.0;result.speed=0.0;
  for(const auto&delta:result.delta) {
    sum+=delta;result.displacement=std::max(result.displacement,
        std::max({std::abs(delta.x),std::abs(delta.y),std::abs(delta.z)}));
    result.speed=std::max({result.speed,
        (Vec3{0.5,0,0}+delta).mag(),(Vec3{0.5,0,0}-delta).mag()});
  }
  result.center=sum.mag();result.edge_deformation=
      gait_edge_deformation(reference,result.delta);
  std::array<std::vector<ftd::eft::QuadraticCoatFaceCurrent>,2> segments;
  segments[0].reserve(count);segments[1].reserve(count);
  result.continuity=0.0;result.causal=0.0;bool currents=true;
  for(int particle=0;particle<count;++particle) {
    const Vec3 x0=position(reference.constituents[particle]);
    const Vec3 x1=x0+Vec3{0.5,0,0}+result.delta[particle];
    const Vec3 x2=x0+Vec3{1.0,0,0};
    const auto first=ftd::eft::make_quadratic_coat_face_current(
        reference.electric.L,x0,x1,reference.charges[particle],false);
    const auto second=ftd::eft::make_quadratic_coat_face_current(
        reference.electric.L,x1,x2,reference.charges[particle],false);
    currents=currents&&first.valid&&second.valid;
    result.continuity=std::max({result.continuity,
        first.continuity_residual,second.continuity_residual});
    result.causal=std::max({result.causal,
        first.causal_excess,second.causal_excess});
    segments[0].push_back(first);segments[1].push_back(second);
  }
  const bool bounded=result.center<=1e-14&&result.speed<=ftd::C_SPEED+1e-12
      &&result.displacement<=displacement_limit&&result.edge_deformation<=0.10;
  if(!currents||!bounded)return result;

  long double norm2=0.0L;int slot=0;result.conjugacy=0.0;
  for(int mode_index=0;mode_index<gait_modes;++mode_index) {
    const auto&mode=modes[mode_index];
    const CVector base=gait_field_fourier(
        reference.electric,reference.magnetic_half,mode.k);
    CVector state=base;
    for(int tick=0;tick<2;++tick) {
      const CVector current=gait_current_fourier(
          segments[tick],mode.k,reference.electric.L);
      CVector electric{},magnetic{};
      for(int i=0;i<3;++i){electric[i]=state[i];magnetic[i]=state[i+3];}
      const CVector curl_t_e=gait_multiply(mode.curl_t,electric);
      for(int i=0;i<3;++i)magnetic[i]-=ftd::C_SPEED*curl_t_e[i];
      const CVector curl_b=gait_multiply(mode.curl,magnetic);
      for(int i=0;i<3;++i)electric[i]+=ftd::C_SPEED*curl_b[i]-current[i];
      for(int i=0;i<3;++i){state[i]=electric[i];state[i+3]=magnetic[i];}
    }
    const Complex phase=std::exp(Complex(0,mode.k.x));
    for(int i=0;i<6;++i)result.rhs[mode_index][i]=-(phase*state[i]-base[i]);
    for(int basis=0;basis<gait_nullity;++basis) {
      const Complex projection=gait_inner(
          mode.left_null[basis],result.rhs[mode_index]);
      result.projections[mode_index][basis]=projection;
      result.values[slot++]=projection.real();
      result.values[slot++]=projection.imag();
      norm2+=std::norm(projection);
    }

    const Vec3 negative_k=mode.k*(-1.0);
    const CVector base_negative=gait_field_fourier(
        reference.electric,reference.magnetic_half,negative_k);
    GaitMode negative_mode=gait_make_mode(
        mode.k.y>0?-1:+1,mode.k.z>0?-1:+1);
    negative_mode.k=negative_k;
    const std::array<Complex,3>d{{
        1.0-std::exp(Complex(0,-negative_k.x)),
        1.0-std::exp(Complex(0,-negative_k.y)),
        1.0-std::exp(Complex(0,-negative_k.z))}};
    CMatrix curl{};curl[0][1]=-d[2];curl[0][2]=d[1];
    curl[1][0]=d[2];curl[1][2]=-d[0];curl[2][0]=-d[1];curl[2][1]=d[0];
    const CMatrix curl_t=gait_adjoint(curl);CVector negative_state=base_negative;
    for(int tick=0;tick<2;++tick) {
      const CVector current=gait_current_fourier(
          segments[tick],negative_k,reference.electric.L);
      CVector electric{},magnetic{};
      for(int i=0;i<3;++i){electric[i]=negative_state[i];magnetic[i]=negative_state[i+3];}
      const CVector cte=gait_multiply(curl_t,electric);
      for(int i=0;i<3;++i)magnetic[i]-=ftd::C_SPEED*cte[i];
      const CVector cb=gait_multiply(curl,magnetic);
      for(int i=0;i<3;++i)electric[i]+=ftd::C_SPEED*cb[i]-current[i];
      for(int i=0;i<3;++i){negative_state[i]=electric[i];negative_state[i+3]=magnetic[i];}
    }
    const Complex negative_phase=std::exp(Complex(0,negative_k.x));
    for(int i=0;i<6;++i) {
      const Complex negative_rhs=-(negative_phase*negative_state[i]-base_negative[i]);
      result.conjugacy=std::max(result.conjugacy,
          std::abs(negative_rhs-std::conj(result.rhs[mode_index][i])));
    }
  }
  result.residual=0.0;
  for(double value:result.values)result.residual=std::max(result.residual,std::abs(value));
  result.full_null_norm=std::sqrt(2.0*static_cast<double>(norm2));
  result.valid=std::isfinite(result.residual)&&result.continuity<=1e-12
      &&result.causal<=1e-12&&result.conjugacy<=1e-10;
  return result;
}

double gait_variable_norm(const GaitVariables&values) {
  double result=0.0;for(double value:values)result=std::max(result,std::abs(value));
  return result;
}

bool gait_solve_gram(GaitGram matrix,GaitResidual rhs,GaitResidual&solution,
                     double&minimum_pivot) {
  minimum_pivot=INFINITY;
  for(int column=0;column<gait_residual_dof;++column) {
    int pivot=column;
    for(int row=column+1;row<gait_residual_dof;++row)
      if(std::abs(matrix[row][column])>std::abs(matrix[pivot][column]))pivot=row;
    const double value=std::abs(matrix[pivot][column]);
    minimum_pivot=std::min(minimum_pivot,value);
    if(value<=1e-14||!std::isfinite(value))return false;
    if(pivot!=column){std::swap(matrix[pivot],matrix[column]);std::swap(rhs[pivot],rhs[column]);}
    for(int row=column+1;row<gait_residual_dof;++row) {
      const double factor=matrix[row][column]/matrix[column][column];
      for(int k=column;k<gait_residual_dof;++k)matrix[row][k]-=factor*matrix[column][k];
      rhs[row]-=factor*rhs[column];
    }
  }
  for(int row=gait_residual_dof-1;row>=0;--row) {
    double value=rhs[row];
    for(int k=row+1;k<gait_residual_dof;++k)value-=matrix[row][k]*solution[k];
    solution[row]=value/matrix[row][row];
  }
  return true;
}

void gait_run(GaitSummary&summary,
              const ftd::eft::ConnectedMooreBlockState&reference,
              const std::array<GaitMode,gait_modes>&modes) {
  GaitVariables variables{};
  auto current=gait_evaluate(reference,modes,variables,summary.evaluation_count);
  if(!current.valid)return;
  summary.evaluations=true;summary.rigid_null_norm=current.full_null_norm;
  summary.rigid_crosscheck=std::abs(current.full_null_norm-gait_parent_null_norm)<=1e-12;
  if(!summary.rigid_crosscheck)return;
  for(int iteration=0;iteration<8&&current.residual>1e-10;++iteration) {
    GaitJacobian jacobian{};const int before=summary.evaluation_count;
    for(int column=0;column<gait_variable_dof;++column) {
      auto plus=variables,minus=variables;
      plus[column]+=gait_fd_step;minus[column]-=gait_fd_step;
      const auto ep=gait_evaluate(reference,modes,plus,summary.evaluation_count);
      const auto em=gait_evaluate(reference,modes,minus,summary.evaluation_count);
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
      const auto candidate=gait_evaluate(reference,modes,trial,summary.evaluation_count);
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
  const auto shifted_eval=gait_evaluate(
      shifted,modes,variables,summary.evaluation_count);
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
      &&current.speed<=ftd::C_SPEED+1e-12&&current.displacement<=0.05
      &&current.edge_deformation<=0.10;
}

void gait_classify(GaitSummary&summary) {
  const bool execution=summary.parent_rest&&summary.parent_spectral
      &&summary.reconstruction&&summary.modes&&summary.rigid_crosscheck
      &&summary.evaluations&&summary.linear_algebra&&summary.conjugacy
      &&std::isfinite(summary.covariance_residual);
  if(!execution)summary.verdict=
      "RESONANT_INTERNAL_GAIT_CANCELLATION_EXECUTION_INVALID";
  else if(summary.root&&summary.covariance)summary.verdict=
      "RESONANT_INTERNAL_GAIT_CANCELLATION_CONSTRUCTIVE";
  else summary.verdict="BOUNDED_INTERNAL_GAIT_CANNOT_CANCEL_LOCKED_RESONANCE";
}

void gait_write(const GaitSummary&summary) {
  const auto directory=std::filesystem::path(__FILE__).parent_path().parent_path()/
      "results/ftd_0712";std::filesystem::create_directories(directory);
  std::ofstream json(directory/"ftd_0712_resonant_internal_gait_cancellation_v1.json");
  json<<std::setprecision(17)<<"{\n  \"ftd_id\": \"FTD-0712\",\n"
      <<"  \"protocol_sha256\": \""<<gait_protocol_sha256<<"\",\n"
      <<"  \"parent_rest_protocol_sha256\": \""<<gait_parent_rest_sha256<<"\",\n"
      <<"  \"parent_spectral_protocol_sha256\": \""<<gait_parent_spectral_sha256<<"\",\n"
      <<"  \"verdict\": \""<<summary.verdict<<"\",\n"
      <<"  \"production_changed\": false,\n  \"volume\": "<<preflight_volume<<",\n"
      <<"  \"parent_rest_pass\": "<<summary.parent_rest<<",\n"
      <<"  \"parent_spectral_pass\": "<<summary.parent_spectral<<",\n"
      <<"  \"reconstruction_pass\": "<<summary.reconstruction<<",\n"
      <<"  \"mode_algebra_pass\": "<<summary.modes<<",\n"
      <<"  \"rigid_crosscheck_pass\": "<<summary.rigid_crosscheck<<",\n"
      <<"  \"evaluation_pass\": "<<summary.evaluations<<",\n"
      <<"  \"linear_algebra_pass\": "<<summary.linear_algebra<<",\n"
      <<"  \"root_pass\": "<<summary.root<<",\n"
      <<"  \"conjugacy_pass\": "<<summary.conjugacy<<",\n"
      <<"  \"covariance_pass\": "<<summary.covariance<<",\n"
      <<"  \"evaluations\": "<<summary.evaluation_count<<",\n"
      <<"  \"accepted_steps\": "<<summary.accepted_steps<<",\n"
      <<"  \"rigid_null_norm\": "<<summary.rigid_null_norm<<",\n"
      <<"  \"final_null_norm\": "<<summary.final_null_norm<<",\n"
      <<"  \"final_residual\": "<<summary.final_residual<<",\n"
      <<"  \"maximum_displacement\": "<<summary.maximum_displacement<<",\n"
      <<"  \"maximum_speed\": "<<summary.maximum_speed<<",\n"
      <<"  \"edge_deformation\": "<<summary.edge_deformation<<",\n"
      <<"  \"center_residual\": "<<summary.center_residual<<",\n"
      <<"  \"continuity_residual\": "<<summary.continuity_residual<<",\n"
      <<"  \"causal_excess\": "<<summary.causal_excess<<",\n"
      <<"  \"conjugacy_residual\": "<<summary.conjugacy_residual<<",\n"
      <<"  \"covariance_residual\": "<<summary.covariance_residual<<"\n}\n";
  std::ofstream iter(directory/"ftd_0712_resonant_internal_gait_iterations_v1.csv");
  iter<<"iteration,residual,step,accepted_scale,minimum_pivot,evaluations\n";
  for(const auto&row:summary.iterations)iter<<row.iteration<<','<<std::setprecision(17)
      <<row.residual<<','<<row.step<<','<<row.scale<<','<<row.minimum_pivot<<','
      <<row.evaluations<<'\n';
  std::ofstream state(directory/"ftd_0712_resonant_internal_gait_state_v1.csv");
  state<<"particle,dx,dy,dz\n";const auto delta=gait_deltas(summary.variables);
  for(int particle=0;particle<count;++particle)state<<particle<<','
      <<std::setprecision(17)<<delta[particle].x<<','<<delta[particle].y<<','
      <<delta[particle].z<<'\n';
}

} // namespace

#ifndef FTD_0712_EMBEDDED
int main() {
  GaitSummary summary;const auto results=std::filesystem::path(__FILE__)
      .parent_path().parent_path()/"results";
  summary.parent_rest=gait_parent_fingerprint(
      results/"ftd_0708/ftd_0708_l33_full_impulse_rest_solve_v1.json",
      gait_parent_rest_sha256,"L33_FULL_IMPULSE_REST_FIXED_POINT_CONSTRUCTIVE");
  summary.parent_spectral=gait_parent_fingerprint(
      results/"ftd_0711/ftd_0711_comoving_field_fourier_solvability_v1.json",
      gait_parent_spectral_sha256,"FINITE_VOLUME_COMOVING_SOURCE_NULLSPACE_INCOMPATIBLE");
  auto reference=gait_reference(summary.reconstruction);
  const auto modes=gait_make_modes(summary.modes);
  if(summary.parent_rest&&summary.parent_spectral&&summary.reconstruction&&summary.modes)
    gait_run(summary,reference,modes);
  gait_classify(summary);gait_write(summary);
  std::cout<<std::setprecision(17)<<"protocol_sha256="<<gait_protocol_sha256<<'\n'
      <<"verdict="<<summary.verdict<<'\n'
      <<"null="<<summary.rigid_null_norm<<" -> "<<summary.final_null_norm
      <<" residual="<<summary.final_residual<<'\n'
      <<"steps="<<summary.accepted_steps<<" evaluations="<<summary.evaluation_count
      <<" displacement="<<summary.maximum_displacement
      <<" speed="<<summary.maximum_speed<<" edge="<<summary.edge_deformation<<'\n'
      <<"conjugacy="<<summary.conjugacy_residual
      <<" covariance="<<summary.covariance_residual<<'\n';
  return summary.verdict==
      "RESONANT_INTERNAL_GAIT_CANCELLATION_EXECUTION_INVALID"?1:0;
}
#endif
