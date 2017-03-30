#ifndef jes_balancefitter_h
#define jes_balancefitter_h

/*
 *  Common code to fit jet response distributions
 *  for example direct balance or asymmetry distributions
 *    pTreco/pTtruth, pTreco/pTref, pTref = photon or Z pT
 *    Asym = (pT1-pT2)/(pT1+pT2)*2
 *
 *  Dag Gillberg, Oct 21, 2012 dag.gillberg AT cern.ch
 */

#include <TString.h>
#include <TH1D.h>
#include <TF1.h>
#include <TPad.h>
#include <TLatex.h>
#include <TMath.h>
#include <stdio.h>
#include <iostream>
#include "Riostream.h"

class JES_BalanceFitter : public TNamed 
{
 public:
  
  typedef TString Str;
  
  
  //  Constructor. 
  //  Parameters:
  //  - Nsigma: how many sigmas, or Poisson effective widths
  JES_BalanceFitter(double Nsigma=1.6); 
  ~JES_BalanceFitter() {};
  
  TF1* Fit(TH1 *h, double fitMin=-1);
  TF1* FitNoRebin(TH1 *h, double fitMin=-1);
  int OptimalRebin(TH1 *h);
  void DrawFitAndHisto();
  void DrawExtendedFit(double minx=0, double maxx=3);
  void FitAndDraw(TH1 *h, double fitMin=-1) // TF1* should be returned??
  { Fit(h,fitMin); DrawFitAndHisto(); };

  
  void SetGaus() { m_param="gaus"; m_fitDesc="Gaussian"; };
  void SetPoisson() { m_param="[0]*2.8*TMath::Poisson(x/pow([2],2),[1]/pow([2],2))"; m_fitDesc="Modified Poisson"; }; 
  void SetFitOpt(Str opt) { m_fitOpt=opt; };
  
  // Get the mean of the balance distribution fit and its error
  double GetMean();
  double GetMeanError();

  // The x-position of the peak
  double GetPeak();
  double GetMedian();
  double GetQuantile(double frac);
  double GetNeg2SigQuantile() { return GetQuantile(2.27501319480000186e-02); };
  double GetNeg1SigQuantile() { return GetQuantile(1.58655253931499984e-01); };
  double GetPos1SigQuantile() { return GetQuantile(8.41344746068499960e-01); };
  double GetPos2SigQuantile() { return GetQuantile(9.77249868051999981e-01); };
  double GetHistoQuantile(double frac, Int_t verbose=0);  // Bogdan's implementation based on Root functions, with log(#bins) complexity
  //  double GetHistoQuantile(double frac);
  double GetHistoQuantileError() { return GetFineHisto()->GetBinWidth(1)/2; };
  double GetHistoMedian() { return GetHistoQuantile(0.5); };
  double GetNeg2SigHistoQuantile() { return GetHistoQuantile(2.27501319480000186e-02); };
  double GetNeg1SigHistoQuantile() { return GetHistoQuantile(1.58655253931499984e-01); };
  double GetPos1SigHistoQuantile() { return GetHistoQuantile(8.41344746068499960e-01); };
  double GetPos2SigHistoQuantile() { return GetHistoQuantile(9.77249868051999981e-01); };
  
  /*
  double GetNeg2SigQuantile() { return GetQuantile(0.045500); }
  double GetNeg1SigQuantile() { return GetQuantile(0.317310); }
  double GetPos1SigQuantile() { return GetQuantile(0.682689); }
  double GetPos2SigQuantile() { return GetQuantile(0.954499); }
  */

  // Get the width of the balance distribution fit and its error
  double GetSigma();
  double GetSigmaError();

  double GetChi2();
  double GetNdof();
  double GetChi2Ndof();
  
  void error(Str msg) {
    printf("JES_BalanceFitter ERROR:\n   %s\n\n",msg.Data()); abort();
  };

  TF1* GetFit();
  TH1* GetHisto();
  TH1* GetFineHisto();

  double GetHistoMean() { return GetFineHisto()->GetMean(); };
  double GetHistoMeanError() { return GetFineHisto()->GetMeanError(); };
  

  void SetFitColor(int col) { m_fitCol=col; };
  void DrawText(double x, double y, Str txt, int col=kBlack);
  void DrawTextLeft(Str txt, int col=kBlack);
  void DrawTextRight(Str txt, int col=kBlack);
  void ResetTextCounters() { m_lNlines=0; m_rNlines=0; };
  void PrintFitInfo();

  ClassDef(JES_BalanceFitter,1);

 private:
  TF1 *OnlyFit(TH1 *histo, double fitMin);
  void SetFitRange(double fitMin);
  double m_Nsigma, m_minPt;
  TF1 *m_fit;
  TH1 *m_histo, *m_fitHisto;
  Str m_param, m_fitOpt, m_fitDesc;
  int m_fitCol, m_lNlines, m_rNlines;
  //
  double m_min0, m_max0, m_xLeft, m_xRight, m_yTop, m_dy;
};

#endif
