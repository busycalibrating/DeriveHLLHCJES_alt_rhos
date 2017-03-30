//Steve Alkire
//17.05.2016

#include <vector>
#include <math.h>
#include <fstream>

#include "DeriveHLLHCJES/CustomRho.h"

#define GEV .001
//typedef std::vector<fastjet::PseudoJet> JetV

ClassImp(CustomRho)

///////////////////////////////////////////////////////////////////////////ddobre::21.02.17-02.03.17
//
// Required algorithms for rho calculation
//
//////////

// Get median
////////////////////

float CustomRho :: median(std::vector<float> V){
  float median;
  int position;
  if (V.size() == 0) {
    median = -1.0;
  } else if (V.size() % 2 == 0) {
    position = V.size()/2;
    median = (V[position] + V[position - 1])/2;
  } else {
    position = V.size()/2;
    median = V[position];
  }
  return median;
}

///////////////////////////////////////////////////////////////////////////ddobre::21.02.17-22.02.17
//
// Global rho
//
//////////

float CustomRho :: GlobalDomain_rho(const std::vector<fastjet::PseudoJet>* jets, const std::vector<float> jetArea){
  std::vector<float> pt_area_ratio;

  //Create a vector of pT/area with a hardcoded eta cut ( |eta| < 2.0 )
  for (int i = 0; i < jets->size() ; ++i) {
    if(std::abs(jets->at(i).eta()) < 2.0 ) { pt_area_ratio.push_back( jets->at(i).pt()/jetArea[i] );}
  }

  //Sort said vector via std::sort and then get the median
  //Median of even sized arrays is the averae of two middle entries
  std::sort(pt_area_ratio.begin(), pt_area_ratio.end());
  float global_rho = median(pt_area_ratio);

  return global_rho;
}

///////////////////////////////////////////////////////////////////////////ddobre::06.03.17
//
// Rho Strip
//
//////////

float CustomRho :: StripDomain_rho(const std::vector<fastjet::PseudoJet>* jets, const std::vector<float> jetArea, double m_Strip_DeltaMax, double target_jet_rapidity, double target_jet_phi, double jet_R){
  std::vector<float> pt_area_ratio;

  //Create a vector of pT/area with kt jets within area of the target antikt jet
  for (int i = 0; i < jets->size(); ++i ) {
    if(std::abs(jets->at(i).rapidity() - target_jet_rapidity) < jet_R*m_Strip_DeltaMax) {
      pt_area_ratio.push_back( jets->at(i).pt()/jetArea[i] );
    }
  }

  std::sort(pt_area_ratio.begin(), pt_area_ratio.end());
  float strip_rho = median(pt_area_ratio);

  return strip_rho;
}

///////////////////////////////////////////////////////////////////////////ddobre::06.03.17
//
// Rho Circle
//
//////////

float CustomRho :: CircleDomain_rho(const std::vector<fastjet::PseudoJet>* jets, const std::vector<float> jetArea, double m_Circle_DeltaMax, double target_jet_rapidity, double target_jet_phi, double jet_R){
  std::vector<float> pt_area_ratio;

  //Create a vector of pT/area with kt jets within area of the target antikt jet
  for (int i = 0; i < jets->size(); ++i ) {
    if( sqrt( pow( jets->at(i).rapidity() - target_jet_rapidity, 2.0) + pow( jets->at(i).phi() - target_jet_phi, 2.0) ) < jet_R*m_Circle_DeltaMax) {
      pt_area_ratio.push_back( jets->at(i).pt()/jetArea[i] );
    }
  }

  std::sort(pt_area_ratio.begin(), pt_area_ratio.end());
  float circle_rho = median(pt_area_ratio);

  return circle_rho;
}

///////////////////////////////////////////////////////////////////////////ddobre::21.02.17
//
// Doughnut Circle
//
//////////

float CustomRho :: DoughnutDomain_rho(const std::vector<fastjet::PseudoJet>* jets, const std::vector<float> jetArea, double m_Doughnut_DeltaMin, double m_Doughnut_DeltaMax, double target_jet_rapidity, double target_jet_phi, double jet_R){
  std::vector<float> pt_area_ratio;

  //Create a vector of pT/area with kt jets within area of the target antikt jet
  for (int i = 0; i < jets->size(); ++i ) {
    double ith_jet_DeltaR = sqrt( pow( jets->at(i).rapidity() - target_jet_rapidity, 2.0) + pow( jets->at(i).phi() - target_jet_phi, 2.0) );
    if( ith_jet_DeltaR < jet_R*m_Doughnut_DeltaMax && ith_jet_DeltaR > jet_R*m_Doughnut_DeltaMin) {
      pt_area_ratio.push_back( jets->at(i).pt()/jetArea[i] );
    }
  }

  std::sort(pt_area_ratio.begin(), pt_area_ratio.end());
  float doughnut_rho = median(pt_area_ratio);

  return doughnut_rho;
}

//////////////////////////////////////////////////////////// STEVE'S CODE ////////////////////////////////////////////////////////////
// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV //


///////////////////////////////////////////////////////////////////////////
//
// Rho Tower Grid (using Tower Cells)
//
//////////

float CustomRho :: Grid(const xAOD::CaloTowerContainer* towers, float etalow, float etahigh, float r, float &sigma = defaultsigma){
  fastjet::MyGridMedianBackgroundEstimator gmbe1 = fastjet::MyGridMedianBackgroundEstimator(etalow,etahigh,r);

  std::vector<fastjet::PseudoJet> clusterPJs;

  for(int c = 0; c < towers->size(); ++c ){
    TLorentzVector tc;
    tc.SetPtEtaPhiE(towers->at(c)->pt(),towers->at(c)->eta(),towers->at(c)->phi(),towers->at(c)->e());
    if(tc.Pt() * GEV < 0 || tc.E() * GEV < 0) continue;
    fastjet::PseudoJet PJ(tc.Px()*GEV,tc.Py()*GEV,tc.Pz()*GEV,tc.E()*GEV);
    clusterPJs.push_back(PJ);
  }

  gmbe1.set_particles(clusterPJs);
  sigma = gmbe1.sigma();
  return gmbe1.rho();
}

///////////////////////////////////////////////////////////////////////////
//
// Rho Grid (using CaloCalTopoClusters)
//
//////////

float CustomRho :: Grid(const xAOD::CaloClusterContainer* clusters, float etalow, float etahigh, float r, float &sigma = defaultsigma){
  fastjet::MyGridMedianBackgroundEstimator gmbe1 = fastjet::MyGridMedianBackgroundEstimator(etalow,etahigh,r);

  std::vector<fastjet::PseudoJet> clusterPJs;

  for(int c = 0; c < clusters->size(); ++c ){
    if((clusters->at(c)->p4(CaloClusterState)).Pt() * GEV < 0 || (clusters->at(c)->p4(CaloClusterState)).E() * GEV < 0) continue;

    fastjet::PseudoJet PJ((clusters->at(c)->p4(CaloClusterState)).Px()*GEV,(clusters->at(c)->p4(CaloClusterState)).Py()*GEV,(clusters->at(c)->p4(CaloClusterState)).Pz()*GEV,(clusters->at(c)->p4(CaloClusterState)).E()*GEV);
    clusterPJs.push_back(PJ);
  }
  gmbe1.set_particles(clusterPJs);
  sigma = gmbe1.sigma();
  return gmbe1.rho();
}


