#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <DeriveHLLHCJES/MakeJets.h>


#include "AsgTools/AsgTool.h"

#include "JetRec/PseudoJetGetter.h"
#include "JetRec/JetFromPseudojet.h"
#include "JetRec/JetFinder.h"
#include "JetRec/JetSplitter.h"
#include "JetRec/JetRecTool.h"
#include "JetRec/JetDumper.h"
#include "JetRec/JetToolRunner.h"

#include "JetInterface/IJetModifier.h"
#include "xAODRootAccess/tools/Message.h"

#include <iostream>
#include "TSystem.h"
#include "TEnv.h"

#define EL_RETURN_CHECK( CONTEXT, EXP )         \
  do {                                          \
  if( ! EXP.isSuccess() ) {                     \
  Error( CONTEXT,                               \
    XAOD_MESSAGE( "Failed to execute: %s" ),    \
         #EXP );                                \
  return EL::StatusCode::FAILURE;               \
  }                                             \
  } while( false )

using namespace std;
using asg::ToolStore;

// this is needed to distribute the algorithm to the workers
ClassImp(MakeJets)



MakeJets :: MakeJets ()
{
}

MakeJets :: MakeJets (string config)
{
  m_config = config;
}



EL::StatusCode MakeJets :: setupJob (EL::Job& job)
{
  job.useXAOD ();
  xAOD::Init( "MakeJets" ).ignore(); // call before opening first file
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MakeJets :: histInitialize ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MakeJets :: fileExecute ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MakeJets :: changeInput (bool firstFile)
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MakeJets :: initialize ()
{
  Info("configure()", "Jet configuration read from : %s \n", m_config.c_str());
  m_config = gSystem->ExpandPathName( m_config.c_str() );
  TEnv* m_settings = new TEnv(m_config.c_str());
  if( !m_settings ) {
    Error("xAODAnalysis::initiaize()", "Failed to read config file!");
    Error("xAODAnalysis::initiaize()", "config name : %s",m_config.c_str());
  }

  SkipNegativeEnergy = m_settings->GetValue("SkipNegativeEnergy",1);
  Label = m_settings->GetValue("Label","LCTopo");
  JetRadius = m_settings->GetValue("JetRadius",0.4);
  PtMin = m_settings->GetValue("PtMin",5000.0);
  OutputContainer = m_settings->GetValue("OutputContainer","MyAntiKt4LCTopoJets");
  m_PseudoJetGetter = m_settings->GetValue("PseudoJetGetter","lcget");
  ClusterOutputContainer = m_settings->GetValue("ClusterOutputContainer","PseudoJetLCTopo");
  

  m_event = wk()->xaodEvent();
  m_store = wk()->xaodStore();
  
  PseudoJetGetter* plcget = new PseudoJetGetter((const char *) m_PseudoJetGetter);
  EL_RETURN_CHECK("initialize()", plcget->setProperty("InputContainer", "CaloCalTopoClusters") );
  EL_RETURN_CHECK("initialize()", plcget->setProperty("OutputContainer", (const char*) ClusterOutputContainer) );
  EL_RETURN_CHECK("initialize()", plcget->setProperty("Label", (const char *) Label) );
  EL_RETURN_CHECK("initialize()", plcget->setProperty("SkipNegativeEnergy", SkipNegativeEnergy) );
  EL_RETURN_CHECK("initialize()", plcget->setProperty("GhostScale", 0.0) );
  EL_RETURN_CHECK("initialize()", plcget->initialize() );
  ToolHandle<IPseudoJetGetter> hlcget(plcget);
  hgets.push_back(hlcget);
  if ( ToolStore::get("lcget") == 0 ) {
    cout << "Tool lcget is not in store!" << endl;
    //return 1;
  }
  //check jetrec.cxx for jvt and many more attributes to be added
  cout << "Creating jet builder." << endl;
  JetFromPseudojet* pbuild = new JetFromPseudojet("jetbuild");
  ToolHandle<IJetFromPseudojet> hbuild(pbuild);
  vector<string> jetbuildatts;
  jetbuildatts.push_back("ActiveArea");
  jetbuildatts.push_back("ActiveAreaFourVector");
  EL_RETURN_CHECK("initialize()", pbuild->setProperty("Attributes", jetbuildatts) );
  EL_RETURN_CHECK("initialize()", pbuild->initialize() );
  
  cout << "Creating jet finder." << endl;
  JetFinder* pfind = new JetFinder("jetfind");
  EL_RETURN_CHECK("initialize()", pfind->setProperty("JetAlgorithm", "AntiKt") );
  EL_RETURN_CHECK("initialize()", pfind->setProperty("JetRadius",JetRadius));
  EL_RETURN_CHECK("initialize()", pfind->setProperty("PtMin", PtMin));
  EL_RETURN_CHECK("initialize()", pfind->setProperty("GhostArea", 0.01));
  EL_RETURN_CHECK("initialize()", pfind->setProperty("RandomOption", 1));
  EL_RETURN_CHECK("initialize()", pfind->setProperty("JetBuilder", hbuild));
  //EL_RETURN_CHECK("initialize()", );
  ToolHandle<IJetFinder> hfind(pfind);
  //ToolStore::put(pfind);
  EL_RETURN_CHECK("initialize()",pfind->initialize() );
  cout << "Creating jetrec tool." << endl;
  JetRecTool* pjrf = new JetRecTool("jrfind");
  EL_RETURN_CHECK("initialize()", pjrf->setProperty("OutputContainer", (const char*) OutputContainer) );
  EL_RETURN_CHECK("initialize()", pjrf->setProperty("PseudoJetGetters", hgets));
  EL_RETURN_CHECK("initialize()", pjrf->setProperty("JetFinder", hfind));
  //EL_RETURN_CHECK("initialize()", pjrf->setProperty("JetModifiers", mods));
  EL_RETURN_CHECK("initialize()", pjrf->initialize());
  ToolHandle<IJetExecuteTool> hjrf(pjrf);
  hrecs.push_back(pjrf);
  vector<string> fvmoms;
  fvmoms.push_back("jetP4()");
  fvmoms.push_back("JetConstitScaleMomentum");
  vector<string> fmoms;
  fmoms.push_back("ActiveArea");

  //JetDumper* pdmp_akt4 = new JetDumper("dump_akt4");
  //cout << "Tool dump_akt4: " << ToolStore::get("dump_akt4") << endl;
  //EL_RETURN_CHECK("initialize()", pdmp_akt4->setProperty("ContainerName", "AntiKt4LCTopoJets") );
  //EL_RETURN_CHECK("initialize()", pdmp_akt4->setProperty("Detail", 2));
  //EL_RETURN_CHECK("initialize()", pdmp_akt4->setProperty("FourVectorMoments", fvmoms));
  //EL_RETURN_CHECK("initialize()", pdmp_akt4->setProperty("FloatMoments", fmoms));
  //EL_RETURN_CHECK("initialize()", pdmp_akt4->setProperty("FloatVectorMoments", flvmoms_orig));
  //EL_RETURN_CHECK("initialize()", pdmp_akt4->setProperty("AssociatedParticleVectors", apvmoms));
    
  //    JetDumper* pdumpjets = new JetDumper("dumpjets");
  //EL_RETURN_CHECK("initialize()", pdumpjets->setProperty("ContainerName", "MyAntiKt4LCTopoJets"));
  //EL_RETURN_CHECK("initialize()", pdumpjets->setProperty("Detail", 2));
  //EL_RETURN_CHECK("initialize()", pdumpjets->setProperty("FourVectorMoments", fvmoms));
  //EL_RETURN_CHECK("initialize()", pdumpjets->setProperty("FloatMoments", fmoms));
  //EL_RETURN_CHECK("initialize()", pdumpjets->setProperty("FloatVectorMoments", flvmoms));
  //EL_RETURN_CHECK("initialize()", pdumpjets->setProperty("AssociatedParticleVectors", apvmoms));
    
  //ToolHandle<IJetExecuteTool> hdumpjets(pdumpjets);
  //ToolStore::put(pdumpjets);
  //hrecs.push_back(hdumpjets);
    
  jrun = new JetToolRunner("jetrunner");
  EL_RETURN_CHECK("initialize()", jrun->setProperty("Tools", hrecs) );
  cout << "Initializing tools." << endl;
  EL_RETURN_CHECK("initialize()", jrun->initialize() );
  jrun->print();
    
  /////////////


  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MakeJets :: execute ()
{
  jrun->execute();
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MakeJets :: postExecute ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MakeJets :: finalize ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode MakeJets :: histFinalize ()
{
  return EL::StatusCode::SUCCESS;
}
