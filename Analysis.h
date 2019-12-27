#ifndef __ANALYSIS__
#define __ANALYSIS__

#include "TObjArray.h"
#include "TObjString.h"
#include "TVector3.h"
#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"

// Utility function
static TString GetOutputSuffix(TString fname, TString filetype = "root")
{
  TObjArray* t = fname.Tokenize("CRUN");  
  TString suffix = ((TObjString*)(t->At(1)))->String();
  t = suffix.Tokenize(".");  
  suffix = "_CRUN";
  suffix += ((TObjString*)(t->At(0)))->String();
  suffix += ".";
  suffix += filetype;
  return suffix; 
}


class Event : public TObjArray
{
  ClassDef(Event,1)
};

class Pixel : public TObject 
{
  public:

    int Compare(const TObject* obj) const { return static_cast<const Pixel*>(obj)->val - this->val; } // Used in Sort(),  descending order

    bool IsSortable() const { return true; }

    int      val; // adc
    int      row; // X
    int      col; // Y
    int      ch;  // ch 
    int      ccd; // ccd id
    TVector3 pos;

  ClassDef(Pixel,1)
};

class Cluster : public TObjArray 
{
  public :

    Cluster() : TObjArray(), isShared(false) {}

    int Compare(const TObject* obj) const { return this->pos.Z() - static_cast<const Cluster*>(obj)->pos.Z(); } // Used in Sort(), ascending order

    bool IsSortable() const { return true; }

    TVector3 pos;
    TVector3 sig;
    int      val;      // adc sum
    int      ccd;      // ccd id
    bool     isShared; // true if this cluster is shared with another track candidates.

  ClassDef(Cluster,1)
};

class Track : public TObjArray 
{
  public:

    int Compare(const TObject* obj) const { return this->chi2 - static_cast<const Track*>(obj)->chi2; } // ascending order

    bool IsSortable() const { return true; }

   // track parameters
   double ax;
   double bx;
   double ay;
   double by;

   // fit results
   double chi2;

  ClassDef(Track,1)
};

class TrackFitter
{
  public:

    TrackFitter();

    void SetHitPoint(double x, double y, double z, double dx = 0., double dy = 0., double dz = 0.); 

    void ClearHitPoints() { _fd.clear(); }

    void PrintFitResults() { _mzr->PrintResults(); }

    static double Chi2(const double *par); 

    struct FitData {
      TVector3 pos; 
      TVector3 poserr; 
    };

    struct FitResults {
      double ax;
      double bx;
      double ay;
      double by;
      double chi2;
    };

    FitResults Fit(); 

  private :
    static std::vector<FitData> _fd;
    ROOT::Math::Minimizer*      _mzr;
    ROOT::Math::Functor*        _funcObj;
};

#endif 
