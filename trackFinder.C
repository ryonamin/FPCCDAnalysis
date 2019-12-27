#include "Analysis.h"

void trackFinder(TString fname = "multiclusters_CRUN_3_1.root")
{
  TFile* fin = new TFile(fname.Data());
  TTree* data = static_cast<TTree*>(fin->Get("EventData"));

  Event* evt = 0;
  data->SetBranchAddress("Event",&evt);

  TString outfilename = "trackhits";
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

    int nclusters = evt->GetEntries();
    //cout << "nclusters = " << nclusters << endl;
    vector<Cluster*> trkcandidates;
    evt->Sort(); // sorting by z position, ascending order.

    // cluster requirements
    for (int ic = 0; ic < nclusters; ic++) {
      Cluster* clr = static_cast<Cluster*>(evt->At(ic));
      //if (clr->GetEntries()<2 && clr->val<10) continue;
      if (clr->GetEntries()<2 && clr->val<30) continue;
      if (clr->val<10) continue;
      trkcandidates.push_back(clr);
    }

    // track must have continuous hits in z 
    if (trkcandidates.size()>1) {
	    for (int i =0; i < trkcandidates.size(); i++) {
	      Cluster* clr = trkcandidates[i];
	    }
      vector<Track*> trkcandidates2;
      Track* trk1 = new Track;
      trk1->Add(trkcandidates[0]);
      trkcandidates2.push_back(trk1);
      for (int i = 1; i < trkcandidates.size(); i++) {
        bool isNewTrackCandidate = true;
	bool isAssociated = false;
        for (int j = 0; j < trkcandidates2.size(); j++) {
          Track* trk = trkcandidates2[j];
          Cluster* clr = static_cast<Cluster*>(trk->Last());
	  //cout << "trkcandidates[" << i << "]->ccd = " << trkcandidates[i]->ccd << " clr->ccd = " << clr->ccd << endl;
          if (trkcandidates[i]->ccd==clr->ccd+1) {
            trk->Add(trkcandidates[i]);
            isNewTrackCandidate = false;
	    if (isAssociated) { // This cluster has been already associated to another track candidates.
              trkcandidates[i]->isShared = true;
	      //cout << "trkcandidates[i]->isShared get true" << endl;
	    } else {
	      //cout << "trkcandidates[i] first association" << endl;
              isAssociated = true;
	    }
          } else if (trkcandidates[i]->ccd==clr->ccd) { // multiple cluster in a ccd found
            Track* newtrk = 0;
            if (trk->GetEntries()==1) { // No need to copy 
	      newtrk = new Track;       // new clusters
	    } else {
	      newtrk = new Track(*trk); // copy the candidate clusters already found and shoud remove the last one.
	      newtrk->RemoveAt(newtrk->GetLast());
	      for (int ictrk = 0; ictrk < newtrk->GetEntries(); ictrk++) {
		 static_cast<Cluster*>(newtrk->At(ictrk))->isShared = true;
	      }
	      //cout << "trk = " << trk << " newtrk = " << newtrk << endl;
	    }

	    if (isAssociated) { // This cluster has been already associated to another track candidates.
              trkcandidates[i]->isShared = true;
	      //cout << "trkcandidates[i]->isShared get true" << endl;
	    } else {
	      //cout << "trkcandidates[i] first association" << endl;
              isAssociated = true;
	    }

	    newtrk->Add(trkcandidates[i]);
            trkcandidates2.push_back(newtrk);
            isNewTrackCandidate = false;
	    j++; // This is necessary because now trkcandidates2.size() has been incremented and loop again with same cluster that is already in.
	  }
        }	
        if (isNewTrackCandidate) { 
          // Distant (in z direction) cluster found. Create new track candidates.
          Track* newtrk = new Track;
          newtrk->Add(trkcandidates[i]);
          trkcandidates2.push_back(newtrk);
        } 
      } 
      //cout << "trkcandidates2.size() = " << trkcandidates2.size() << endl;

      for (int i = 0; i < trkcandidates2.size(); i++) {
        Track* trk = trkcandidates2[i];
        int nclusters = trk->GetEntries();
	cout << "  nclusters associated to this track = " << nclusters << endl;
	if (nclusters>1) { 
          cout << "  " << nclusters << " clusters found. Saved this track." << endl;
          outevt->Add(trk);
	} else {
          cout << "  Only " << nclusters << " clusters found. Neglect this cluster." << endl;
	}
      }
    } else {
      cout << "  No track cluster candidate found. Skip." << endl;
    }
    outdata->Fill();
  }
  fout->Write();

}
