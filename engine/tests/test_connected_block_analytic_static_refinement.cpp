// FTD-0638: full-coordinate Newton refinement using the FTD-0637 analytic jet.

#define main ftd_0637_embedded_main
#include "test_connected_block_analytic_envelope_hessian.cpp"
#undef main

namespace {
constexpr char refinement_protocol_sha256[] =
    "2E74799DB0372137071B5CF840D5C330AF4A3FEDE14EE9C4972B2C1796D056BA";
constexpr char refinement_parent_result_sha256[] =
    "0F4F8E4539735E0B0515D691A314F43374A13842EF6744A0BED88C0A3734D81A";

struct RefinedArm {
  bool valid=false,stationary=false,positive=false,sector_preserved=false;
  int orientation=0,accepted_steps=0;
  double initial_energy=INFINITY,final_energy=INFINITY,energy_change=0.0;
  double initial_gradient=INFINITY,final_gradient=INFINITY;
  double final_gradient_comparison=INFINITY;
  double min_eigen=INFINITY,max_eigen=INFINITY;
  double maximum_displacement=0.0;
  ftd::eft::ConnectedMooreBlockState initial_state{L},final_state{L};
  Arm analytic;
};
struct RefinedSummary {
  bool parent=false,normalization=false,covariance=false;
  double beta=0.0,energy_covariance=INFINITY,spectrum_covariance=INFINITY,
      displacement_covariance=INFINITY;
  std::string verdict="CONNECTED_BLOCK_ANALYTIC_REFINEMENT_EXECUTION_INVALID";
  std::vector<RefinedArm> arms;
};

bool refinement_parent_fingerprint() {
  const auto path=std::filesystem::path(__FILE__).parent_path().parent_path()/
      "results/ftd_0637/ftd_0637_connected_block_analytic_envelope_hessian_v1.json";
  std::ifstream input(path,std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(input)),{});
  return bytes.find(protocol_sha256)!=std::string::npos
      && bytes.find("CONNECTED_BLOCK_ANALYTIC_NONSTATIONARY")!=std::string::npos;
}

Poisson solve_poisson_strict(const std::vector<double>& source) {
  Poisson result;result.potential.assign(V,0.0);
  std::vector<double> rhs=source,residual(V),direction(V),image(V);
  const long double mean=std::accumulate(rhs.begin(),rhs.end(),0.0L)/V;
  for(double& value:rhs)value-=static_cast<double>(mean);
  residual=direction=rhs;long double rr=dot(residual,residual);
  for(int iteration=1;iteration<=4096;++iteration){double recursive=0;for(double value:residual)recursive=std::max(recursive,std::abs(value));if(recursive<=1e-14){result.iterations=iteration-1;break;}negative_laplacian(direction,image);const long double denominator=dot(direction,image);if(!(denominator>0))break;const long double alpha=rr/denominator;for(std::size_t i=0;i<V;++i){result.potential[i]+=static_cast<double>(alpha*direction[i]);residual[i]-=static_cast<double>(alpha*image[i]);}const long double next=dot(residual,residual),ratio=next/rr;for(std::size_t i=0;i<V;++i)direction[i]=residual[i]+static_cast<double>(ratio*direction[i]);rr=next;result.iterations=iteration;}
  negative_laplacian(result.potential,image);result.residual=0;for(std::size_t i=0;i<V;++i)result.residual=std::max(result.residual,std::abs(image[i]-rhs[i]));result.valid=result.residual<=1e-13;return result;
}

Poisson solve_poisson_scaled(const std::vector<double>& source) {
  double scale=0;for(double value:source)scale=std::max(scale,std::abs(value));
  if(scale==0){Poisson result;result.valid=true;result.residual=0;result.potential.assign(V,0.0);return result;}
  std::vector<double> normalized=source;for(double& value:normalized)value/=scale;
  auto result=solve_poisson_strict(normalized);for(double& value:result.potential)value*=scale;result.residual*=scale;return result;
}

Arm analytic_at(const std::string& label,int orientation,
                const ftd::eft::ConnectedMooreBlockState& state,double beta,
                const ftd::eft::ConnectedMooreBlockOptions& options) {
  Arm arm;arm.label=label;arm.orientation=orientation;
  for(const auto& p:state.constituents){const Vec3 x=position(p);for(int a=0;a<3;++a){const double f=component(x,a)-std::floor(component(x,a));arm.minimum_knot_clearance=std::min(arm.minimum_knot_clearance,std::abs(f-.5));}}
  const auto d=deposit(state);arm.charge_residual=d.charge_residual;arm.derivative_charge_residual=d.derivative_charge_residual;arm.derivative_moment_residual=d.derivative_moment_residual;if(!d.valid)return arm;
  const auto phi=solve_poisson_strict(d.rho);arm.maximum_poisson_iterations=phi.iterations;arm.maximum_poisson_residual=phi.residual;if(!phi.valid)return arm;
  std::vector<std::vector<double>> response(N);
  for(int j=0;j<N;++j){auto p=solve_poisson_strict(d.first[j]);arm.maximum_poisson_iterations=std::max(arm.maximum_poisson_iterations,p.iterations);arm.maximum_poisson_residual=std::max(arm.maximum_poisson_residual,p.residual);if(!p.valid)return arm;response[j]=std::move(p.potential);}
  arm.gradient.assign(N,0);arm.hessian.assign(N,std::vector<double>(N,0));
  for(int i=0;i<N;++i){arm.gradient[i]=beta*dot(d.first[i],phi.potential);for(int j=0;j<N;++j)arm.hessian[i][j]=beta*(dot(d.first[i],response[j])+field_curvature_term(state,phi.potential,i,j));}
  add_binding(state,arm.gradient,arm.hessian);
  arm.gradient_inf=0;arm.antisymmetry=0;for(int i=0;i<N;++i){arm.gradient_inf=std::max(arm.gradient_inf,std::abs(arm.gradient[i]));for(int j=0;j<N;++j)arm.antisymmetry=std::max(arm.antisymmetry,std::abs(arm.hessian[i][j]-arm.hessian[j][i]));}
  bool energy_valid=false;const double measured=energy(state,beta,options,&energy_valid);const double analytic=ftd::eft::connected_moore_block_binding_energy(state,options)+.5*beta*dot(d.rho,phi.potential);arm.energy_identity_residual=std::abs(measured-analytic);
  arm.translation_identity=0;for(int axis=0;axis<3;++axis){std::vector<double> aggregate(V,0),v(N,0);for(int p=0;p<count;++p){v[3*p+axis]=.25;for(std::size_t k=0;k<V;++k)aggregate[k]+=d.first[3*p+axis][k];}const auto response_t=solve_poisson_strict(aggregate);if(!response_t.valid)return arm;double direct=beta*dot(aggregate,response_t.potential);for(int p=0;p<count;++p)direct+=beta*field_curvature_term(state,phi.potential,3*p+axis,3*p+axis);long double rayleigh=0;for(int i=0;i<N;++i)for(int j=0;j<N;++j)rayleigh+=static_cast<long double>(v[i])*arm.hessian[i][j]*v[j];arm.translation_identity=std::max(arm.translation_identity,std::abs(static_cast<double>(rayleigh)-direct/16));}
  const auto spectrum=diagonalize(arm.hessian);arm.eigenvalues=spectrum.values;arm.eigen_residual=spectrum.residual;arm.orthogonality=spectrum.orthogonality;if(!spectrum.values.empty()){arm.min_eigen=spectrum.values.front();arm.max_eigen=spectrum.values.back();}
  arm.stationary=arm.gradient_inf<=1e-12;arm.positive=arm.min_eigen>1e-5;
  arm.valid=energy_valid&&arm.minimum_knot_clearance>0&&arm.maximum_poisson_residual<=1e-13&&arm.energy_identity_residual<=1e-11&&arm.antisymmetry<=1e-12&&arm.translation_identity<=1e-12&&spectrum.valid;
  return arm;
}

std::vector<double> solve_linear(Matrix matrix,std::vector<double> rhs,bool& valid) {
  valid=false;for(int k=0;k<N;++k){int pivot=k;for(int i=k+1;i<N;++i)if(std::abs(matrix[i][k])>std::abs(matrix[pivot][k]))pivot=i;if(std::abs(matrix[pivot][k])<1e-12)return {};std::swap(matrix[k],matrix[pivot]);std::swap(rhs[k],rhs[pivot]);const double diagonal=matrix[k][k];for(int j=k;j<N;++j)matrix[k][j]/=diagonal;rhs[k]/=diagonal;for(int i=0;i<N;++i)if(i!=k){const double factor=matrix[i][k];if(factor==0)continue;for(int j=k;j<N;++j)matrix[i][j]-=factor*matrix[k][j];rhs[i]-=factor*rhs[k];}}
  valid=std::all_of(rhs.begin(),rhs.end(),[](double x){return std::isfinite(x);});return rhs;
}
std::vector<int> sector_signature(const ftd::eft::ConnectedMooreBlockState& state) {
  std::vector<int> result(N);for(int p=0;p<count;++p){const Vec3 x=position(state.constituents[p]);for(int a=0;a<3;++a)result[3*p+a]=static_cast<int>(std::floor(component(x,a)+.5));}return result;
}
ftd::eft::ConnectedMooreBlockState apply_delta(
    const ftd::eft::ConnectedMooreBlockState& state,
    const std::vector<double>& delta,double alpha) {
  auto result=state;for(int p=0;p<count;++p){Vec3 x=position(result.constituents[p]);for(int a=0;a<3;++a)set_component(x,a,component(x,a)+alpha*delta[3*p+a]);result.constituents[p]=point_at(x);}return result;
}
double stable_energy_change(const ftd::eft::ConnectedMooreBlockState& before,
                            const ftd::eft::ConnectedMooreBlockState& after,
                            double beta) {
  const auto d0=deposit(before),d1=deposit(after);if(!d0.valid||!d1.valid)return NAN;
  const auto phi0=solve_poisson_strict(d0.rho);std::vector<double> delta(V);for(std::size_t i=0;i<V;++i)delta[i]=d1.rho[i]-d0.rho[i];const auto dphi=solve_poisson_scaled(delta);if(!phi0.valid||!dphi.valid)return NAN;
  long double binding=0;for(const auto& edge:before.edges){const Vec3 b=position(before.constituents[edge.first])-position(before.constituents[edge.second]);const Vec3 a=position(after.constituents[edge.first])-position(after.constituents[edge.second]);const long double u0=static_cast<long double>(b.dot(b))-edge.rest_length_squared,u1=static_cast<long double>(a.dot(a))-edge.rest_length_squared;binding+=.25L*(u1-u0)*(u1+u0);}
  return static_cast<double>(binding)+beta*(dot(delta,phi0.potential)+.5*dot(delta,dphi.potential));
}
double gradient_comparison(const ftd::eft::ConnectedMooreBlockState& state,
                           const Arm& analytic,double beta,
                           const ftd::eft::ConnectedMooreBlockOptions& options) {
  double result=0;for(int i=0;i<N;++i){bool vp=false,vm=false;const double ep=energy(displace(state,i,gradient_h),beta,options,&vp),em=energy(displace(state,i,-gradient_h),beta,options,&vm);if(!vp||!vm)return INFINITY;result=std::max(result,std::abs((ep-em)/(2*gradient_h)-analytic.gradient[i]));}return result;
}
RefinedArm refine(const std::string& label,int orientation,const Theta& theta,
                  double beta,const ftd::eft::ConnectedMooreBlockOptions& options) {
  RefinedArm result;result.orientation=orientation;const auto initialized=ftd::eft::initialize_connected_moore_block(L,2,orientation,orientation,.5,1e-13,4096);if(!initialized.valid)return result;result.initial_state=geometry_from(initialized.state,theta,orientation);result.final_state=result.initial_state;const auto locked_sector=sector_signature(result.initial_state);bool evalid=false;result.initial_energy=energy(result.initial_state,beta,options,&evalid);if(!evalid)return result;
  Arm current=analytic_at(label,orientation,result.final_state,beta,options);if(!current.valid){std::cerr<<label<<" initial analytic invalid poisson="<<current.maximum_poisson_residual<<" energy="<<current.energy_identity_residual<<" antisym="<<current.antisymmetry<<" translation="<<current.translation_identity<<" eig="<<current.eigen_residual<<'\n';return result;}result.initial_gradient=current.gradient_inf;
  for(int iteration=0;iteration<4&&current.gradient_inf>1e-12;++iteration){std::vector<double> rhs=current.gradient;for(double& x:rhs)x=-x;bool solved=false;const auto delta=solve_linear(current.hessian,rhs,solved);if(!solved){std::cerr<<label<<" iteration="<<iteration<<" linear solve failed gradient="<<current.gradient_inf<<'\n';return result;}const double directional=dot(current.gradient,delta);double delta_inf=0;for(double value:delta)delta_inf=std::max(delta_inf,std::abs(value));bool accepted=false;double accepted_alpha=0,accepted_change=0;for(double alpha:{1.0,.5,.25,.125,.0625,.03125,.015625}){const auto candidate=apply_delta(result.final_state,delta,alpha);if(sector_signature(candidate)!=locked_sector)continue;bool candidate_valid=false;energy(candidate,beta,options,&candidate_valid);const double change=stable_energy_change(result.final_state,candidate,beta);if(candidate_valid&&std::isfinite(change)&&change<=1e-4*alpha*directional){result.final_state=candidate;++result.accepted_steps;result.energy_change+=change;accepted=true;accepted_alpha=alpha;accepted_change=change;break;}}if(!accepted){std::cerr<<label<<" iteration="<<iteration<<" line search failed gradient="<<current.gradient_inf<<" directional="<<directional<<" delta_inf="<<delta_inf<<'\n';result.sector_preserved=false;return result;}current=analytic_at(label,orientation,result.final_state,beta,options);std::cerr<<label<<" iteration="<<iteration<<" alpha="<<accepted_alpha<<" delta_inf="<<delta_inf<<" energy_change="<<accepted_change<<" new_gradient="<<current.gradient_inf<<'\n';if(!current.valid){std::cerr<<label<<" iteration="<<iteration<<" analytic invalid poisson="<<current.maximum_poisson_residual<<" energy="<<current.energy_identity_residual<<" antisym="<<current.antisymmetry<<" translation="<<current.translation_identity<<" eig="<<current.eigen_residual<<'\n';return result;}}
  result.sector_preserved=sector_signature(result.final_state)==locked_sector;result.analytic=current;result.final_energy=result.initial_energy+result.energy_change;result.final_gradient=current.gradient_inf;result.final_gradient_comparison=gradient_comparison(result.final_state,current,beta,options);result.min_eigen=current.min_eigen;result.max_eigen=current.max_eigen;for(int p=0;p<count;++p)result.maximum_displacement=std::max(result.maximum_displacement,(position(result.final_state.constituents[p])-position(result.initial_state.constituents[p])).mag());result.stationary=result.final_gradient<=1e-12;result.positive=result.min_eigen>1e-5;result.valid=result.accepted_steps>=1&&result.accepted_steps<=4&&result.sector_preserved&&result.energy_change<0&&result.stationary&&result.positive&&result.final_gradient_comparison<=5e-8&&current.valid;return result;
}
double cyclic_displacement_residual(const RefinedArm& x,const RefinedArm& y) {
  double result=0;const Vec3 cx=center(x.initial_state),cy=center(y.initial_state);
  for(int p=0;p<count;++p){const Vec3 px=position(x.initial_state.constituents[p])-cx;const Vec3 target{px.y,px.x,px.z};int match=-1;for(int q=0;q<count;++q){const Vec3 py=position(y.initial_state.constituents[q])-cy;if(x.initial_state.charges[p]==y.initial_state.charges[q]&&(py-target).mag()<1e-9){match=q;break;}}if(match<0)return INFINITY;const Vec3 dx=position(x.final_state.constituents[p])-position(x.initial_state.constituents[p]);const Vec3 dy=position(y.final_state.constituents[match])-position(y.initial_state.constituents[match]);result=std::max(result,(Vec3{dx.y,dx.x,dx.z}-dy).mag());}
  return result;
}
void write_refinement(const RefinedSummary& s) {
  const auto dir=std::filesystem::path(__FILE__).parent_path().parent_path()/"results/ftd_0638";std::filesystem::create_directories(dir);std::ofstream json(dir/"ftd_0638_connected_block_analytic_static_refinement_v1.json");json<<std::setprecision(17)<<"{\n  \"ftd_id\": \"FTD-0638\",\n  \"protocol_sha256\": \""<<refinement_protocol_sha256<<"\",\n  \"parent_result_sha256\": \""<<refinement_parent_result_sha256<<"\",\n  \"verdict\": \""<<s.verdict<<"\",\n  \"production_changed\": false,\n  \"energy_covariance\": "<<s.energy_covariance<<",\n  \"spectrum_covariance\": "<<s.spectrum_covariance<<",\n  \"displacement_covariance\": "<<s.displacement_covariance<<"\n}\n";
  std::ofstream arms(dir/"ftd_0638_connected_block_analytic_static_refinement_arms_v1.csv");arms<<"ftd_id,orientation,valid,stationary,positive,sector_preserved,steps,initial_energy,final_energy,energy_change,initial_gradient,final_gradient,gradient_comparison,min_eigen,max_eigen,max_displacement\n";for(const auto&a:s.arms)arms<<std::setprecision(17)<<"FTD-0638,"<<a.orientation<<','<<a.valid<<','<<a.stationary<<','<<a.positive<<','<<a.sector_preserved<<','<<a.accepted_steps<<','<<a.initial_energy<<','<<a.final_energy<<','<<a.energy_change<<','<<a.initial_gradient<<','<<a.final_gradient<<','<<a.final_gradient_comparison<<','<<a.min_eigen<<','<<a.max_eigen<<','<<a.maximum_displacement<<'\n';
  std::ofstream states(dir/"ftd_0638_connected_block_analytic_static_refinement_states_v1.csv");states<<"ftd_id,orientation,particle,charge,x0,y0,z0,x1,y1,z1\n";for(const auto&a:s.arms)for(int p=0;p<count;++p){const Vec3 x0=position(a.initial_state.constituents[p]),x1=position(a.final_state.constituents[p]);states<<std::setprecision(17)<<"FTD-0638,"<<a.orientation<<','<<p<<','<<a.initial_state.charges[p]<<','<<x0.x<<','<<x0.y<<','<<x0.z<<','<<x1.x<<','<<x1.y<<','<<x1.z<<'\n';}
  std::ofstream eigen(dir/"ftd_0638_connected_block_analytic_static_refinement_eigenvalues_v1.csv");eigen<<"ftd_id,orientation,index,eigenvalue\n";for(const auto&a:s.arms)for(std::size_t i=0;i<a.analytic.eigenvalues.size();++i)eigen<<std::setprecision(17)<<"FTD-0638,"<<a.orientation<<','<<i<<','<<a.analytic.eigenvalues[i]<<'\n';
}
}

#ifdef FTD_0638_EMBEDDED
int ftd_0638_embedded_main() {
#else
int main() {
#endif
  RefinedSummary summary;summary.parent=refinement_parent_fingerprint();const auto normalization=ftd::eft::measure_face_flux_normalization();summary.normalization=normalization.valid;summary.beta=normalization.mapped_field_work_coefficient;ftd::eft::ConnectedMooreBlockOptions options;options.allow_shared_anchor_chart=true;
  if(summary.parent&&summary.normalization){summary.arms.push_back(refine("refined_x",0,theta_x,summary.beta,options));std::cout<<"completed refined_x\n"<<std::flush;summary.arms.push_back(refine("refined_y",1,theta_y,summary.beta,options));std::cout<<"completed refined_y\n"<<std::flush;}
  if(summary.arms.size()==2){const auto&x=summary.arms[0];const auto&y=summary.arms[1];const bool spectra_present=x.analytic.eigenvalues.size()==N&&y.analytic.eigenvalues.size()==N;summary.energy_covariance=std::abs(x.final_energy-y.final_energy);summary.spectrum_covariance=0;if(spectra_present)for(int i=0;i<N;++i)summary.spectrum_covariance=std::max(summary.spectrum_covariance,std::abs(x.analytic.eigenvalues[i]-y.analytic.eigenvalues[i])/std::max({1.0,std::abs(x.analytic.eigenvalues[i]),std::abs(y.analytic.eigenvalues[i])}));else summary.spectrum_covariance=INFINITY;summary.displacement_covariance=cyclic_displacement_residual(x,y);summary.covariance=summary.energy_covariance<=1e-9&&summary.spectrum_covariance<=1e-9&&summary.displacement_covariance<=1e-9;const bool analytic_valid=spectra_present&&std::all_of(summary.arms.begin(),summary.arms.end(),[](const RefinedArm&a){return a.analytic.valid;});if(analytic_valid&&summary.covariance){if(std::all_of(summary.arms.begin(),summary.arms.end(),[](const RefinedArm&a){return a.valid;}))summary.verdict="CONNECTED_BLOCK_ANALYTIC_STATIC_BASIN_CONSTRUCTIVE";else if(std::any_of(summary.arms.begin(),summary.arms.end(),[](const RefinedArm&a){return !a.sector_preserved;}))summary.verdict="CONNECTED_BLOCK_ANALYTIC_REFINEMENT_LEFT_SECTOR";else summary.verdict="CONNECTED_BLOCK_ANALYTIC_REFINEMENT_NONSTATIONARY";}}
  write_refinement(summary);std::cout<<std::setprecision(17)<<"protocol_sha256="<<refinement_protocol_sha256<<'\n'<<"verdict="<<summary.verdict<<'\n'<<"covariance=("<<summary.energy_covariance<<','<<summary.spectrum_covariance<<','<<summary.displacement_covariance<<")\n";for(const auto&a:summary.arms)std::cout<<"orientation="<<a.orientation<<" valid="<<a.valid<<" steps="<<a.accepted_steps<<" energy=("<<a.initial_energy<<','<<a.final_energy<<") gradient=("<<a.initial_gradient<<','<<a.final_gradient<<") spectrum=("<<a.min_eigen<<','<<a.max_eigen<<") displacement="<<a.maximum_displacement<<'\n';return summary.verdict=="CONNECTED_BLOCK_ANALYTIC_REFINEMENT_EXECUTION_INVALID"?1:0;
}
