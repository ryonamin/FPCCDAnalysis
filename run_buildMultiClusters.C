{
  gSystem->Load("libAnalysis.so");
  gROOT->LoadMacro("buildMultiClusters.C");
  buildMultiClusters();
}
