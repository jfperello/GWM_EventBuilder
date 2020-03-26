/*GainMatcher.h
 *Class used to hold relevant information for performing gain matching
 *Based on work done by S. Balak, K. Macon, and E. Good from LSU
 *
 *Created Jan 2020 by GWM
 */

#ifndef GAIN_MATCHER_H
#define GAIN_MATCHER_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class GainMatcher {
  
  public:
    GainMatcher();
    ~GainMatcher();
    double GetScaler(int channel);

  private:
    void MakeVector();
    vector<double> scaler_map;
    string gain_file_name;
};

#endif