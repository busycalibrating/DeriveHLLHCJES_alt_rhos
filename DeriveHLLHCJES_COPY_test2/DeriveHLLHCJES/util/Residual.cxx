//Steve Alkire, alkire@cern.ch
//Usage: Residual input.root configs ResidualName default.NewResidual

#ifndef NEWRESIDUAL_H
#define NEWRESIDUAL_H
#include <iostream>
#include <stdio.h>
//#include <stdlib.h>

#include "TCut.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TList.h"
#include "TEventList.h"
#include "TROOT.h"
#include "TKey.h"
#include "TFile.h"
#include "TSystem.h"
#include "TColor.h"
#include "TLegend.h"
#include "TF2.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include "TH3F.h"
#include "TApplication.h"
#include "CalibTools.h"
#include "TStyle.h"
#include "TEnv.h"

#define NBINS 45

//////////////////////////////////////////////////
// etaFitBins

std::vector<double> etaFitBins; //important to include the upper bound bin
Double_t etaDependence(Double_t* x, Double_t* par)
{
  float xx = x[0];
  double par0new = par[0];
  for(int i = 1; i < etaFitBins.size()-1; ++i){
    if(xx < etaFitBins.at(i)) return par0new + par[i]*(xx-etaFitBins.at(i-1));
    par0new = par0new + par[i]*(etaFitBins.at(i) - etaFitBins.at(i-1));
  }
  return par0new + par[etaFitBins.size()-1]*(xx - etaFitBins.at(etaFitBins.size()-2));
}

//////////////////////////////////////////////////
// DeliverEta

TString DeliverEta(const char* path, const char* hist, const char* fit, TString eta){
  TFile* file = TFile::Open(path);
  TH1* h = (TH1*)gDirectory->Get(hist);

  TF1* term = h->GetFunction(fit);
  term->Print();
  TString m0(TString::Format("%0.4f",term->GetParameter(0)));
  TString m1(TString::Format("%0.4f",term->GetParameter(1))); TString m12("("+m0+"+"+m1+"*1.2)");
  TString m2(TString::Format("%0.4f",term->GetParameter(2))); TString m23("("+m0+"+"+m1+"*1.2+"+m2+"*0.9)");
  TString m3(TString::Format("%0.4f",term->GetParameter(3))); TString m34("("+m0+"+"+m1+"*1.2+"+m2+"*0.9+"+m3+"*0.7)");
  TString m4(TString::Format("%0.4f",term->GetParameter(4)));
  TString formula("((abs("+eta+")<1.2)*("+m0+"+"+m1+"*abs("+eta+"))+(abs("+eta+")>=1.2&&abs("+eta+")<2.1)*("+m12+"+"+m2+"*(abs("+eta+")-1.2))+(abs("+eta+")>=2.1&&abs("+eta+")<2.8)*("+m23+"+"+m3+"*(abs("+eta+")-2.1))+(abs("+eta+")>=2.8)*("+m34+"+"+m4+"*(abs("+eta+")-2.8)))");
  file->Close();
  return formula;
}

/*
TString DeliverEta(std::vector<double> npv, std::vector<double> mu, TString eta){

}
*/

int main(int argc, char* argv[]){
  //gROOT->ProcessLine(".L AtlasStyle.C");
  //gROOT->ProcessLine("SetAtlasStyle();");
  /////PROCESSING INPUT
  //

  std::cout << "-----> Begin Residual script" << std::endl;

  for (int n = 1; n < argc; n++) std::cout << n << ": " << argv[ n ] << std::endl;

  //hardcoded - Not using data from Download.config
  TEnv* downloadE = new TEnv("Download.config");
  //

  //////////////////////////////////////////////////
  // Input parsing

  // Format:
  // Residual <inputfile> <R offset corr name> <environment> <algs pos> <calculated term>
  TString InputFile  (argv[1]);                     // file to calculate residuals
  TString ResidualOffsetCorrectionName (argv[2]);   // ?
  TString envname    (argv[3]);                     // Environment name (FINEmu.Residual, FINEnpv, FINEconst...)
  int jetAlgsPosition = TString(argv[4]).Atoi();    // an integer (starts at 0 I believe
  TString Term(argv[5]);                            // which term you are calculating (mu, npv, const)

  std::cout<<"jetAlgsPosition: "<<jetAlgsPosition<<std::endl;

  TEnv* env = new TEnv(envname);                    // general environemnt (env) taken from 3rd line of input string
  TString mainS = env->GetValue("MainConfig","");
  TString localS = env->GetValue("LocalConfig",""); // Gets all paramaters from environment (arg3)
  TEnv* mainE = new TEnv(mainS);                    // Only used in getting Residual.Config
  TEnv* localE = new TEnv(localS);                  // Gets:  working dir
                                                    //        postfix - NOCUT
                                                    //        ResidualAbsEtaBins
                                                    //        JetAlgos (antikt4EM)

  TString path = localE->GetValue("OutputFolder","");
  //localE->GetValue("OutputFolder","");

  TString Postfix = localE->GetValue("Postfix","NOCUT");

  //system("mkdir "+path+"/"+ResidualOffsetCorrectionName);

  TString ResidualOffSetCalibFile = mainE->GetValue("ResidualOffset.CalibFile","Residual.config");
  //TString ResidualOffsetCorrectionName = env->GetValue("ResidualOffsetCorrection.Name","CHOOSERESIDUAL");
  TEnv* residualE = new TEnv(ResidualOffSetCalibFile);
  etaFitBins = VectorizeD(localE->GetValue("ResidualAbsEtaBins","0 1.2 2.1 2.8 6.0"));

  for(int i  =0 ; i < etaFitBins.size(); ++i){
    std::cout<<"ETAFIT:"<<etaFitBins.at(i)<<std::endl;
  }

  std::vector<TString> JetAlgos = Vectorize(localE->GetValue("JetAlgos",""));
  TString Tag = env->GetValue("Tag","");            // FINE, other associated tags (not sure)

  std::cout << "-----> Accepted input files" << std::endl;

  //////////////////////////////////////////////////
  // Initialized. Begin running stuff

  // All stufff here coming from mu.residual, npv.residual, const.residual etc.
  // Make one for each rho you calculate! (??)
  TString SubTerm0File = env->GetValue("SubTerm0File","");
  TString SubTerm0Hist = env->GetValue("SubTerm0Hist","SPECIFY SubTerm0Hist");
  TString SubTerm0Fit = env->GetValue("SubTerm0Fit","SPECIFY SubTerm0Fit");
  TString SubTerm0Var = env->GetValue("SubTerm0Var","1"); //put it in parens

  TString SubTerm1File = env->GetValue("SubTerm1File","");
  TString SubTerm1Hist = env->GetValue("SubTerm1Hist","SPECIFY SubTerm1Hist");
  TString SubTerm1Fit = env->GetValue("SubTerm1Fit","SPECIFY SubTerm1Fit");
  TString SubTerm1Var = env->GetValue("SubTerm1Var","1"); //put it in parens

  //gather initial hists
  TString IPar1 = env->GetValue("IPar1","ADD IPar1");
  TString SubTerm0 = (SubTerm0File == "") ? "1" : DeliverEta(path+"/"+SubTerm0File,SubTerm0Hist,SubTerm0Fit,IPar1);
  TString SubTerm1 = (SubTerm1File == "") ? "1" : DeliverEta(path+"/"+SubTerm1File,SubTerm1Hist,SubTerm1Fit,IPar1);

  std::cout <<"-----> Initial histograms gathered. Opening up input file..." << std::endl;
  //TString InputFile = env->GetValue("InputFile","NAME THE INPUT FILE");
  TFile*  file      = TFile::Open(InputFile);                                       // Opening up input file; abs path or local
  TString OutputFile = env->GetValue("OutputFile","NAME THE OUTPUT FILE");          // Creating the output file
  int NTest = env->GetValue("NTest",0);                                             // ?
  std::cout << JetAlgos.at(jetAlgsPosition) << " " << Postfix << std::endl;
  TTree* EventTree = (TTree*)file->Get(JetAlgos.at(jetAlgsPosition)+Postfix);       // Grab event tree (AntiKt4EMTopoNOCUT)
  //((TTree*)gDirectory->Get(env->GetValue("EventTree","EventTree")));
  int NTree = env->GetValue("NTree",0);                                             // This value is 0 in the config too?

  // Since NTree = 0 in configs and default, shouldn't run this loop
  for(int n = 0; n < NTree; ++n){
    TString treename = env->GetValue("Tree"+TString::Format("%i",n),"OOPS NO TREE");
    TTree* tree = (TTree*)gDirectory->Get(treename);
    std::cout<<"Adding Friend: "<<treename <<std::endl;
    EventTree->AddFriend(tree,treename);
  }
  ///////////////

  std::cout << "-----> Getting env variables" << std::endl;
  int NHPar = env->GetValue("NHPar", 2);
  int NIPar = env->GetValue("NIPar",2);
  int IPar0NBin = env->GetValue("IPar0NBin",1);
  std::vector<float> IPar0Bin;
  TString IPar0 = env->GetValue("IPar0","ADD IPar0");
  TList* IPar0BinS = new TList;
  for(int n = 0; n <= IPar0NBin; ++n){
    IPar0Bin.push_back(env->GetValue("IPar0Bin"+TString::Format("%i",n),(float)n));
    IPar0BinS->Add(new TObjString(env->GetValue("IPar0Bin"+TString::Format("%i",n),"ADD IPar0Bins")));
  }

  int IPar1NBin = env->GetValue("IPar1NBin",100);
  std::vector<float> IPar1Bin;
  TList* IPar1BinS = new TList;
  for(int n = 0; n <= IPar1NBin; ++n){
    IPar1Bin.push_back(env->GetValue("IPar1Bin"+TString::Format("%i",n),(float)n));
    IPar1BinS->Add(new TObjString(env->GetValue("IPar1Bin"+TString::Format("%i",n),"ADD IPar1Bins")));
  }

  std::cout << "-----> Getting more variables" << std::endl;

  TList* IPar0_list = PTools::CutList(IPar0BinS, IPar0);
  TList* IPar1_list = PTools::CutList(IPar1BinS, IPar1);
  TList* IPar1BinGausWithBinWidth_list = PTools::GausList(IPar1Bin, IPar1);
  new TCanvas;
  TString HPar0 = env->GetValue("HPar0","jM.pt - TM.pt");
  TString HPar0NBinS = env->GetValue("HPar0NBin","100");
  TString HPar0LowS = env->GetValue("HPar0Low","0.");
  TString HPar0HighS = env->GetValue("HPar0High","100.");
  TString HPar1 = env->GetValue("HPar1", "MU");
  TString HPar1FacS = env->GetValue("HPar1Fac", "1.0");
  TString HPar1NBinS = env->GetValue("HPar1NBin","25");
  TString HPar1LowS = env->GetValue("HPar1Low", "10.5");
  TString HPar1HighS = env->GetValue("HPar1High","34.5");
  TString HPar2 = env->GetValue("HPar2", "aM.pt * RHO");
  TString HPar2FacS = env->GetValue("HPar2Fac", "1.0");
  TString HPar2NBinS = env->GetValue("HPar2NBin","17");
  TString HPar2LowS = env->GetValue("HPar2Low", "6.5");
  TString HPar2HighS = env->GetValue("HPar2High","23.5");
  TString Cut = env->GetValue("Cut","1");

  std::cout << "-----> Got all of the parameters from the env. Now drawing the hists..." << std::endl;

  //The usual way to get the bins
  //TList* h_list = PTreeTools::Draw(IPar1_list,IPar0_list,EventTree,HPar0+"-"+SubTerm0+"*"+SubTerm0Var+"-"+SubTerm1+"*"+SubTerm1Var+":"+HPar1+"*"+HPar1FacS+":"+HPar2+"*"+HPar2FacS+">>area("+HPar2NBinS+","+HPar2LowS+"*"+HPar2FacS+","+HPar2HighS+"*"+HPar2FacS+","+HPar1NBinS+","+HPar1LowS+"*"+HPar1FacS+","+HPar1HighS+"*"+HPar1FacS+","+HPar0NBinS+","+HPar0LowS+","+HPar0HighS+")","("+Cut+")","",((bool)NTest) ? NTest : 100000000, ((bool)NTest) ? 2000000:0);
  //Trying a gaussian with bin width in order to handle low stats with more bins

  //////////////////////////////////////////////////
  //
  // Printing stuff
  //
  /////////////////////////

  std::cout<<"----IPar1BinGausWithBinWidth_list:-----"<<std::endl;
  IPar1BinGausWithBinWidth_list->Print();
  std::cout<<""<<std::endl;

  std::cout<<"-----IPar0_list:-----"<<std::endl;
  IPar0_list->Print();
  std::cout<<""<<std::endl;

  std::cout<<"-----EventTree:-----"<<std::endl;
  std::cout << EventTree << std::endl;
  std::cout<<""<<std::endl;

  std::cout<< "-----HPar0+-+SubTerm0+*+SubTerm0Var+-+SubTerm1+*+SubTerm1Var+:+HPar1+*+HPar1FacS+:+HPar2+*+HPar2FacS+>>area(+HPar2NBinS+,+HPar2LowS+*+HPar2FacS+,+HPar2HighS+*+HPar2FacS+,+HPar1NBinS+,+HPar1LowS+*+HPar1FacS+,+HPar1HighS+*+HPar1FacS+,+HPar0NBinS+,+HPar0LowS+,+HPar0HighS+) ,(+Cut+),:-----" <<std::endl;

  std::cout << HPar0+"-"+SubTerm0+"*"+SubTerm0Var+"-"+SubTerm1+"*"+SubTerm1Var+":"+HPar1+"*"+HPar1FacS+":"+HPar2+"*"+HPar2FacS+">>area("+HPar2NBinS+","+HPar2LowS+"*"+HPar2FacS+","+HPar2HighS+"*"+HPar2FacS+","+HPar1NBinS+","+HPar1LowS+"*"+HPar1FacS+","+HPar1HighS+"*"+HPar1FacS+","+HPar0NBinS+","+HPar0LowS+","+HPar0HighS+")" ",("+Cut+"),"<<std::endl;
  std::cout << " ----- " <<std::endl;

//  TList* h_list = PTreeTools::DrawWithFunctionWeight(IPar1BinGausWithBinWidth_list,EventTree,HPar0+"-"+SubTerm0+"*"+SubTerm0Var+"-"+SubTerm1+"*"+SubTerm1Var+":"+HPar1+"*"+HPar1FacS+":"+HPar2+"*"+HPar2FacS+">>area("+HPar2NBinS+","+HPar2LowS+"*"+HPar2FacS+","+HPar2HighS+"*"+HPar2FacS+","+HPar1NBinS+","+HPar1LowS+"*"+HPar1FacS+","+HPar1HighS+"*"+HPar1FacS+","+HPar0NBinS+","+HPar0LowS+","+HPar0HighS+")","("+Cut+")","",((bool)NTest) ? NTest : 100000000, ((bool)NTest) ? 2000000:0);

//////////////////////////////////////////////////

    TList* h_list = PTreeTools::DrawWithFunctionWeight(IPar1BinGausWithBinWidth_list,IPar0_list,EventTree,HPar0+"-"+SubTerm0+"*"+SubTerm0Var+"-"+SubTerm1+"*"+SubTerm1Var+":"+HPar1+"*"+HPar1FacS+":"+HPar2+"*"+HPar2FacS+">>area("+HPar2NBinS+","+HPar2LowS+"*"+HPar2FacS+","+HPar2HighS+"*"+HPar2FacS+","+HPar1NBinS+","+HPar1LowS+"*"+HPar1FacS+","+HPar1HighS+"*"+HPar1FacS+","+HPar0NBinS+","+HPar0LowS+","+HPar0HighS+")","("+Cut+")","",((bool)NTest) ? NTest : 100000000, ((bool)NTest) ? 2000000:0);
  TList* h_slice_list = PTools::Slice3D(h_list, "zx",2);

//////////////////////////////////////////////////

  //TList* h_mean_list = PTools::GetMeanFitPeak(h_slice_list,3);
  TList* h_mean_list = PTools::GetMean(h_slice_list,3);
  TString Fit = env->GetValue("Fit","[0]+[1]*x");
  float FitHigh = env->GetValue("FitHigh",30.);
  float FitLow = env->GetValue("FitLow",5.);
  TF1* fit = new TF1("fit",Fit,FitLow,FitHigh);
  fit->SetParameter(0,0.); fit->SetParameter(1,0.);

  TList* h_fit_mean_list = PTools::Fit(h_mean_list,fit,3);
  std::vector<float> HPar1Vec;
  int HPar1NBin = env->GetValue("HPar1NBin",25);
  float HPar1Low = env->GetValue("HPar1Low", 9.5);
  float HPar1High = env->GetValue("HPar1High",34.5);
  float interval = (HPar1High - HPar1Low)/((float)HPar1NBin);
  for(int i = 0; i < HPar1NBin; ++i){
    HPar1Vec.push_back(HPar1Low+(interval*(float)i));
  }
  HPar1Vec.push_back(HPar1High);
  TList* par_h_fit_mean_list = PTools::PlotMultiFitParams(h_fit_mean_list,HPar1Vec,2,2);
  TString Average = env->GetValue("Average","[0]");
  float AverageLow = env->GetValue("AverageLow",10.5);
  float AverageHigh = env->GetValue("AverageHigh",34.5);

  TF1* scalar = new TF1("scalar",Average,AverageLow,AverageHigh);;
  scalar->SetParameter(0,0.);
  TList* average = PTools::Fit((TList*)par_h_fit_mean_list->At(1),scalar,2);
  TList* par_average = PTools::PlotMultiFitParams(average,IPar0Bin,1,1);

  new TCanvas;
  TF1* IPar0Fit = new TF1("IPar0Fit","[0]+[1]*log(x/25.0)",20,200);
  IPar0Fit->SetParameter(0,0.); IPar0Fit->SetParameter(1,1.);

  TList* fit_IPar0Fit_list = PTools::Fit((TList*)par_average->At(0), IPar0Fit, 1);
  TList* par_fit_IPar0Fit_list = PTools::PlotFitParams(fit_IPar0Fit_list, IPar1Bin, 2);
  TF1* IPar1Fit = new TF1("IPar1Fit", etaDependence, 0., 5., etaFitBins.size());
  //TF1* ptDepFit = new TF1("ptDepFit", "[0]",
  //IPar1Fit->SetParameters(-0.05, 0., 0., 0., 0.);
  IPar1Fit->SetParameters(-0.05, 0., 0., 0., 0.);
  TFitResultPtr fr0;
  TFitResultPtr fr1;
  TF1* fit0_IPar1Fit = PTools::Fit((TH1*)par_fit_IPar0Fit_list->At(0), IPar1Fit, etaDependence, fr0);
  TF1* fit1_IPar1Fit = PTools::Fit((TH1*)par_fit_IPar0Fit_list->At(1), IPar1Fit, etaDependence, fr1);

  TH1* h_uncfit = new TH1D("uncfit","uncfit",NBINS,((TH1*)par_fit_IPar0Fit_list->At(0))->GetXaxis()->GetXmin(),((TH1*)par_fit_IPar0Fit_list->At(0))->GetXaxis()->GetXmax());
  TH1* h_uncfitP = new TH1D("uncfitP","uncfitP",NBINS,((TH1*)par_fit_IPar0Fit_list->At(0))->GetXaxis()->GetXmin(),((TH1*)par_fit_IPar0Fit_list->At(0))->GetXaxis()->GetXmax());

  double xxx[NBINS];
  for(int i = 0; i < NBINS; ++i){
    xxx[i] = h_uncfit->GetBinCenter(i);
  }

  double err[NBINS];  // error on the function at point x0
  double errP[NBINS];

  fr0->GetConfidenceIntervals(NBINS, 1, 1, xxx, err, 0.683, false);
  fr0->GetConfidenceIntervals(NBINS, 1, 1, xxx, errP, 0.683, true);

  for(int j = 1; j <= NBINS; ++j){
    h_uncfit->SetBinContent(j,fit0_IPar1Fit->Eval(h_uncfit->GetBinCenter(j)));
    h_uncfit->SetBinError(j,err[j-1]);
    h_uncfitP->SetBinContent(j,fit0_IPar1Fit->Eval(h_uncfitP->GetBinCenter(j)));
    h_uncfitP->SetBinError(j,errP[j-1]);
  }

  std::vector<double> fitVector;
  for(int j = 0; j < etaFitBins.size(); ++j){
    fitVector.push_back(fit0_IPar1Fit->GetParameter(j));
  }

  // Outputs the relevant term (MuTerm, NPV, etc) + tag (FINE) + Jet algo .config
  TEnv* outResidualE = new TEnv(path+"/residual/"+Term+Tag+ResidualOffsetCorrectionName+JetAlgos.at(jetAlgsPosition)+".config");


  if(Term.Contains("NPV")) outResidualE->SetValue(Tag+ResidualOffsetCorrectionName+JetAlgos.at(jetAlgsPosition)+".Description", ResidualOffsetCorrectionName+" mu- and N_{PV}-dependent jet pileup corrections");

  if(Term.Contains("NPV")) outResidualE->SetValue(Tag+ResidualOffsetCorrectionName+JetAlgos.at(jetAlgsPosition)+".AbsEtaBins",Devectorize(etaFitBins));
  outResidualE->SetValue(Tag+ResidualOffsetCorrectionName+JetAlgos.at(jetAlgsPosition)+"."+Term+"."+JetAlgos.at(jetAlgsPosition),Devectorize(fitVector));
  outResidualE->SaveLevel(kEnvLocal);


  PTools::Pause(path+"/residual/"+Term+Tag+ResidualOffsetCorrectionName+JetAlgos.at(jetAlgsPosition)+".root");

  return 0;
}
#endif //NEWRESIDUAL_H
