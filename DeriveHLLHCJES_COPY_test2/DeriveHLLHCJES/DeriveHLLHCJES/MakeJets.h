#ifndef DeriveHLLHCJES_MakeJets_H
#define DeriveHLLHCJES_MakeJets_H

#include <EventLoop/Algorithm.h>
#include "JetRec/JetRecTool.h"
#include "JetRec/PseudoJetGetter.h"
#include "JetRec/JetToolRunner.h"
#include "JetRec/PseudoJetGetter.h"
#include "xAODRootAccess/Init.h"

using namespace std;

class MakeJets : public EL::Algorithm
{
  string m_config;
  
 public:
  xAOD::TEvent *m_event;  //!
  xAOD::TStore *m_store;  //!
  
  ToolHandleArray<IPseudoJetGetter> hgets; //!
  ToolHandleArray<IJetExecuteTool> hrecs; //!
  //PseudoJetGetter* plcget; //!
  JetToolRunner * jrun; //!
  
public:
  // Tree *myTree; //!
  // TH1 *myHist; //!
  bool SkipNegativeEnergy; //!
  TString Label; //!
  float JetRadius; //!
  float PtMin; //!
  TString OutputContainer; //!
  TString m_PseudoJetGetter; //!
  TString ClusterOutputContainer; //!
  

  // this is a standard constructor
  MakeJets ();
  MakeJets (string config);

  // these are the functions inherited from Algorithm
  virtual EL::StatusCode setupJob (EL::Job& job);
  virtual EL::StatusCode fileExecute ();
  virtual EL::StatusCode histInitialize ();
  virtual EL::StatusCode changeInput (bool firstFile);
  virtual EL::StatusCode initialize ();
  virtual EL::StatusCode execute ();
  virtual EL::StatusCode postExecute ();
  virtual EL::StatusCode finalize ();
  virtual EL::StatusCode histFinalize ();

  // this is needed to distribute the algorithm to the workers
  ClassDef(MakeJets, 1);
};

#endif
