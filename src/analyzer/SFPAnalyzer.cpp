/*SFPAnalyzer.cpp
 *Class designed to analyze coincidence events. Currently only implemented for focal plane
 *data. Additional changes for SABRE would include this file and the sructure ProcessedEvent
 *in DataStructs.h. Based on code written by S. Balak, K. Macon, and E. Good.
 *
 *Gordon M. Oct. 2019
 *
 *Refurbished and updated Jan 2020 by GWM. Now uses both focal plane and SABRE data
 */
#include "SFPAnalyzer.h"
#include <TMath.h>

using namespace std;

/*Constructor takes in kinematic parameters for generating focal plane weights*/
SFPAnalyzer::SFPAnalyzer(int zt, int at, int zp, int ap, int ze, int ae, double ep,
                         double angle, double b) {
  Zt = zt; At = at; Zp = zp; Ap = ap; Ze = ze; Ae = ae; Ep = ep; 
  Angle = angle; B = b;
  event_address = new CoincEvent();
  rootObj = new THashTable();
  rootObj->SetOwner(false);//Stops THashTable from owning its members; prevents double delete
}

SFPAnalyzer::~SFPAnalyzer() {
  delete event_address;
}

void SFPAnalyzer::Reset() {
  pevent = blank; //set output back to blank
}

/*Use functions from FP_kinematics to calculate weights for xavg*/
void SFPAnalyzer::GetWeights() {
  w1 = (Wire_Dist()/2.0-Delta_Z(Zt, At, Zp, Ap, Ze, Ae, Ep, Angle, B))/Wire_Dist();
  w2 = 1.0-w1;
  cout<<"w1: "<<w1<<" w2: "<<w2<<endl;
}

/*2D histogram fill wrapper for use with THashTable (faster)*/
void SFPAnalyzer::MyFill(string name, int binsx, double minx, double maxx, double valuex,
                                      int binsy, double miny, double maxy, double valuey) {
  TH2F *histo = (TH2F*) rootObj->FindObject(name.c_str());
  if(histo != NULL) {
    histo->Fill(valuex, valuey);
  } else {
    TH2F *h = new TH2F(name.c_str(), name.c_str(), binsx, minx, maxx, binsy, miny, maxy);
    h->Fill(valuex, valuey);
    rootObj->Add(h);
  }
}

/*1D histogram fill wrapper for use with THashTable (faster)*/
void SFPAnalyzer::MyFill(string name, int binsx, double minx, double maxx, double valuex) {
  TH1F *histo = (TH1F*) rootObj->FindObject(name.c_str());
  if(histo != NULL) {
    histo->Fill(valuex);
  } else {
    TH1F *h = new TH1F(name.c_str(), name.c_str(), binsx, minx, maxx);
    h->Fill(valuex);
    rootObj->Add(h);
  }
}

/*Bulk of the work done here*/
void SFPAnalyzer::Run(const char *input, const char *output) {
  TFile* inputFile = new TFile(input, "READ");
  TTree* inputTree = (TTree*) inputFile->Get("SortTree");
  inputTree->SetBranchAddress("event", &event_address);

  TFile* outputFile = new TFile(output, "RECREATE");
  TTree* outputTree = new TTree("SPSTree", "SPSTree");
  outputTree->Branch("event", &pevent);
  GetWeights();
  Float_t place;
  Float_t blentries = inputTree->GetEntries();
  for(int i=0; i<inputTree->GetEntries(); i++) {
    inputTree->GetEntry(i);
    cevent = *event_address;
    place = ((Float_t)i)/blentries*100;
    /*Non-continuous progress update*/
    if(fmod(place, 10.0) == 0) cout<<"\rPercent of file processed: "<<place<<"%"<<flush;
    Reset();

    pevent.anodeFront = cevent.anodeLongF;
    pevent.anodeBack = cevent.anodeLongB;
    pevent.scintLeft = cevent.scintLongL;
    pevent.scintRight = cevent.scintLongR;
    pevent.cathode = cevent.cathodeLong;
    if(cevent.sabreFrontData.size() != 0) {
      pevent.sabreFrontE = cevent.sabreFrontData[0].Long;
      pevent.sabreChannelFront = cevent.sabreFrontData[0].Ch;
      pevent.sabreFrontTime = cevent.sabreFrontData[0].Time;
    }
    if(cevent.sabreBackData.size() != 0) {
      pevent.sabreBackE = cevent.sabreBackData[0].Long;
      pevent.sabreChannelBack = cevent.sabreBackData[0].Ch;
      pevent.sabreBackTime = cevent.sabreBackData[0].Time;
    }
    pevent.delayFrontRightE = cevent.delayLongFR;
    pevent.delayFrontLeftE = cevent.delayLongFL;
    pevent.delayBackRightE = cevent.delayLongBR;
    pevent.delayBackLeftE = cevent.delayLongBL;

    pevent.anodeFrontTime = cevent.anodeTimeF;
    pevent.anodeBackTime = cevent.anodeTimeB;
    pevent.scintLeftTime = cevent.scintTimeL;
    pevent.scintRightTime = cevent.scintTimeR;
    pevent.sabreFrontMult = cevent.sabreFrontMult;
    pevent.sabreBackMult = cevent.sabreBackMult;

    if(pevent.anodeBack != -1 && pevent.scintLeft != -1) {
      MyFill("anodeBack vs scintLeft",512,0,4096,pevent.scintLeft,512,0,4096,pevent.anodeBack);
    }
    if(cevent.delayTimeFL != -1 && cevent.delayTimeFR != -1) { 
      pevent.fp1_tdiff = (cevent.delayTimeFL-cevent.delayTimeFR)*0.5;
      pevent.fp1_tsum = (cevent.delayTimeFL+cevent.delayTimeFR);
      pevent.fp1_tcheck = (pevent.fp1_tsum)/2.0-cevent.anodeTimeF;
      pevent.delayFrontMaxTime = max(cevent.delayTimeFL, cevent.delayTimeFR);
      pevent.x1 = pevent.fp1_tdiff*1.0/1.83; //position from time, based on total delay
      //pevent.x1 = 0.52*pevent.fp1_tdiff - 0.128; //position from time, based on delay chips
      MyFill("x1",1200,-300,300,pevent.x1);
      MyFill("x1 vs anodeBack",600,-300,300,pevent.x1,512,0,4096,pevent.anodeBack);
    }
    if(cevent.delayTimeBL != -1 && cevent.delayTimeBR != -1) {
      pevent.fp2_tdiff = (cevent.delayTimeBL-cevent.delayTimeBR)*0.5;
      pevent.fp2_tsum = (cevent.delayTimeBL+cevent.delayTimeBR);
      pevent.fp2_tcheck = (pevent.fp2_tsum)/2.0-pevent.anodeBackTime;
      pevent.delayBackMaxTime = max(cevent.delayTimeBL, cevent.delayTimeBR);
      pevent.x2 = pevent.fp2_tdiff*1.0/1.96; //position from time, based on total delay
      //pevent.x2 = 0.48*pevent.fp2_tdiff - 2.365; //position from time, based on delay chips
      MyFill("x2",1200,-300,300,pevent.x2);
      MyFill("x2 vs anodeBack",600,-300,300,pevent.x2,512,0,4096,pevent.anodeBack);
    }
    if(pevent.x1 != -1e6 && pevent.x2 != -1e6) {
      pevent.xavg = pevent.x1*w1+pevent.x2*w2;
      MyFill("xavg",1200,-300,300,pevent.xavg);
      pevent.theta = atan((pevent.x2-pevent.x1)/36.0);
      if(pevent.theta<0) pevent.theta = pevent.theta + TMath::Pi()/2.0;
      MyFill("xavg vs theta",600,-300,300,pevent.xavg,314,0,3.14,pevent.theta);
      MyFill("x1 vs x2",600,-300,300,pevent.x1,600,-300,300,pevent.x2);
    }
    if(cevent.anodeTimeF != -1 && cevent.scintTimeR != -1) {
      pevent.fp1_y = pevent.anodeFrontTime-pevent.scintRightTime;
    }
    if(cevent.anodeTimeB != -1 && cevent.scintTimeR != -1) {
      pevent.fp2_y = pevent.anodeBackTime-pevent.scintRightTime;
    }
    outputTree->Fill();
  }
  cout<<endl;
  outputFile->cd();
  rootObj->Write();
  delete rootObj;
  outputTree->Write(outputTree->GetName(), TObject::kOverwrite);
  outputFile->Close();
  inputFile->Close();
  delete outputFile;
  delete inputFile;
}
