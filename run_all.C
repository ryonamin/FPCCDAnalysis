{

  TString fname_id = "CRUN_3_1";


  TString fname_unpack = "output_";
  fname_unpack += fname_id;
  fname_unpack += ".dat";

  TString fname_cluster = "pixel_";
  fname_cluster += fname_id;
  fname_cluster += ".root";

  TString fname_multiclusters = "cluster_";
  fname_multiclusters += fname_id;
  fname_multiclusters += ".root";

  TString fname_trackhits = "multiclusters_";
  fname_trackhits += fname_id;
  fname_trackhits += ".root";

  TString fname_tracks = "trackhits_";
  fname_tracks += fname_id;
  fname_tracks += ".root";

  TString fname_hist = "tracks_";
  fname_hist += fname_id;
  fname_hist += ".root";

  gSystem->Load("libAnalysis.so");
  gROOT->LoadMacro("unpack.C");
  gROOT->LoadMacro("buildCluster.C");
  gROOT->LoadMacro("buildMultiClusters.C");
  gROOT->LoadMacro("trackFinder.C");
  gROOT->LoadMacro("trackFit.C");
  gROOT->LoadMacro("plot.C");
  unpack(fname_unpack);
  buildCluster(fname_cluster);
  buildMultiClusters(fname_multiclusters);
  trackFinder(fname_trackhits);
  trackFit(fname_tracks);
  plot(fname_hist);
}
