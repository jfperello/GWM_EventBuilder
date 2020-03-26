/*SFPAnalyzer.h
 *Class designed to analyze coincidence events. Currently only implemented for focal plane
 *data. Additional changes for SABRE would include this file and the sructure ProcessedEvent
 *in DataStructs.h. Based on code written by S. Balak, K. Macon, and E. Good.
 *
 *Gordon M. Oct. 2019
 *
 *Refurbished and updated Jan 2020 by GWM. Now uses both focal plane and SABRE data
 */
#ifndef SFPANALYZER_H
#define SFPANALYZER_H

#include <TROOT.h>
#include <TTree.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <iostream>
#include <vector>
#include <string>
#include <THashTable.h>
#include <TCutG.h>
#include "DataStructs.h"
#include "FP_kinematics.h"

using namespace std;

class SFPAnalyzer {

  public:
    SFPAnalyzer(int zt, int at, int zp, int ap, int ze, int ae, double ep, double angle,
                double b);
    ~SFPAnalyzer();
    void Run(const char *input, const char *output);

  private:
    void Reset(); //Sets ouput structure back to "zero"
    void GetWeights(); //weights for xavg

    /*Fill wrappers for use with THashTable*/
    void MyFill(string name, int binsx, double minx, double maxx, double valuex,
                             int binsy, double miny, double maxy, double valuey);
    void MyFill(string name, int binsx, double minx, double maxx, double valuex);

    CoincEvent cevent, *event_address; //Input branch address
    ProcessedEvent pevent, blank; //output branch and reset

    Int_t Zt, At, Zp, Ap, Ze, Ae;
    Double_t Ep, Angle, B;
    Double_t w1, w2;

    THashTable *rootObj; //root storage
};

#endif