#include "Analysis.h"

ClassImp(Event)
ClassImp(Pixel)
ClassImp(Cluster)
ClassImp(Track)

// static member variable must be initialized in global scope.
std::vector<TrackFitter::FitData> TrackFitter::_fd;

TrackFitter::TrackFitter()
{
   _mzr =  ROOT::Math::Factory::CreateMinimizer("Minuit2", "Migrad");
   _funcObj = new ROOT::Math::Functor(&TrackFitter::Chi2,4); // 4: ax, bx, ay, by
   _mzr->SetFunction(*_funcObj);
   _mzr->SetVariable(0,"axz",0.,0.01);
   _mzr->SetVariable(1,"bxz",3.,0.01);
   _mzr->SetVariable(2,"ayz",0.,0.01);
   _mzr->SetVariable(3,"byz",3.,0.01);
   _mzr->SetVariableLowerLimit(0,-1.);
   _mzr->SetVariableLowerLimit(2,-1.);
   _mzr->SetVariableUpperLimit(0,1.);
   _mzr->SetVariableUpperLimit(2,1.);
   _mzr->SetVariableLowerLimit(1,-100.);
   _mzr->SetVariableLowerLimit(3,-100.);
   _mzr->SetVariableUpperLimit(1,100.);
   _mzr->SetVariableUpperLimit(3,100.);
   _mzr->SetPrintLevel(1);
   _mzr->SetTolerance(0.001);
   _mzr->SetMaxIterations(100000);
}

void TrackFitter::SetHitPoint(double x, double y, double z, double dx, double dy, double dz) 
{
   FitData newhit;
   newhit.pos = TVector3(x,y,z);
   newhit.poserr = TVector3(dx,dy,dz);
   _fd.push_back(newhit); 
}


TrackFitter::FitResults TrackFitter::Fit() 
{
  _mzr->Minimize();

  const double* params = _mzr->X();
  double minchi2 = _mzr->MinValue();

  FitResults r;
  double axz = params[0];
  double bxz = params[1];
  double ayz = params[2];
  double byz = params[3];
  r.ax = 1./axz;
  r.bx = -bxz/axz;
  r.ay = 1./ayz;
  r.by = -byz/ayz;
  r.chi2 = minchi2;

  return r;
}

double TrackFitter::Chi2(const double *par) 
{
  double axz = par[0]; 
  double bxz = par[1]; // x = [axz] * z + [bxz]

  double ayz = par[2]; 
  double byz = par[3]; // y = [ayz] * z + [byz]
  double ret_chi2 = 0.;
  for (int ihit = 0; ihit < _fd.size(); ihit++) {
     TVector3 pos = _fd[ihit].pos;
     TVector3 err = _fd[ihit].poserr;
     double x = axz*pos.Z() + bxz;
     double y = ayz*pos.Z() + byz;
     double dx = axz*err.Z();
     double dy = ayz*err.Z();
     //ret_chi2 += (pos.X()-x)*(pos.X()-x)/(err.X()*err.X()+dx*dx); // This does't work, because dx,dy are sensitive to axz, ayz.
     //ret_chi2 += (pos.Y()-y)*(pos.Y()-y)/(err.Y()*err.Y()+dy*dy);
     ret_chi2 += (pos.X()-x)*(pos.X()-x)/(err.X()*err.X());
     ret_chi2 += (pos.Y()-y)*(pos.Y()-y)/(err.Y()*err.Y());
     //std::cout << "pos.X()-x = " << pos.X()-x << " err.X() = " << err.X() << " dx = " << dx << std::endl;
  }
  return ret_chi2;
}
