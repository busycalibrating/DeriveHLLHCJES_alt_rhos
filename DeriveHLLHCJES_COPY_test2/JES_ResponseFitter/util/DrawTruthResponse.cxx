#include "JES_ResponseFitter/JES_BalanceFitter.h"
#include "util/DrawTruthResponse.h"

/*
 *  Example code using the JES_BalanceFitter
 *  To study the jet response distributions from JetPerformance
 *  Five key observables are examined: 
 *    mean, mode, width, chi2 of the fits and 0, 1 and 2 sigma quantiles
 *
 *  Dag Gillberg, Oct 12, 2012
 */

void error(Str msg) { printf("\nERROR:\n\n  %s\n\n",msg.Data()); abort(); }

void BookHistos(Str prefix, vector<double> ptBins);
void SaveToSummaryHisto(Str prefix, int bin, JES_BalanceFitter *fitter);
void SetBinContent(Str hname, int bin, double y, double err);
void DrawHistoCompare(Str jetAlgo, Str var, Str ytit="", double min=0.8, double max=1.2);
void DrawQuantileCompare(Str jetAlgo);

void DrawJetFlav(Str flavDesc) { DrawText(0.7,0.97,flavDesc); }
void DrawJetAndEtaBin(Str jetAlgo, double etaMin, double etaMax, Str jetFlavDesc="") {
  DrawTextLeft(GetJetDesc(jetAlgo),0.9);
  DrawTextLeft(Form("%.1f #leq |#it{#eta}| < %.1f",etaMin,etaMax),0.85);
  DrawJetFlav(jetFlavDesc);
}

int main() {
  gErrorIgnoreLevel=2000;
  double NsigmaForFit = 1.6;
  JES_BalanceFitter *myFitter = new JES_BalanceFitter(NsigmaForFit);

  Str in_fn = "example_code/Pythia_EM_LC_correctPythiaDijets_OriginalBinning_ResponseHistograms.root";
  TFile *file = TFile::Open(in_fn); if (file==NULL) error("Cannot open "+in_fn);
  //TFile *file = TFile::Open("Pythia_EM_LC_correctPythiaDijets_ResponseHistograms.root");
  TCanvas *can = new TCanvas();
  can->SetMargin(0.12,0.04,0.12,0.04);

  StrV jetFlavs = Vectorize("All","Light","Glu","Other");
  StrV jetFlavDescs = Vectorize("inclusive jets","light-quark jets","gluon jets","unmatched jets");

  // Set flavor
  Str jetFlav = jetFlavs[0], jetFlavDesc=jetFlavDescs[0];

  Str name="TruthBalanceFits_"+jetFlav;
  Str ps=name+".ps";
  Str psSummary=name+"_Summary.ps";
  can->Print(ps+"["); can->Print(psSummary+"[");

  StrV jetAlgos = Vectorize("AntiKt4EMTopo","AntiKt4LCTopo");
  StrV etaBins = Vectorize("0_0.3","0.3_0.8","0.8_1.2","1.2_2.1");
  add(etaBins,"2.1_2.8","2.8_3.6","3.6_4.4");
  double etas[8] = {0.0,0.3,0.8,1.2,2.1,2.8,3.6,4.4};

  // 
  const int NptBins = 13;
  double ptVec[NptBins+1] = {15, 20, 30, 45, 60, 80, 110, 160, 210, 260, 310, 400, 500, 600};
  std::vector<double> ptBins(ptVec,ptVec+NptBins+1);

  for (unsigned ei=0;ei<etaBins.size();++ei) {
    for (unsigned ji=0;ji<jetAlgos.size();++ji) {
      TString jetAlgo = jetAlgos[ji];
      for (int mi=0;mi<2;++mi) {
         Str method=mi==0?"gaus":"pois";
         BookHistos(jetAlgo+Form("_etaBin%d_",ei)+method,ptBins);
      }
    }
  }

  // 1. Loop over eta bins
  for (unsigned ei=0;ei<etaBins.size();++ei) {
    double etaMin=etas[ei], etaMax=etas[ei+1];
    Str etaBin=Form("_etaBin%d",ei);

    // 2. Loop over jet algorithms
    for (unsigned ji=0;ji<jetAlgos.size();++ji) {
    //for (int ji=0;ji<1;++ji) {
      Str jetAlgo = jetAlgos[ji], prefix = jetAlgo+etaBin;
    
      printf("Processing %s ...\n",prefix.Data());


      // 3. Loop over pT-truth bins
      double minPt = jetAlgo.Contains("EM") ? 13.0 : 10.0;
      for (int pti=0;pti<NptBins;++pti) {
	//for (int pti=4;pti<5;++pti) {
	can->SetLogx(0);
	double ptLow = ptVec[pti], ptHigh = ptVec[pti+1];

	TString hname=Form("%s_%s/respVsPt%s%s_absEta_%s_NPV_0_50_pt_%.0f_%.0f",
			   jetAlgo.Data(),jetFlav.Data(),jetFlav.Data(),jetAlgo.Data(),
			   etaBins[ei].Data(),ptLow,ptHigh);
	TH1D *h = (TH1D*)file->Get(hname);
	if (ei>0&&h==NULL) continue;
	if ( h->GetEffectiveEntries() < 100) continue;
	if (h==NULL) { printf("Cannot access histo %s\n",hname.Data()); abort(); }
	FormatHisto(h,"#it{p}_{T}^{reco} / #it{p}_{T}^{truth}","Frequency");
      
	myFitter->SetGaus(); myFitter->SetFitColor(kRed);
	myFitter->Fit(h,minPt/ptLow); 
	SaveToSummaryHisto(prefix+"_gaus",pti+1,myFitter);
	myFitter->DrawFitAndHisto();
      
	// Poission fit - draw on same canvas
	myFitter->SetPoisson(); myFitter->SetFitColor(kBlue);
	myFitter->Fit(h,minPt/ptLow); 
	//myFitter->GetFit()->Draw("same");
	myFitter->DrawExtendedFit();
	myFitter->DrawTextRight("");
	myFitter->PrintFitInfo();
	SaveToSummaryHisto(prefix+"_pois",pti+1,myFitter);

	myFitter->DrawTextLeft(GetJetDesc(jetAlgo));
	myFitter->DrawTextLeft(Form("%.0f #leq #it{p}_{T}^{truth} < %.0f GeV",ptLow,ptHigh));
	myFitter->DrawTextLeft(Form("%.1f #leq |#it{#eta}| < %.1f",etaMin,etaMax));
	int nbins=h->GetNbinsX();
	double uf=h->GetBinContent(0)/h->Integral(0,nbins+1), of=h->GetBinContent(nbins+1)/h->Integral(0,nbins+1);
	myFitter->DrawTextLeft(Form("uf/of: %.2f%% / %.2f%%",uf*100,of*100));
	DrawJetFlav(jetFlavDesc);
	can->Print(ps);
      } // for each pt bin
      can->SetLogx();

      // Draw summary
      DrawHistoCompare(prefix,"mean","mean of fit",0.9,1.1);
      DrawJetAndEtaBin(jetAlgo,etaMin,etaMax,jetFlavDesc); DrawTextRight("Gauss",0.9,cols[1]); DrawTextRight("Poisson",0.85,cols[2]);
      can->Print(ps); can->Print(psSummary);

      DrawHistoCompare(prefix,"peak","peak (mode) of fit",0.9,1.1);
      DrawJetAndEtaBin(jetAlgo,etaMin,etaMax,jetFlavDesc); DrawTextRight("Gauss",0.9,cols[1]); DrawTextRight("Poisson",0.85,cols[2]);
      can->Print(ps); can->Print(psSummary);

      DrawHistoCompare(prefix,"jer","#sigma/mean of fit (JER)",0.0,0.4);
      DrawJetAndEtaBin(jetAlgo,etaMin,etaMax,jetFlavDesc); DrawTextRight("Gauss",0.9,cols[1]); DrawTextRight("Poisson",0.85,cols[2]);
      can->Print(ps); can->Print(psSummary);

      DrawHistoCompare(prefix,"chi2","#it{#chi}^{2}/#it{n}_{dof} of fit",0.0,5);
      DrawJetAndEtaBin(jetAlgo,etaMin,etaMax,jetFlavDesc); DrawTextRight("Gauss",0.9,cols[1]); DrawTextRight("Poisson",0.85,cols[2]);
      DrawTextLeft(Form("Fit: #pm%.2f#sigma, min #it{p}_{T} = %.0f GeV",NsigmaForFit,minPt),0.8);
      can->Print(ps); can->Print(psSummary);

      //DrawHistoCompare(jetAlgo,"histmedian","median of histogram",0.9,1.1);
      DrawQuantileCompare(prefix);
      DrawText(0.25,0.9,GetJetDesc(jetAlgo)); 
      DrawText(0.25,0.85,Form("%.1f #leq |#it{#eta}| < %.1f",etaMin,etaMax));
      DrawText(0.7,0.97,jetFlavDesc);
      DrawTextRight("Raw distribution",0.9,cols[0]); DrawTextRight("Gaussian fit",0.85,cols[1]); DrawTextRight("Poisson fit",0.8,cols[2]);
      can->Print(ps); can->Print(psSummary);

    } // for each jet algo

  } // for each eta bin
  
  cout << "\nAll done!" << endl;

  can->Print(ps+"]"); can->Print(psSummary+"]");
  cout << "\n  Converting " << ps << endl;
  gSystem->Exec(Form("ps2pdf %s",ps.Data()));
  gSystem->Exec(Form("rm -f %s",ps.Data()));
  cout << "\n  Converting " << psSummary << endl;
  gSystem->Exec(Form("ps2pdf %s",psSummary.Data()));
  gSystem->Exec(Form("rm -f %s",psSummary.Data()));

  printf("\nProduced\n   %s\nand\n   %s\n\n",
	 gSystem->BaseName(ps.ReplaceAll("ps","pdf")),
	 gSystem->BaseName(psSummary.ReplaceAll("ps","pdf")));
  return 0;
}

void SetBinContent(Str hname, int bin, double y, double err) {
  TH1D* h = _histos[hname];
  if (h==NULL) { printf("Cannot access histo %s\n",hname.Data()); abort(); }
  h->SetBinContent(bin,y); h->SetBinError(bin,err);
}

void BookHistos(Str prefix, vector<double> bins) {
  MakeHisto(prefix+"_mean",bins,"#LT #it{p}_{T}^{reco} / #it{p}_{T}^{truth} #GT");
  MakeHisto(prefix+"_peak",bins,"Peak: #LT #it{p}_{T}^{reco} / #it{p}_{T}^{truth} #GT");
  MakeHisto(prefix+"_jer",bins,"#sigma(#it{p}_{T}^{reco} / #it{p}_{T}^{truth})");
  MakeHisto(prefix+"_chi2",bins,"#it{#chi}^{2}/#it{n}_{dof}");
  // Quantiles ...
  MakeHisto(prefix+"_median",bins,"Median of #it{p}_{T}^{reco} / #it{p}_{T}^{truth}");
  MakeHisto(prefix+"_pos1sigQuantile",bins,"+1#sigma quantile of #it{p}_{T}^{reco} / #it{p}_{T}^{truth}");
  MakeHisto(prefix+"_neg1sigQuantile",bins,"-1#sigma quantile of #it{p}_{T}^{reco} / #it{p}_{T}^{truth}");
  MakeHisto(prefix+"_pos2sigQuantile",bins,"+2#sigma quantile of #it{p}_{T}^{reco} / #it{p}_{T}^{truth}");
  MakeHisto(prefix+"_neg2sigQuantile",bins,"-2#sigma quantile of #it{p}_{T}^{reco} / #it{p}_{T}^{truth}");
  MakeHisto(prefix+"_histmedian",bins,"Median of #it{p}_{T}^{reco} / #it{p}_{T}^{truth}");
  MakeHisto(prefix+"_histpos1sigQuantile",bins,"+1#sigma quantile of #it{p}_{T}^{reco} / #it{p}_{T}^{truth}");
  MakeHisto(prefix+"_histneg1sigQuantile",bins,"-1#sigma quantile of #it{p}_{T}^{reco} / #it{p}_{T}^{truth}");
  MakeHisto(prefix+"_histpos2sigQuantile",bins,"+2#sigma quantile of #it{p}_{T}^{reco} / #it{p}_{T}^{truth}");
  MakeHisto(prefix+"_histneg2sigQuantile",bins,"-2#sigma quantile of #it{p}_{T}^{reco} / #it{p}_{T}^{truth}");
}

void SaveToSummaryHisto(Str prefix, int ptBin, JES_BalanceFitter *fitter) {
  SetBinContent(prefix+"_mean",ptBin,fitter->GetMean(),fitter->GetMeanError());
  SetBinContent(prefix+"_peak",ptBin,fitter->GetPeak(),fitter->GetMeanError());
  SetBinContent(prefix+"_jer",ptBin,
		fitter->GetSigma()/fitter->GetMean(),
		fitter->GetSigmaError()/fitter->GetMean());
  SetBinContent(prefix+"_chi2",ptBin,fitter->GetChi2Ndof(),0);
  SetBinContent(prefix+"_median",ptBin,fitter->GetMedian(),0);
  SetBinContent(prefix+"_neg2sigQuantile",ptBin,fitter->GetNeg2SigQuantile(),0);
  SetBinContent(prefix+"_neg1sigQuantile",ptBin,fitter->GetNeg1SigQuantile(),0);
  SetBinContent(prefix+"_pos1sigQuantile",ptBin,fitter->GetPos1SigQuantile(),0);
  SetBinContent(prefix+"_pos2sigQuantile",ptBin,fitter->GetPos2SigQuantile(),0);
  SetBinContent(prefix+"_histmedian",ptBin,fitter->GetHistoMedian(),fitter->GetHistoQuantileError());
  SetBinContent(prefix+"_histneg2sigQuantile",ptBin,fitter->GetNeg2SigHistoQuantile(),fitter->GetHistoQuantileError());
  SetBinContent(prefix+"_histneg1sigQuantile",ptBin,fitter->GetNeg1SigHistoQuantile(),fitter->GetHistoQuantileError());
  SetBinContent(prefix+"_histpos1sigQuantile",ptBin,fitter->GetPos1SigHistoQuantile(),fitter->GetHistoQuantileError());
  SetBinContent(prefix+"_histpos2sigQuantile",ptBin,fitter->GetPos2SigHistoQuantile(),fitter->GetHistoQuantileError());
}

void DrawHistoCompare(Str jetAlgo, Str var, Str ytit, double min, double max) {
  if (ytit=="") ytit=var;
  TH1 *hp = _histos[jetAlgo+"_pois_"+var], *hg = _histos[jetAlgo+"_gaus_"+var];
  FormatHisto(hg,"#it{p}_{T}^{true} [GeV]",ytit,1); FormatHisto(hp,"#it{p}_{T}^{true} [GeV]",ytit,2);
  hg->GetYaxis()->SetRangeUser(min,max); hg->Draw(); hp->Draw("same");
}

void DrawQuantileCompare(Str jetAlgo) {
  TH1 *h_med = _histos[jetAlgo+"_pois_histmedian"]; if (h_med==NULL) { cout << jetAlgo << endl; abort(); }
  FormatHisto(h_med,"#it{p}_{T}^{true} [GeV]","#it{p}_{T}^{reco}/#it{p}_{T}^{true} 0#sigma, #pm1#sigma and #pm2#sigma quantiles",0);
  h_med->GetYaxis()->SetRangeUser(0.3,1.7); h_med->Draw();
  static const TString quants[] = {"median","neg2sigQuantile","neg1sigQuantile","pos1sigQuantile","pos2sigQuantile"};
  for (int i=1;i<5;++i) FormatHisto(_histos[jetAlgo+"_pois_hist"+quants[i]],"","",0)->Draw("same");
  for (int i=0;i<5;++i) FormatHisto(_histos[jetAlgo+"_gaus_"+quants[i]],"","",1)->Draw("samel");
  for (int i=0;i<5;++i) FormatHisto(_histos[jetAlgo+"_pois_"+quants[i]],"","",2)->Draw("samel");
}
