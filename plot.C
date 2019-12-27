#include "Analysis.h"

void plot(TString fname = "tracks_CRUN_3_1.root")
{
  TFile* fin = new TFile(fname.Data());
  TTree* data = static_cast<TTree*>(fin->Get("EventData"));

  Event* evt = 0;
  data->SetBranchAddress("Event",&evt);

  TString outfilename = "hist";
  outfilename += GetOutputSuffix(fname);
  TFile* fout = new TFile(outfilename.Data(),"RECREATE");

  TH2F* hxdist = new TH2F("hxdist","",100,-0.05,0.05,100,0.,1.1);
  TH2F* hydist = new TH2F("hydist","",100,-0.05,0.05,100,0.,1.1);
  TH1F* hnpxls = new TH1F("hnpxl","",100,0.,100.);
  TH1F* hadc_pxl = new TH1F("hadc_pxl","",100,0.,100);
  TH1F* hadc_clr = new TH1F("hadc_clr","",100,0.,1000);

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

    data->GetEntry(ev);
    int ntracks = evt->GetEntries();
    for (int itrk =0; itrk < ntracks; itrk++) {
      Track* trk = static_cast<Track*>(evt->At(itrk));

      // 1) Draw hit points
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

      int nclusters = trk->GetEntries();
      for (int iclr = 0; iclr < nclusters; iclr++) {
        Cluster* clr = static_cast<Cluster*>(trk->At(iclr));
	hadc_clr->Fill(clr->val);
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

	int npixels = clr->GetEntries();
	hnpxls->Fill(npixels);
        for (int ipxl = 0; ipxl < npixels; ipxl++) {
          Pixel* pxl = static_cast<Pixel*>(clr->At(ipxl));
	  hxdist->Fill(pxl->pos.X()-pos.X(),float(pxl->val)/float(clr->val));
	  hydist->Fill(pxl->pos.Y()-pos.Y(),float(pxl->val)/float(clr->val));
	  hadc_pxl->Fill(pxl->val);
	}
      }

      g_zx.GetHistogram()->SetMaximum(30.);
      g_zx.GetHistogram()->SetMinimum(-3.);
      g_zx.GetHistogram()->GetXaxis()->SetLimits(0.,6.144);
      g_zx.GetHistogram()->SetStats(0);
      c->cd(1);
      g_zx.Draw("ap");
      g_zy.GetHistogram()->SetMaximum(30.);
      g_zy.GetHistogram()->SetMinimum(-3.);
      g_zy.GetHistogram()->GetXaxis()->SetLimits(0.,6.144);
      g_zy.GetHistogram()->SetStats(0);
      c->cd(2);
      g_zy.Draw("ap");

      // 2) Draw track
      TF1 fitzx("fitzx","[0]*x+[1]",0.,6.144);
      TF1 fitzy("fitzy","[0]*x+[1]",0.,6.144);
      fitzx.SetNpx(1000);
      fitzx.SetParameter(0,trk->ax);
      fitzx.SetParameter(1,trk->bx);
      fitzy.SetNpx(1000);
      fitzy.SetParameter(0,trk->ay);
      fitzy.SetParameter(1,trk->by);
      c->cd(1);
      fitzx.Draw("same");
      c->cd(2);
      fitzy.Draw("same");

      TString outpdf = "evt_used";
      outpdf += GetOutputSuffix(fname,"pdf");
      if (ev==firstEvent && itrk==firstTrack) {
	outpdf += "(";
      } else if (ev==lastEvent && itrk==lastTrack) {
	outpdf += ")";
      } 
      c->Print(outpdf.Data());
    }
  }
  c->Close();
  fout->Write();
}
