//Steve Alkire 
//17.05.2016

#ifndef DeriveHLLHCJES__TowerSlide_H
#define DeriveHLLHCJES__TowerSlide_H

#ifdef ROOTCORE
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"
#include "xAODRootAccess/TActiveStore.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#endif

#include "xAODJet/JetContainer.h"

#include <vector>
#include <TTree.h>
#include <TEnv.h>
#include <EventLoop/Algorithm.h>

#include "util/MyClusterSequence.hh"
#include "util/MyJetMedianBackgroundEstimator.hh"
#include "DeriveHLLHCJES/MyGridMedianBackgroundEstimator.h"
#include "fastjet/ClusterSequenceArea.hh"

#include "fastjet/contrib/ConstituentSubtractor.hh"
//#include "fastjet/tools/GridMedianBackgroundEstimator.hh"
#include "fastjet/tools/Subtractor.hh"

#include "JetCalibTools/JetCalibrationTool.h"

#include "xAODCaloEvent/CaloTowerContainer.h"
#include "xAODCaloEvent/CaloTowerAuxContainer.h"

//TO DO: add const correctness
class TowerSlide{
  bool m_doGrid;

  float GridRho(std::vector<fastjet::PseudoJet>* pjs, float etalow, float etahigh, float r);

 public:
  TH1F* rho;
  TH1F* rhogrid;
  
 TowerSlide():m_doGrid(false){}
 TowerSlide(bool doGrid):m_doGrid(doGrid){}
  
  void OncePerEvent(const xAOD::CaloTowerContainer* towers);
  float PUSF(float pt, float eta, float a);
  float GridPUSF(float pt, float eta, float a);
  float GetRho(float eta);
  float GetGridRho(float eta);


  ~TowerSlide(){
    //if(rho) delete rho;
    //if(rhogrid) delete rhogrid;
  }
  
  ClassDef(TowerSlide, 1);
};

#endif
