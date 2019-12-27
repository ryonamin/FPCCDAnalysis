#include "Analysis.h"

const float pixel_pitch = 0.006; // mm
const float pixel_pitch_sig2 = pixel_pitch*pixel_pitch/12.; // mm^2

void buildMultiClusters(TString fname = "cluster_CRUN_3_1.root")
{
  TFile* fin = new TFile(fname.Data());
  TTree* data = static_cast<TTree*>(fin->Get("EventData"));

  Event* evt = 0;
  data->SetBranchAddress("Event",&evt);

  TString outfilename = "multiclusters";
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

    for (int ic = 0; ic < nclusters; ic++) {
      Cluster* clr = static_cast<Cluster*>(evt->At(ic)); 
      //cout << clr->ccd <<endl;
      int npixels = clr->GetEntries();
      clr->Sort(); // pixel val descending order
      //cout << "npixels = " << npixels << endl;

      // Find multiple peaks in this cluster.
      vector<Cluster*> newclrs; // Store new split clusters 
      for (int ip = 0; ip < npixels; ip++) {
        Pixel* pxl = static_cast<Pixel*>(clr->At(ip));
	if (pxl->val==0) break; // neglect adc == 0

	//cout << "pxl " << ip << " : " << pxl->val << " row : " << pxl->row << " col : " << pxl->col << endl;
	if (ip==0) {
	  Cluster* newclr = new Cluster;
	  newclr->Add(pxl);
	  newclrs.push_back(newclr);
	  continue;
	}

	bool noFoundNeighbors = true;
	for (int nic = 0; nic < newclrs.size(); nic++) {
          Cluster* newclr = newclrs[nic];
	  for (int nip = 0; nip < newclr->GetEntries(); nip++) {
            //cout << "nip = " << nip << endl;
            Pixel* p = static_cast<Pixel*>(newclr->At(nip));
            //cout << "abs(p->row - pxl->row) = " << abs(p->row - pxl->row)  << endl;
            //cout << "abs(p->col - pxl->col) = " << abs(p->col - pxl->col)  << endl;
            if ( abs(p->row - pxl->row) <= 1 && abs(p->col - pxl->col) <= 1 ) { // searching neighbors
              //cout << "p->val = " << p->val << " pxl->val = " << pxl->val << endl;
              if (p->val >= pxl->val) { // new neighbors should be lower adc than previous pixels. Otherwise it should be a part of another cluster.
	        newclr->Add(pxl);
		//cout << "new element to be clustered." << endl;
                noFoundNeighbors = false; 
		//cout << "Found neighbers" << endl;
	        break;
	      } 
	    } 
          }
          if (!noFoundNeighbors) break; 
        }
        if (noFoundNeighbors) {
          //cout << "Second cluster found !!!" << endl;
	  //abort();
	  Cluster* newclr = new Cluster;
	  newclr->Add(pxl);
	  newclrs.push_back(newclr);
	}	
      }

      //cout << "New cluster size = " << newclrs.size() << endl;
      for (int ic = 0; ic < newclrs.size(); ic++) {
        Cluster* clr = newclrs[ic];
	int adcsum = 0;
	double x = 0.;
	double y = 0.;
	double z = 0.;
	double x2 = 0.;
	double y2 = 0.;
        for (int ip = 0; ip < clr->GetEntries(); ip++) {
          Pixel* pxl = static_cast<Pixel*>(clr->At(ip));
	  adcsum += pxl->val;
	  TVector3 pos = pxl->pos;
	  x += pos.X()*pxl->val;
	  y += pos.Y()*pxl->val;
	  z += pos.Z()*pxl->val;
	  x2 += pos.X()*pos.X()*pxl->val;
	  y2 += pos.Y()*pos.Y()*pxl->val;
	}
	x /= adcsum;
	y /= adcsum;
	z /= adcsum;
	x2 /= adcsum;
	y2 /= adcsum;
	double dx = TMath::Sqrt(pixel_pitch_sig2 + x2-x*x);
	double dy = TMath::Sqrt(pixel_pitch_sig2 + y2-y*y);
        clr->pos = TVector3(x,y,z);
        clr->sig = TVector3(dx,dy,0.1); // FIXME 0.1 is just a tentative value.
	clr->val = adcsum;
	clr->ccd = static_cast<Pixel*>(clr->First())->ccd;

	cout << "ic = " << ic << " adc = " << clr->val << " npixels = " << clr->GetEntries() << " pos(" << x << ", " << y << ", " << z << ")" 
	     << " sig(" << dx << ", " << dy << ")" << endl;
	//cout << "clr->sig.X() " << clr->sig.X() << " clr->ccd = " << clr->ccd << endl;
	//cout << "x2-x*x = " << x2-x*x << " x2 = " << x2 << " x*x = " << x*x << endl;
	outevt->Add(clr);
      }

    }
    outdata->Fill();
  }
  fin->Close();
  fout->Write();
}
