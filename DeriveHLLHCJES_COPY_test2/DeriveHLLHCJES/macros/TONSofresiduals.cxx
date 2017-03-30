#include "CalibrationTools.h"

//To run on all residuals leave algorithm_position out
//rtcc is the rtag
void TONSofresiduals(const char * rtcc = "",int algorithm_position = -1, const char* ver=""){
  std::vector<TString> residualTags;
  //residualTags.push_back("");
  residualTags.push_back("FINE");
  residualTags.push_back("GLOBAL_RHO");
  residualTags.push_back("RHOSTRIP_1R");
  residualTags.push_back("RHOSTRIP_2R");
  residualTags.push_back("RHOSTRIP_3R");
  residualTags.push_back("RHOCIRCLE_1R");
  residualTags.push_back("RHOCIRCLE_2R");
  residualTags.push_back("RHOCIRCLE_3R");
  residualTags.push_back("RHODOUGHNUT_1_3R");
  residualTags.push_back("RHODOUGHNUT_1_4R");
  residualTags.push_back("RHODOUGHNUT_2_4R");
  residualTags.push_back("RHODOUGHNUT_2_5R");
  // residualTags.push_back("RHOGRID");
  // residualTags.push_back("RHOTGRID");
  // residualTags.push_back("TOWER");
  // residualTags.push_back("TGRID");
  //-----REMOVE-----v
  //residualTags.push_back("NOSUB");
  //-----REMOVE-----^

  TString rt(rtcc);
  TEnv* downloadE = new TEnv("Download.config"); //directing the download and weighting
  downloadE->Print();
  TString LocalConfig = downloadE->GetValue("LocalConfig","Local.config");
  TEnv* localE = new TEnv(LocalConfig);
  TString InputFolder = localE->GetValue("OutputFolder","");
  std::cout<<"InputFolder: "<<InputFolder<<std::endl;
  //TString User = downloadE->GetValue("User","alkire");
  //std::cout<<"User: "<<User<<std::endl;
  TString verS(ver);
  TString Version = downloadE->GetValue("Version","VERSION");
  if(verS!=""){ Version = verS; InputFolder = "/tmp/alkire/"+Version;}

  std::cout<<"Version: "<<Version<<std::endl;
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
  std::cout<<" "<<std::endl;

  for(int tag = 0; tag < residualTags.size(); ++tag){
    for (int alg = 0; alg < JetAlgos.size(); ++alg){
      TString posS = algorithm_position < 0 ? TString::Format("%i",alg) : TString::Format("%i",algorithm_position);
      for (int i = 0 ; i < Rtags.size(); ++i){
	TString PU("");
	//if(Rtags.at(i).Contains("r7288")||Rtags.at(i).Contains("r7482")||Rtags.at(i).Contains("r7485")||Rtags.at(i).Contains("r7768")||Rtags.at(i).Contains("r7700")||Rtags.at(i).Contains("r7703")) PU="140";
	//if(Rtags.at(i).Contains("r7287")||Rtags.at(i).Contains("r7480")||Rtags.at(i).Contains("r7484")||Rtags.at(i).Contains("r7709")||Rtags.at(i).Contains("r7699")||Rtags.at(i).Contains("r7702")) PU="80";

	if(1 || count%simultaneous==0 || count==total_calibrations){
	  std::cout<<"RUNNING: Residual "+InputFolder+"/"+Rtags.at(i)+".root "+Version+Rtags.at(i)+" "+"residual_files/"+residualTags.at(tag)+"mu"+PU+".Residual "+posS+" MuTerm &> "+InputFolder+"/residual/MuTerm"+Rtags.at(i)+".output"<<std::endl;
	  //system("Residual "+InputFolder+"/"+Rtags.at(i)+".root "+Version+Rtags.at(i)+" "+"residual_files/"+residualTags.at(tag)+"mu"+PU+".Residual "+posS+" MuTerm");
	  system("Residual "+InputFolder+"/"+Rtags.at(i)+".root "+Version+Rtags.at(i)+" "+"residual_files/"+residualTags.at(tag)+"mu"+PU+".Residual "+posS+" MuTerm &> /dev/null &");
	} //"+InputFolder+"/residual/MuTerm"+Rtags.at(i)+".output
	count++;
      }
      if(algorithm_position >= 0) break;
    }
  }


  std::cout<<" "<<std::endl;
  count=1;

  for(int tag = 0; tag < residualTags.size(); ++tag){
    for (int alg = 0; alg < JetAlgos.size(); ++alg){
      TString posS = algorithm_position < 0 ? TString::Format("%i",alg) : TString::Format("%i",algorithm_position);
      for (int i = 0 ; i < Rtags.size(); ++i){
	TString PU("");
	//if(Rtags.at(i).Contains("r7288")||Rtags.at(i).Contains("r7482")||Rtags.at(i).Contains("r7485")||Rtags.at(i).Contains("r7768")||Rtags.at(i).Contains("r7700")||Rtags.at(i).Contains("r7703")) PU="140";
	//if(Rtags.at(i).Contains("r7287")||Rtags.at(i).Contains("r7480")||Rtags.at(i).Contains("r7484")||Rtags.at(i).Contains("r7709")||Rtags.at(i).Contains("r7699")||Rtags.at(i).Contains("r7702")) PU="80";

	if(1 || count%simultaneous==0 || count==total_calibrations){
	  std::cout<<"RUNNING: Residual "+InputFolder+"/"+Rtags.at(i)+".root "+Version+Rtags.at(i)+" "+"residual_files/"+residualTags.at(tag)+"npv"+PU+".Residual "+posS+" NPVTerm &> "+InputFolder+"/residual/NPVTerm"+Rtags.at(i)+".output"<<std::endl;
	  //system("Residual "+InputFolder+"/"+Rtags.at(i)+".root "+Version+Rtags.at(i)+" "+"residual_files/"+residualTags.at(tag)+"npv"+PU+".Residual "+posS+" NPVTerm");
	  system("Residual "+InputFolder+"/"+Rtags.at(i)+".root "+Version+Rtags.at(i)+" "+"residual_files/"+residualTags.at(tag)+"npv"+PU+".Residual "+posS+" NPVTerm &> /dev/null &");
	} //"+InputFolder+"/residual/NPVTerm"+Rtags.at(i)+".output
	count++;
      }
      if(algorithm_position >= 0) break;
    }
  }

  std::cout<<" "<<std::endl;

  for(int tag = 0; tag < residualTags.size(); ++tag){
    for (int alg = 0; alg < JetAlgos.size(); ++alg){
      TString posS = algorithm_position < 0 ? TString::Format("%i",alg) : TString::Format("%i",algorithm_position);
      for (int i = 0 ; i < Rtags.size(); ++i){
	TString PU("");
	//if(Rtags.at(i).Contains("r7288")||Rtags.at(i).Contains("r7482")||Rtags.at(i).Contains("r7485")||Rtags.at(i).Contains("r7768")||Rtags.at(i).Contains("r7700")||Rtags.at(i).Contains("r7703")) PU="140";
	//if(Rtags.at(i).Contains("r7287")||Rtags.at(i).Contains("r7480")||Rtags.at(i).Contains("r7484")||Rtags.at(i).Contains("r7709")||Rtags.at(i).Contains("r7699")||Rtags.at(i).Contains("r7702")) PU="80";

	if(1 || count%simultaneous==0 || count==total_calibrations){
	  std::cout<<"RUNNING: Linearize "+InputFolder+"/"+Rtags.at(i)+".root "+Version+Rtags.at(i)+" "+"residual_files/"+residualTags.at(tag)+"const"+".Residual "+posS+" ConstTerm &> "+InputFolder+"/residual/ConstTerm"+Rtags.at(i)+".output"<<std::endl;
	  //system("Linearize "+InputFolder+"/"+Rtags.at(i)+".root "+Version+Rtags.at(i)+" "+"residual_files/"+residualTags.at(tag)+"const"+".Residual "+posS+" ConstTerm");
	  system("Linearize "+InputFolder+"/"+Rtags.at(i)+".root "+Version+Rtags.at(i)+" "+"residual_files/"+residualTags.at(tag)+"const"+".Residual "+posS+" ConstTerm &> /dev/null &");
	} //"+InputFolder+"/residual/ConstTerm"+Rtags.at(i)+".output
	count++;
      }
      if(algorithm_position >= 0) break;
    }
  }
  std::cout<<" "<<std::endl;
  std::cout<<"All residuals started. Be patient. Use top."<<std::endl;
}
