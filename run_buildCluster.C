{
  gSystem->Load("libAnalysis.so");
  gROOT->LoadMacro("buildCluster.C");
  buildCluster();
}
