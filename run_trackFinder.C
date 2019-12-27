{
  gSystem->Load("libAnalysis.so");
  gROOT->LoadMacro("trackFinder.C");
  trackFinder();
}
