{
  gSystem->Load("libAnalysis.so");
  gROOT->LoadMacro("trackFit.C");
  trackFit();
}
