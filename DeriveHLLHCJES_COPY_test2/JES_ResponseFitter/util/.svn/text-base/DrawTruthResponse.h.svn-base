/*
 *  Example code using the JES_BalanceFitter
 *  To study the jet response distributions from JetPerformance
 *  Five key observables are examined: 
 *    mean, mode, width, chi2 of the fits and 0, 1 and 2 sigma quantiles
 *
 *  Dag Gillberg, Oct 12, 2012
 */
#include <TSystem.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TString.h>
#include <TError.h>
#include <vector>
#include <map>

using namespace std;
typedef TString Str;
typedef std::vector<TString> StrV;
typedef std::map<TString,TH1D*> HistMap;

HistMap _histos;

// Helpers
void add(StrV &v, Str a) { v.push_back(a); }
void add(StrV &v, Str a, Str b) { add(v,a); add(v,b); }
void add(StrV &v, Str a, Str b, Str c) { add(v,a,b); add(v,c); }
StrV Vectorize(Str a, Str b) { StrV v; add(v,a,b); return v; }
StrV Vectorize(Str a, Str b, Str c) { StrV v; add(v,a,b); add(v,c); return v; }
StrV Vectorize(Str a, Str b, Str c, Str d) { StrV v; add(v,a,b); add(v,c,d); return v; }
StrV Vectorize(Str a, Str b, Str c, Str d, Str e) { StrV v; add(v,a,b,c); add(v,d,e); return v; }
StrV Vectorize(Str a, Str b, Str c, Str d, Str e, Str f) { StrV v; add(v,a,b,c); add(v,d,e,f); return v; }

void MakeHisto(TString hname, std::vector<double> ptBins, TString ytit="");
TH1 *FormatHisto(TH1 *h, TString xtit, TString ytit="", int style=0);
TString GetJetDesc(TString jetAlgo) {
  return Form("Anti-k_{t} #it{R} = 0.%d, %s+JES",jetAlgo.Contains("Kt4")?4:6,jetAlgo.Contains("EM")?"EM":"LCW");
}

int cols[] = {kBlack, kRed, kBlue, kGreen+1};
int ms[]   = {20, 21, 22, 23};

TH1* FormatHisto(TH1 *h, TString xtit, TString ytit, int style) {
  h->SetXTitle(xtit); h->SetYTitle(ytit);
  h->SetStats(0); h->SetTitle("");
  h->SetLineColor(cols[style%4]); h->SetMarkerColor(cols[style%4]);
  h->SetMarkerStyle(ms[style%4]); h->SetMarkerSize(0.8); h->SetLineWidth(2);
  TAxis *ax = h->GetXaxis(), *ay = h->GetYaxis();
  ax->SetTitleFont(42); ax->SetLabelFont(42); ay->SetTitleFont(42); ay->SetLabelFont(42);
  ax->SetTitleOffset(1.2); ay->SetTitleOffset(1.4);
  if (xtit.Contains("GeV")) ax->SetMoreLogLabels(); return h;
}

void MakeHisto(TString hname, std::vector<double> ptBins, TString ytit) {
  _histos[hname] = new TH1D(hname,"",ptBins.size()-1,&ptBins[0]);
  TH1D *h = _histos[hname];
  h->SetXTitle("#it{p}_{T}^{truth}"); h->SetYTitle(ytit);
}

void DrawText(double x, double y, Str txt, int col=kBlack) {
  static TLatex *tex = new TLatex(); tex->SetNDC(); tex->SetTextFont(42);
  tex->SetTextSize(0.04);
  tex->SetTextColor(col); tex->DrawLatex(x,y,txt);
}
void DrawTextLeft(Str txt, double y, int col=kBlack) 
{ DrawText(0.15,y,txt,col); }
void DrawTextRight(Str txt, double y,int col=kBlack)
{ DrawText(0.7,y,txt,col); }

