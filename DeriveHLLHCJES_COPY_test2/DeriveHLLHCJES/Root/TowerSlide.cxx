//Steve Alkire
//17.05.2016
//Slightly different algorithm from Tom McCarthy and Sven Menke as far as I know
//Has been adjusted for efficiency
//Towers < 0 are removed before median --this is the wrong choice, hence the grid method


#include <vector>
#include <math.h>
//#include <algortihm>

#include "xAODCaloEvent/CaloTowerContainer.h"
#include "xAODCaloEvent/CaloTowerAuxContainer.h"

#include "DeriveHLLHCJES/TowerSlide.h"

#define GEV .001
#define TOWERAREA .01

ClassImp(TowerSlide)

//calculate rhos -- to be used once per event loop
//Hardcoded: 0.2 steps from -4.5 to 4.5 w/ window full width ~.8
//Grid is symmtric in eta.
void TowerSlide::OncePerEvent(const xAOD::CaloTowerContainer *tc){
  float GRIDR = 0.5;

  rho = new TH1F("rho","rho",45,-4.5,4.5);
  rhogrid = new TH1F("rhogrid","rhogrid",23,-.1,4.5);

  std::vector<float> densities[45];
  std::vector<fastjet::PseudoJet> pjs[23];

  for(int i = 0; i < tc->size(); ++i){
    int bin = (int) floor((tc->at(i)->eta()+4.5)/.2);
    float density = tc->at(i)->pt()*GEV;

    if(density>0){
      for(int b = bin-2; b <= bin+2; ++b){
	if(b >=0 && b < 45){
	  //std::cout<<"i/eta/bin/b/density "<<i<<" "<<tc->at(i)->eta()<<" "<<bin<<" "<<b<<" "<<density<<std::endl;

	  densities[b].push_back(density/TOWERAREA);
	  if(m_doGrid){
	    TLorentzVector t;
	    t.SetPtEtaPhiE(tc->at(i)->pt(),tc->at(i)->eta(),tc->at(i)->phi(),tc->at(i)->e());

	    fastjet::PseudoJet pj(t.Px()*GEV,t.Py()*GEV,t.Pz()*GEV,t.E()*GEV);
	    pjs[b > 22 ? b-22 : 22-b].push_back(pj);
	  }
	}
      }
    }
  }

  for(int i = 0; i < 45; ++i){
    //std::cout<<"i: "<<i;
    if(m_doGrid){
      if(i==1||i==0){
	rhogrid->SetBinContent(i+1,GridRho(&pjs[i],0,0.8,GRIDR));
      }else if(i<23){
	rhogrid->SetBinContent(i+1,GridRho(&pjs[i],((float) i)*0.2 - 0.4,((float) i)*0.2 + 0.4,GRIDR));
      }
      //std::cout<<"i/g "<<((float) i)*0.2<<" "<<rhogrid->GetBinContent(i+1) << std::endl;
    }
    if(densities[i].size() == 0){rho->SetBinContent(i+1,0.); continue;}
    std::sort(densities[i].begin(),densities[i].end());
    //std::cout<<"dnesitysize/floor/ceil "<<densities[i].size()<<" "<<(int)floor(((float)densities[i].size()+1.)*.5)<<" "<<(int)ceil(((float)densities[i].size()-1.)*.5)<< " " << densities[i].at((int)floor(((float)densities[i].size()+1.)*.5))<< " " << densities[i].at((int)ceil(((float)densities[i].size()-1.)*.5))<<std::endl;
    rho->SetBinContent(i+1,.5*densities[i].at((int)floor(((float)densities[i].size()+1.)*.5))+.5*densities[i].at((int)ceil(((float)densities[i].size()-1.)*.5)));
    //std::cout<<"r"<<rho->GetBinContent(i+1);
  }

  //std::cout<<":"<<GetRho(3.7)<<" " << GetGridRho(3.7) << std::endl;
  //std::cout<<std::endl;
}

//returns pile-up subtraction scale factor
float TowerSlide::PUSF(float pt, float eta, float a){
  int bin = rho->FindBin(eta);
  float offset = a*rho->GetBinContent(bin);
  //if(offset >= pt) return TOWERAREA*GEV/pt;

  //std::cout<<"pt/eta/a/SF - "<<pt<<" "<<eta<<" "<<a<<" "<<(pt-offset)/pt<<std::endl;
  return (pt-offset)/pt;
}

float TowerSlide::GetRho(float eta){
  int bin = rho->FindBin(eta);
  //std::cout<<">"<< GetGridRho(3.7) << std::endl;
  return rho->GetBinContent(bin);
}

float TowerSlide::GetGridRho(float eta){
  int bin = rhogrid->FindBin(fabs(eta));
  //std::cout<<">>"<<GetRho(3.7) << std::endl;
  return rhogrid->GetBinContent(bin);
}

//returns pile-up subtraction scale factor
float TowerSlide::GridPUSF(float pt, float eta, float a){
  int bin = rhogrid->FindBin(fabs(eta));
  float offset = a*rhogrid->GetBinContent(bin);
  //if(offset >= pt) return TOWERAREA*GEV/pt;

  //std::cout<<"pt/eta/a/SF - "<<pt<<" "<<eta<<" "<<a<<" "<<(pt-offset)/pt<<std::endl;
  return (pt-offset)/pt;
}

//performs regular fastjet grid rho with a little hack to allow symmetric eta range.
float TowerSlide::GridRho(std::vector<fastjet::PseudoJet>* pjs, float etalow, float etahigh, float r){//,float& sigma
  fastjet::MyGridMedianBackgroundEstimator gmed = fastjet::MyGridMedianBackgroundEstimator(etalow,etahigh,r);
  gmed.set_particles(*pjs);
  //sigma = gmed.sigma();
  return gmed.rho();
}
