//Steve Alkire
//17.05.2016

#ifndef DeriveHLLHCJES__CustomRho_H
#define DeriveHLLHCJES__CustomRho_H

#include "util/MyClusterSequence.hh"
#include "util/MyJetMedianBackgroundEstimator.hh"
#include "DeriveHLLHCJES/MyGridMedianBackgroundEstimator.h"
#include "fastjet/ClusterSequenceArea.hh"

#include "fastjet/contrib/ConstituentSubtractor.hh"
#include "fastjet/tools/GridMedianBackgroundEstimator.hh"
#include "fastjet/tools/Subtractor.hh"

#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloTowerContainer.h"
#include "xAODCaloEvent/CaloTowerAuxContainer.h"

class CustomRho{
  xAOD::CaloCluster::State CaloClusterState;
  static float defaultsigma;
 public:

  // Sorting algorithms //
  int partition(std::vector<float>& A, int left, int right, int pivot);
  void quicksort(std::vector<float>& A, int left, int right);
  float median(std::vector<float> V);

  //rho by grid method in symmetric eta window
  float Grid(const xAOD::CaloTowerContainer* towers, float etalow, float etahigh, float r, float &sigma);
  float Grid(const xAOD::CaloClusterContainer* clusters, float etalow, float etahigh, float r, float &sigma);

  //rho by median jet method in various rapidity and phi regions
  float GlobalDomain_rho(const std::vector<fastjet::PseudoJet>* jets, const std::vector<float> jetArea);
  float StripDomain_rho(const std::vector<fastjet::PseudoJet>* jets, const std::vector<float> jetArea, double m_Strip_DeltaMax, double target_jet_rapidity, double target_jet_phi, double jet_R);
  float CircleDomain_rho(const std::vector<fastjet::PseudoJet>* jets, const std::vector<float> jetArea, double m_Circle_DeltaMax, double target_jet_rapidity, double target_jet_phi, double jet_R);
  float DoughnutDomain_rho(const std::vector<fastjet::PseudoJet>* jets, const std::vector<float> jetArea, double m_Doughnut_DeltaMin, double m_Doughnut_DeltaMax, double target_jet_rapidity, double target_jet_phi, double jet_R);


  CustomRho():CaloClusterState(xAOD::CaloCluster::UNCALIBRATED){}
  CustomRho(xAOD::CaloCluster::State state):CaloClusterState(state){}

  ~CustomRho(){}

  ClassDef(CustomRho, 1);
};

#endif

