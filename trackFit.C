#include "Analysis.h"

bool isGoodTrack(Track* trk) {
  int nclusters = trk->GetEntries();
  if (nclusters>1) return true;
  return false;
}

void trackFit(TString fname = "trackhits_CRUN_3_1.root")
{
  TFile* fin = new TFile(fname.Data());
  TTree* data = static_cast<TTree*>(fin->Get("EventData"));

  Event* evt = 0;
  data->SetBranchAddress("Event",&evt);

  TString outfilename = "tracks";
  outfilename += GetOutputSuffix(fname);
  TFile* fout = new TFile(outfilename.Data(),"RECREATE");
  TTree* outdata = new TTree("EventData","");
  Event* outevt = new Event;
  outdata->Branch("Event", &outevt);

  TCanvas* c = new TCanvas("c","",400,800);
  c->Divide(2,1);

  int nevents = data->GetEntries();

  // Find first track and last track in all the events for opening pdf and closing pdf.
  int firstEvent = -1;
  int firstTrack = -1;
  int lastEvent = -1;
  int lastTrack = -1;
  for (int ev = 0; ev < nevents; ev++) {
    data->GetEntry(ev);
    int ntracks = evt->GetEntries();
    for (int itrk = 0; itrk < ntracks; itrk++) {
      Track* trk = static_cast<Track*>(evt->At(itrk)); 
      if (firstEvent<0) firstEvent = ev;
      else              lastEvent = ev;
      if (firstTrack<0) firstTrack = itrk;
      else              lastTrack = itrk;
    }
  }

  for (int ev = 0; ev < nevents; ev++) {
    cout << endl;
    cout << "### ev = " << ev << endl;
    outevt->Clear(); // Clear previous event data
    data->GetEntry(ev);

    int ntracks = evt->GetEntries();

    if (ntracks==0) cout << "  No tracks found." << endl;

    TrackFitter fitter;

    TObjArray candidates;
    bool isTrackSharedClusters = false;
    for (int itrk = 0; itrk < ntracks; itrk++) {

      // 1) Draw all hit points
      stringstream zx_title,zy_title;
      zx_title << "ev" << ev << ";X [mm];Z [mm]" << ends;
      zy_title << ";Y [mm];Z [mm]" << ends;
      TGraphErrors g_zx;
      g_zx.SetTitle(zx_title.str().data());
      g_zx.SetMarkerStyle(24);
      g_zx.SetMarkerColor(kBlack);
      TGraphErrors g_zy;
      g_zy.SetTitle(zy_title.str().data());
      g_zy.SetMarkerStyle(24);
      g_zy.SetMarkerColor(kBlack);
      // auxiliary TGraphErrors to show shared clusters in red
      TGraphErrors g_zx_shared;
      g_zx_shared.SetMarkerStyle(24);
      g_zx_shared.SetMarkerColor(kRed);
      TGraphErrors g_zy_shared;
      g_zy_shared.SetMarkerStyle(24);
      g_zy_shared.SetMarkerColor(kRed);

      // Track fitting
      fitter.ClearHitPoints();

      Track* trk = static_cast<Track*>(evt->At(itrk));
      int nclusters = trk->GetEntries();
      int nshared = 0;
      for (int iclr = 0; iclr < nclusters; iclr++) {
        Cluster* clr = static_cast<Cluster*>(trk->At(iclr));
	TVector3 pos = clr->pos;
	TVector3 sig = clr->sig;
	float sqrtN = TMath::Sqrt(clr->GetEntries());
	TVector3 err = 1./sqrtN*sig;
        // XZ plane
        g_zx.SetPoint(iclr,pos.X(),pos.Z());
        g_zx.SetPointError(iclr,err.X(),err.Z());
	g_zx.SetMarkerSize(clr->val/200.);
        // YZ plane
        g_zy.SetPoint(iclr,pos.Y(),pos.Z());
        g_zy.SetPointError(iclr,err.Y(),err.Z());
	g_zy.SetMarkerSize(clr->val/200.);

	if (clr->isShared) {
          isTrackSharedClusters = true; // one of candidates will be chosen by chi2
          g_zx_shared.SetPoint(nshared,pos.X(),pos.Z());
          g_zx_shared.SetPointError(nshared,err.X(),err.Z());
          g_zx_shared.SetMarkerSize(clr->val/300.);
          g_zy_shared.SetPoint(nshared,pos.Y(),pos.Z());
          g_zy_shared.SetPointError(nshared,err.Y(),err.Z());
          g_zy_shared.SetMarkerSize(clr->val/300.);
	  nshared++;
	} 

        fitter.SetHitPoint(pos.X(),pos.Y(),pos.Z(),err.X(),err.Y(),err.Z());
      }

      // Draw
      g_zx.GetHistogram()->SetMaximum(30.);
      g_zx.GetHistogram()->SetMinimum(-3.);
      g_zx.GetHistogram()->GetXaxis()->SetLimits(0.,6.144);
      g_zx.GetHistogram()->SetStats(0);
      c->cd(1);
      //gPad->SetLeftMargin(0.2);
      g_zx.Draw("ap");
      if (nshared>0) g_zx_shared.Draw("p same");
      g_zy.GetHistogram()->SetMaximum(30.);
      g_zy.GetHistogram()->SetMinimum(-3.);
      g_zy.GetHistogram()->GetXaxis()->SetLimits(0.,6.144);
      g_zy.GetHistogram()->SetStats(0);
      c->cd(2);
      //gPad->SetLeftMargin(0.2);
      g_zy.Draw("ap");
      if (nshared>0) g_zy_shared.Draw("p same");

      if (isGoodTrack(trk)) { // 2 hits are necessary at least.
        TrackFitter::FitResults results = fitter.Fit();
	fitter.PrintFitResults();
        cout << "  chi2 = " << results.chi2 << ", ax = " << results.ax << ", bx = " << results.bx << ", ay = " << results.ay << ", by = " << results.by << endl;
        TF1 fitzx("fitzx","[0]*x+[1]",0.,6.144);
        TF1 fitzy("fitzy","[0]*x+[1]",0.,6.144);
	fitzx.SetNpx(1000);
	fitzx.SetParameter(0,results.ax);
	fitzx.SetParameter(1,results.bx);
	fitzy.SetNpx(1000);
	fitzy.SetParameter(0,results.ay);
	fitzy.SetParameter(1,results.by);
	c->cd(1);
	fitzx.DrawCopy("same");
	c->cd(2);
	fitzy.DrawCopy("same");

	trk->ax = results.ax; 
	trk->bx = results.bx; 
	trk->ay = results.ay; 
	trk->by = results.by; 
	trk->chi2 = results.chi2; 
	candidates.Add(trk);
      }

      TString outpdf = "evt";
      outpdf += GetOutputSuffix(fname,"pdf");
      if (ev==firstEvent && itrk==firstTrack) {
	outpdf += "(";
      } else if (ev==lastEvent && itrk==lastTrack) {
	outpdf += ")";
      } 
      c->Print(outpdf.Data());
    }

    candidates.Sort();
    if (isTrackSharedClusters) {
       outevt->Add(candidates.First());
    } else {
      for (int itrk =0; itrk < candidates.GetEntries();itrk++){
        //cout << "chi2 = " << static_cast<Track*>(candidates.At(itrk))->chi2 << endl;
        outevt->Add(candidates.At(itrk));
      } 
    }
    outdata->Fill();
  }
  fout->Write();
  c->Close();
}
