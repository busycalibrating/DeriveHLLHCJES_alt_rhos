#ifndef PlottingTools_PlottingTools_h
#define PlottingTools_PlottingTools_h

#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>

#include <TCanvas.h>
#include <TString.h>
#include <TFile.h>
#include <TFrame.h>
#include <TGraphErrors.h>
#include <TH2D.h>
#include <TH1D.h>
#include <TStyle.h>
#include "TLatex.h"
#include <TLine.h>

#include "SystTool/SystTool.h"
#include "SystTool/SystContainer.h"
#include "SystTool/Utils.h"

#include "JES_ResponseFitter/JES_BalanceFitter.h"

using namespace std;

namespace PlottingTools {

  // ___________________________________________________________

  void PrepareTwoPadCanvas( TCanvas* canvas,
                            TString xTitle, TString yTitle1, TString yTitle2,
                            float xmin, float xmax,
                            float ymin1, float ymax1,
                            float ymin2, float ymax2,
                            int nDivisions);

  void PrepareOnePadCanvas( TCanvas* canvas,
                            TString xTitle, TString yTitle,
                            float xmin, float xmax,
                            float ymin, float ymax,
                            bool setLogX = false, bool setLogY = false );

  TGraphErrors* MakeGraphFromHisto(TH1D *histo, TString name, bool ignoreYerrors, bool ignoreXerrors, bool abs);

  void SetGraphStyle(TGraph* gr, int mstyle, float msize, int mcol, int lstyle, int lcol);

  void PrintLatex( char* text,
                   double xpos, double ypos,
                   double xmax, double xmin,
                   double ymax, double ymin);

  void ResponseVs( TH2D* hist2d, TCanvas* RespVsCan, TCanvas* binnedRespCan, TGraphErrors* RespVs, char *prefix, char *variable,bool isdPhi=false);
  TLine *MakeTLine(double x1, double y1, double x2, double y2, int col = 2);


  void CopyHistoStyle(TH1D *h1, TH1D *h2);
  void CopyHistoStyle(TH1D *h1, TGraph *h2);
  void CopyHistoStyle(TGraph *h1, TH1D *h2);
  TF1 *PlotConstantLine( float y, float xmin, float xmax, int lineType, int lineWidth );

  // ___________________________________________________________

  vector<double> vectorize(string bins)
  {
    istringstream stream(bins);
    string bin;
    vector<double> output;

    while (getline(stream, bin, ',')) {
        output.push_back( atof(bin.c_str()) );
    }
    return output;
  }

  vector<string> vectorizeStr(string bins)
  {
    istringstream stream(bins);
    string bin;
    vector<string> output;

    while (getline(stream, bin, ',')) {
        while (bin[0] == ' ') bin.erase(bin.begin());
        output.push_back(bin);
    }
    return output;
  }


  // ___________________________________________________________

  bool EndsWith(const std::string &str, const std::string &suffix)
  {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
  }

  // ___________________________________________________________

  vector<double> GetBins(TH1D* hist)
  {
    vector<double> bins;
    for (int i = 0; i < hist->GetNbinsX(); ++i) {
      bins.push_back(hist->GetBinLowEdge(i));
    }
    return bins;
  }

}

#endif


