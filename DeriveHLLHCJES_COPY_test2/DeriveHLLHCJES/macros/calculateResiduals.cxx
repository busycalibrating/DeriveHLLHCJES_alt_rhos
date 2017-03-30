#include "CalibrationTools.h"

//for all residuals leave algorithm_position out
//rtcc is the rtag
void calculateResiduals(const char * rtcc = "",int algorithm_position = -1){
  TString rt(rtcc);
  TEnv* downloadE = new TEnv("Download.config"); //directing the download and weighting
  downloadE->Print();
  TString InputFolder = downloadE->GetValue("InputFolder","");
  std::cout<<"InputFolder: "<<InputFolder<<std::endl;
  //TString User = downloadE->GetValue("User","alkire");
  //std::cout<<"User: "<<User<<std::endl;
  TString Version = downloadE->GetValue("Version","VERSION");
  std::cout<<"Version: "<<Version<<std::endl;
  TString LocalConfig = downloadE->GetValue("LocalConfig","Local.config");
  TEnv* localE = new TEnv(LocalConfig);
  std::vector<TString> JetAlgos = Vectorize(localE->GetValue("JetAlgos",""));
  
  std::vector<TString> Rtags;
  if(rt== "") Rtags = Vectorize(downloadE->GetValue("Rtags",""));
  else Rtags.push_back(rt);
  //std::cout<<"InputFolder: "<<InputFolder<<std::endl;
  int total_calibrations = algorithm_position < 0 ? JetAlgos.size()*Rtags.size() : Rtags.size();

  system("mkdir "+InputFolder+"/residual");
  //system("rm -fr "+InputFolder+"/temp/*");
  int simultaneous = 9;
  int count = 1;
  std::cout<<"Beginning poor man's parallelization."<<std::endl;
  for (int alg = 0; alg < JetAlgos.size(); ++alg){
    TString posS = algorithm_position < 0 ? TString::Format("%i",alg) : TString::Format("%i",algorithm_position);
    for (int i = 0 ; i < Rtags.size(); ++i){
      TString PU("200");
      if(Rtags.at(i).Contains("r7288")||Rtags.at(i).Contains("r7482")||Rtags.at(i).Contains("r7485")||Rtags.at(i).Contains("r7768")||Rtags.at(i).Contains("r7700")||Rtags.at(i).Contains("r7703")) PU="140";
      if(Rtags.at(i).Contains("r7287")||Rtags.at(i).Contains("r7480")||Rtags.at(i).Contains("r7484")||Rtags.at(i).Contains("r7709")||Rtags.at(i).Contains("r7699")||Rtags.at(i).Contains("r7702")) PU="80";
      
      if(count%simultaneous==0 || count==total_calibrations){
	std::cout<<"RUNNING: Residual "+InputFolder+"/"+Rtags.at(i)+".root "+Version+Rtags.at(i)+" mu"+PU+".Residual "+posS+" MuTerm &> "+InputFolder+"/residual/MuTerm"+Rtags.at(i)+".output"<<std::endl;
	system("Residual "+InputFolder+"/"+Rtags.at(i)+".root "+Version+Rtags.at(i)+" mu"+PU+".Residual "+posS+" MuTerm &> "+InputFolder+"/residual/MuTerm"+Rtags.at(i)+".output");
      }else{
	std::cout<<"RUNNING: Residual "+InputFolder+"/"+Rtags.at(i)+".root "+Version+Rtags.at(i)+" mu"+PU+".Residual "+posS+" MuTerm &> "+InputFolder+"/residual/MuTerm"+Rtags.at(i)+".output &"<<std::endl;
	system("Residual "+InputFolder+"/"+Rtags.at(i)+".root "+Version+Rtags.at(i)+" mu"+PU+".Residual "+posS+" MuTerm &> "+InputFolder+"/residual/MuTerm"+Rtags.at(i)+".output &");
      } 
      count++;
    }
    if(algorithm_position >= 0) break;
  }
  count=1;
  for (int alg = 0; alg < JetAlgos.size(); ++alg){
    TString posS = algorithm_position < 0 ? TString::Format("%i",alg) : TString::Format("%i",algorithm_position);
    for (int i = 0 ; i < Rtags.size(); ++i){
      TString PU("200");
      if(Rtags.at(i).Contains("r7288")||Rtags.at(i).Contains("r7482")||Rtags.at(i).Contains("r7485")||Rtags.at(i).Contains("r7768")||Rtags.at(i).Contains("r7700")||Rtags.at(i).Contains("r7703")) PU="140";
      if(Rtags.at(i).Contains("r7287")||Rtags.at(i).Contains("r7480")||Rtags.at(i).Contains("r7484")||Rtags.at(i).Contains("r7709")||Rtags.at(i).Contains("r7699")||Rtags.at(i).Contains("r7702")) PU="80";
      
      if(count%simultaneous==0 || count==total_calibrations){
	std::cout<<"RUNNING: Residual "+InputFolder+"/"+Rtags.at(i)+".root "+Version+Rtags.at(i)+" npv"+PU+".Residual "+posS+" NPVTerm &> "+InputFolder+"/residual/NPVTerm"+Rtags.at(i)+".output"<<std::endl;
	system("Residual "+InputFolder+"/"+Rtags.at(i)+".root "+Version+Rtags.at(i)+" npv"+PU+".Residual "+posS+" NPVTerm &> "+InputFolder+"/residual/NPVTerm"+Rtags.at(i)+".output");
      }else{
	std::cout<<"RUNNING: Residual "+InputFolder+"/"+Rtags.at(i)+".root "+Version+Rtags.at(i)+" npv"+PU+".Residual "+posS+" NPVTerm &> "+InputFolder+"/residual/NPVTerm"+Rtags.at(i)+".output &"<<std::endl;
	system("Residual "+InputFolder+"/"+Rtags.at(i)+".root "+Version+Rtags.at(i)+" npv"+PU+".Residual "+posS+" NPVTerm &> "+InputFolder+"/residual/NPVTerm"+Rtags.at(i)+".output &");
      } 
      count++;
    }
    if(algorithm_position >= 0) break;
  }
  std::cout<<"All residuals started. May not finish immediately."<<std::endl;
}
