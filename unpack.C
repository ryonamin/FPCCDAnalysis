#include "Analysis.h"

TVector3 GetPosition(int ch, int X, int Y)
{
   const float pixel_pitch_x = 0.006; // mm
   const float pixel_pitch_y = 0.006; // mm

   float x = pixel_pitch_x * (X+0.5);

   float y = pixel_pitch_y * (Y+0.5);
   if      (ch%4==3) y += (256*0+4) * pixel_pitch_y; //mm, 4*pixel_pitch_y: transfer band
   else if (ch%4==2) y += (256*1+3) * pixel_pitch_y; //mm, 256:offset of ch%4==3, 3*pixel_pitch_y: transfer band
   else if (ch%4==1) y += (256*2+2) * pixel_pitch_y; //mm, 256*2:offset of ch%4==3,2, 2*pixel_pitch_y: transfer band
   else              y += (256*3+1) * pixel_pitch_y; //mm, 256*3:offset of ch%4==3,2,1, 2*pixel_pitch_y: transfer band

   float z = 0.; // default 12 <= ch <= 15
   if (ch<4) {
     z = 27.9; // mm
   } else if (ch<8) {
     z = 15.7; // mm
   } else if (ch<12) {
     z = 3.5; // mm
   } 

   return TVector3(x,y,z); 
}

void unpack(TString fname = "output_CRUN_3_1.dat")
{

  TString outfilename = "pixel";
  outfilename += GetOutputSuffix(fname);
  TFile* fout = new TFile(outfilename.Data(),"RECREATE");
  TTree* data = new TTree("EventData","");

  Event* evt = 0;
  data->Branch("Event","Event",&evt);

  ifstream fin(fname.Data());
  int val[8];
  bool isStartReading = false;
  while (fin >> val[0] >> val[1] >> val[2] >> val[3] >> val[4] >> val[5] >> val[6] >> val[7] ) {

    if (val[0]==0) {                                // one of meta lines
      if (val[1]==1) {                              // # first one 
        if (val[3] || val[4] || val[5] || val[6]) { //   # found hits
	   isStartReading = true;
	   cout << "New event" << endl;
	   evt->Clear();                             //   # Clear previous data
	   continue;                                //   # Go to next line
	}
      }    
      if (val[1]==2) {                              // # second one
	if (isStartReading) {                       //   # stop reading if already started
          cout << "Event data filled" << endl;
	  data->Fill();                             //   # save event data
	  isStartReading = false;
	}
      }    
    }

    if (isStartReading) { 
      int ch  = val[1];
      int X   = val[5];
      int Y   = val[6];
      int adc = val[7];
      TVector3 pos = GetPosition(ch,X,Y);
      cout << "ch = " << ch << ", X = " << X << ", Y = " << Y << ", adc = " << adc << ", pos = (" << pos.X() << ", " << pos.Y() << ", " << pos.Z() << ")"<< endl;
      Pixel* pxl = new Pixel();
      pxl->val = adc; 
      pxl->ch  = ch; 
      pxl->row = X; 
      pxl->col = Y; 
      int ccd = 1;
      if      (ch<4)  ccd = 4; 
      else if (ch<8)  ccd = 3; 
      else if (ch<12) ccd = 2; 
      pxl->ccd = ccd; 
      pxl->pos = pos;

      evt->Add(pxl);

    } 

  }

  fout->Write();

}
