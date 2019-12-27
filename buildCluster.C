#include "Analysis.h"

void buildCluster(TString fname = "pixel_CRUN_3_1.root")
{
  TFile* fin = new TFile(fname.Data());
  TTree* data = static_cast<TTree*>(fin->Get("EventData"));

  Event* evt = 0;
  data->SetBranchAddress("Event",&evt);

  TString outfilename = "cluster";
  outfilename += GetOutputSuffix(fname);
  TFile* fout = new TFile(outfilename.Data(),"RECREATE");
  TTree* outdata = new TTree("EventData","");
  Event* outevt = new Event;
  outdata->Branch("Event", &outevt);

  int nevents = data->GetEntries();
  for (int ev = 0; ev < nevents; ev++) {
    cout << endl;
    cout << "### ev = " << ev << endl;

    outevt->Clear(); // Clear previous event data
    data->GetEntry(ev);

    set<int> ccdIDs;
    vector<Cluster*> clrs;

    int npixels = evt->GetEntries();
    for (int ipxl = 0; ipxl < npixels; ipxl++) {
      Pixel* pxl = static_cast<Pixel*>(evt->At(ipxl));
      //cout << "ipxl = " <<  ipxl << endl;
      auto retval = ccdIDs.insert(pxl->ccd);
      if (retval.second) { // found new ccdID
      //cout << "New ccdID " <<  pxl->ccd << endl;
 	Cluster* newclr = new Cluster;
	newclr->ccd = pxl->ccd;
	newclr->Add(pxl);
	clrs.push_back(newclr);
      } else {             // this pixel must be added to existing one of clusters
      //cout << "ccdID exist :" <<  pxl->ccd << endl;
	for (int ic = 0; ic < clrs.size(); ic++) {
          Cluster* clr = clrs[ic];
      //cout << "  ic :" <<ic << " " << clr->ccd << endl;
	  if (clr->ccd == pxl->ccd) {
	    clr->Add(pxl); 
	    break;
	  } 
	}
      }
    }

    cout << "ev = " << ev << " nclusters = " << clrs.size() << endl;
    for (int ic = 0; ic < clrs.size(); ic++) outevt->Add(clrs[ic]);
    outdata->Fill();
  }

  fin->Close();
  fout->Write();
}
