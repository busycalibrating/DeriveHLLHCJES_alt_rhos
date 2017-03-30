// jetrec.cxx

// Demonstration program to build jets.

#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <TFile.h>
#include <TError.h>
#include <TString.h>
#include "AsgTools/SgTEvent.h"
#ifdef ROOTCORE
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"
#include "xAODRootAccess/TActiveStore.h"
#endif
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "JetRec/PseudoJetGetter.h"
#include "JetRec/JetFromPseudojet.h"
#include "JetRec/JetFinder.h"
#include "JetRec/JetSplitter.h"
#include "JetRec/JetRecTool.h"
#include "JetRec/JetDumper.h"
#include "JetRec/JetToolRunner.h"
#include "JetMomentTools/JetWidthTool.h"
#include "JetMomentTools/JetVertexFractionTool.h"
#include "JetMomentTools/JetVertexTaggerTool.h"
#include "JetRecTools/SimpleJetTrackSelectionTool.h"
#include "JetRecTools/TrackVertexAssociationTool.h"
#include "JetRecTools/TrackPseudoJetGetter.h"

using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::istringstream;
using asg::ToolStore;

typedef vector<string> NameList;

int main(int argc, char* argv[]) {

  // Set this flag to add the width moment to all jets.
  bool dowidth = true;

  // Set this flag to groom jets.
  bool groom = true;

  // Set this flag to calculate jet vertex fraction
  bool jvf=true;

  // Set this flag to recalculate jet vertex tag
  bool jvt = true;

  // The application's name:
  string appname = argv[0];
  string myname = appname + ": ";

  // Check command line:
  if ( argc < 2 ) {
    cout << "Usage: " << appname << " FILENAME" << endl;
    return 1;
  }

  // Presence of 3rd CL argument makes job verbose.
  int verbose = 0;
  if ( argc > 3 ) {
    istringstream ssarg(argv[3]);
    ssarg >> verbose;
  }

  // Fail job on unchecked status code.
  xAOD::TReturnCode::enableFailure();
  StatusCode::enableFailure();

  // Show event store access.
  //Broken in 2.0.13// if ( verbose ) asg::SgTEvent::setDebug(1);

  // Initialise the application:
  if ( xAOD::Init(appname.c_str()) );

  // Open the input file:
  string filename = argv[1];
  cout << myname << "Input file: " << filename << endl;
  TFile* pinfile = TFile::Open(filename.c_str(), "READ");
  //cout << pinfile->get() << endl;

  // Create a TEvent object:
  xAOD::TEvent eventstore(xAOD::TEvent::kClassAccess);
  if ( ! eventstore.readFrom(pinfile) ) {
    cout << myname << "Unable to retrieve events from input file." << endl;
    return 2;
  }
  xAOD::TStore tds;
  xAOD::TStore* ptds = xAOD::TActiveStore::store();
  if ( ptds == 0 ) {
    std::cerr << myname << "TStore not found." << std::endl;
    return 0;
  }

  // Open the output file.
  TFile* poutfile = TFile::Open("out.root", "RECREATE");
  if ( ! eventstore.writeTo(poutfile) ) {
    cout << myname << "Error attaching output file." << endl;
    return 3;
  }

  // Decide how many events to run over:
  Long64_t nevt = eventstore.getEntries();
  if ( argc > 2 ) {
    Long64_t maxevt = 0;
    istringstream ssmaxevt(argv[2]);
    ssmaxevt >> maxevt;
    if ( maxevt >= 0 && maxevt < nevt ) {
      cout << myname << "Event count: " << maxevt << "/" << nevt << endl;
      nevt = maxevt;
    }
  } else {
    cout << myname << "Event count: " << nevt << endl;
  }

  ToolHandleArray<IPseudoJetGetter> hgets;
  ToolHandleArray<IJetExecuteTool> hrecs;
  bool fail = false;

  // Create a PseudoJet builder.
  cout << "Creating pseudojet builder." << endl;
  PseudoJetGetter* plcget = new PseudoJetGetter("lcget");
  fail |= plcget->setProperty("InputContainer", "CaloCalTopoCluster").isFailure();
  fail |= plcget->setProperty("OutputContainer", "PseudoJetLCTopo").isFailure();
  fail |= plcget->setProperty("Label", "LCTopo").isFailure();
  fail |= plcget->setProperty("SkipNegativeEnergy", true).isFailure();
  fail |= plcget->setProperty("GhostScale", 0.0).isFailure();
  //ToolStore::put(plcget);
  ToolHandle<IPseudoJetGetter> hlcget(plcget);
  hgets.push_back(hlcget);
  if ( ToolStore::get("lcget") == 0 ) {
    cout << "Tool lcget is not in store!" << endl;
    //return 1;
  }

  // Create list of modifiers.
  ToolHandleArray<IJetModifier> mods;
  ToolHandleArray<IJetModifier> gmods;
  if ( dowidth ) {
    JetWidthTool* ptool = new JetWidthTool("width");
    ToolHandle<IJetModifier> htool(ptool);
    mods.push_back(htool);
    gmods.push_back(htool);
  }

  // Customize jet vertex fraction tool and config tool it needs
  // pointer to tools for JVT
  JetVertexFractionTool         *pjvftool    = 0; // calculates JVF
  JetVertexTaggerTool           *pjvttool    = 0; // calculates JVT
  TrackPseudoJetGetter          *pgtrackget  = 0; // gets track pointing to this jet
  TrackVertexAssociationTool    *ptvatool    = 0; // decides from which vertex is the track comming from
  SimpleJetTrackSelectionTool   *ptrkseltool = 0; // simple selection of tracks (pt cut)
  // names of containers and tools
  string trkcont       = "InDetTrackParticles";
  string vtxcont       = "PrimaryVertices";
  string assoctrcks    = "GhostTrack";
  string jetassoctrcks = string("PseudoJet")+assoctrcks;
  string seltrcks      = "myJetSelectedTracks";
  string tvassoc       = "myJetTrackVtxAssoc";
  string trkseltool    = "mytracksel";
  string jvfname       = "myJVF";
  string jvtname       = "Jvt"; // myJVT
  // tool handlers will will be registered in jet scheduler
  ToolHandle<IPseudoJetGetter>  gtrackget;
  ToolHandle<IJetExecuteTool>   tvatool;
  ToolHandle<IJetTrackSelector> trksel;
  ToolHandle<IJetModifier>      jvftool;
  ToolHandle<IJetModifier>      jvttool;
  if (jvf || jvt){
      // configure track-vertex association
      ptvatool = new TrackVertexAssociationTool("mytvassoc");
      if (verbose) ptvatool->msg().setLevel(MSG::VERBOSE);
      fail |= ptvatool->setProperty( "TrackParticleContainer"  , trkcont ).isFailure();
      fail |= ptvatool->setProperty( "TrackVertexAssociation"  , tvassoc ).isFailure();
      fail |= ptvatool->setProperty( "VertexContainer"         , vtxcont ).isFailure();
      fail |= ptvatool->setProperty( "MaxTransverseDistance"   , 1.5     ).isFailure();
      fail |= ptvatool->setProperty( "MaxLongitudinalDistance" , 1.0e7   ).isFailure();
      fail |= ptvatool->setProperty( "MaxZ0SinTheta"           , 1.5     ).isFailure();
      tvatool = ToolHandle<IJetExecuteTool>(ptvatool);
      hrecs.push_back(tvatool);

      // configure simple jet track selection
      ptrkseltool = new SimpleJetTrackSelectionTool(trkseltool);
      if (verbose) ptrkseltool->msg().setLevel(MSG::VERBOSE);
      fail |= ptrkseltool ->setProperty( "PtMin"           , 500.0    ).isFailure();
      fail |= ptrkseltool ->setProperty( "InputContainer"  , trkcont  ).isFailure();
      fail |= ptrkseltool ->setProperty( "OutputContainer" , seltrcks ).isFailure();
      trksel = ToolHandle<IJetTrackSelector>(ptrkseltool);
      // handler trksel you need for JVT tool
      // new handler will be register in jet scheduler
      ToolHandle<IJetExecuteTool> trksel_run = ToolHandle<IJetExecuteTool>(ptrkseltool);
      hrecs.push_back(trksel_run);
      // configure ghost track getter
      pgtrackget = new TrackPseudoJetGetter("gtrackget");
      if (verbose) ptrkseltool->msg().setLevel(MSG::VERBOSE);
      fail |= pgtrackget ->setProperty( "InputContainer"         , seltrcks      ).isFailure();
      fail |= pgtrackget ->setProperty( "Label"                  , assoctrcks    ).isFailure();
      fail |= pgtrackget ->setProperty( "OutputContainer"        , jetassoctrcks ).isFailure();
      fail |= pgtrackget ->setProperty( "TrackVertexAssociation" , tvassoc       ).isFailure();
      fail |= pgtrackget ->setProperty( "SkipNegativeEnergy"     , true          ).isFailure();
      fail |= pgtrackget ->setProperty( "GhostScale"             , 1e-20         ).isFailure();
      gtrackget = ToolHandle<IPseudoJetGetter>(pgtrackget);
      hgets.push_back(gtrackget); //< this not going to hrecs but to list of gettters
  }

  if (jvf) {
      // configure jet vertex fraction
      pjvftool = new JetVertexFractionTool(jvfname);
      if (verbose) pjvftool->msg().setLevel(MSG::VERBOSE);
      fail |= pjvftool->setProperty( "VertexContainer"        , vtxcont    ).isFailure();
      fail |= pjvftool->setProperty( "AssociatedTracks"       , assoctrcks ).isFailure();
      fail |= pjvftool->setProperty( "TrackVertexAssociation" , tvassoc    ).isFailure();
      fail |= pjvftool->setProperty( "TrackSelector"          , trksel     ).isFailure();
      fail |= pjvftool->setProperty( "JVFName"                , jvfname    ).isFailure();
      pjvftool->print();
      jvftool = ToolHandle<IJetModifier>(pjvftool);
      mods.push_back(jvftool);
      gmods.push_back(jvftool);
  }

  if(jvt) {
      // configure jet vertex tagger
      pjvttool = new JetVertexTaggerTool(jvtname);
      if (verbose) pjvttool->msg().setLevel(MSG::VERBOSE);
      fail |= pjvttool->setProperty( "VertexContainer"        , vtxcont    ).isFailure();
      fail |= pjvttool->setProperty( "AssociatedTracks"       , assoctrcks ).isFailure();
      fail |= pjvttool->setProperty( "TrackParticleContainer"  , trkcont ).isFailure();
      fail |= pjvttool->setProperty( "TrackVertexAssociation" , tvassoc    ).isFailure();
      fail |= pjvttool->setProperty( "TrackSelector"          , trksel     ).isFailure();
      fail |= pjvttool->setProperty( "JVTName"                , jvtname    ).isFailure();
      fail |= pjvttool->setProperty( "JVTFileName"            , "JetMomentTools/JVTlikelihood_20140805.root" ).isFailure();
      fail |= pjvttool->initialize().isFailure();
      pjvttool->print();
      jvttool = ToolHandle<IJetModifier>(pjvttool);
      mods.push_back(jvttool);
      gmods.push_back(jvttool);
  }

  // Create the jet reco tools.
  cout << "Creating jet builder." << endl;
  JetFromPseudojet* pbuild = new JetFromPseudojet("jetbuild");
  ToolHandle<IJetFromPseudojet> hbuild(pbuild);
  NameList jetbuildatts;
  jetbuildatts.push_back("ActiveArea");
  jetbuildatts.push_back("ActiveAreaFourVector");
  fail |= pbuild->setProperty("Attributes", jetbuildatts).isFailure();
  //ToolStore::put(pbuild);
  if ( verbose ) pbuild->msg().setLevel(MSG::DEBUG);
  if ( verbose  > 1) pbuild->msg().setLevel(MSG::VERBOSE);
  fail |= pbuild->initialize().isFailure();
  cout << "Creating jet finder." << endl;
  JetFinder* pfind = new JetFinder("jetfind");
  fail |= pfind->setProperty("JetAlgorithm", "AntiKt").isFailure();
  fail |= pfind->setProperty("JetRadius", 0.4).isFailure();
  fail |= pfind->setProperty("PtMin", 5000.0).isFailure();
  fail |= pfind->setProperty("GhostArea", 0.01).isFailure();
  fail |= pfind->setProperty("RandomOption", 1).isFailure();
  fail |= pfind->setProperty("JetBuilder", hbuild).isFailure();
  ToolHandle<IJetFinder> hfind(pfind);
  //ToolStore::put(pfind);
  fail |= pfind->initialize().isFailure();
  if ( verbose ) pfind->msg().setLevel(MSG::DEBUG);
  cout << "Creating jetrec tool." << endl;
  JetRecTool* pjrf = new JetRecTool("jrfind");
  fail |= pjrf->setProperty("OutputContainer", "MyAntiKt4LCTopoJets").isFailure();
  fail |= pjrf->setProperty("PseudoJetGetters", hgets).isFailure();
  fail |= pjrf->setProperty("JetFinder", hfind).isFailure();
  fail |= pjrf->setProperty("JetModifiers", mods).isFailure();
  fail |= pjrf->initialize().isFailure();
  if ( verbose ) pjrf->msg().setLevel(MSG::DEBUG);
  if ( verbose > 1 ) pjrf->msg().setLevel(MSG::VERBOSE);
  ToolHandle<IJetExecuteTool> hjrf(pjrf);
  hrecs.push_back(pjrf);
  if ( groom ) {
    cout << "Creating jet splitter." << endl;
    JetSplitter* psplit = new JetSplitter("jetsplit");
    fail |= psplit->setProperty("MuMax", 1.0).isFailure();
    fail |= psplit->setProperty("YMin", 0.4).isFailure();
    fail |= psplit->setProperty("RClus", 0.3).isFailure();
    fail |= psplit->setProperty("BDRS", false).isFailure();
    fail |= psplit->setProperty("NSubjetMax", 3).isFailure();
    fail |= psplit->setProperty("JetBuilder", hbuild).isFailure();
    //ToolStore::put(psplit);
    fail |= psplit->initialize().isFailure();
    ToolHandle<IJetGroomer> hsplit(psplit);
    if ( verbose ) psplit->msg().setLevel(MSG::DEBUG);
    if ( verbose > 1 ) psplit->msg().setLevel(MSG::VERBOSE);
    cout << "Creating jetrec tool for splitting." << endl;
    JetRecTool* pjrs = new JetRecTool("jrsplit");
    fail |= pjrs->setProperty("OutputContainer", "AntiKt4SplitY40Jets").isFailure();
    fail |= pjrs->setProperty("InputContainer", "MyAntiKt4LCTopoJets").isFailure();
    fail |= pjrs->setProperty("JetGroomer", hsplit).isFailure();
    fail |= pjrs->setProperty("JetModifiers", gmods).isFailure();
    fail |= pjrs->initialize().isFailure();
    if ( verbose ) pjrs->msg().setLevel(MSG::DEBUG);
    if ( verbose > 1 ) pjrs->msg().setLevel(MSG::VERBOSE);
    ToolHandle<IJetExecuteTool> hjrs(pjrs);
    hrecs.push_back(hjrs);
  }

  // Dump LC clusters.
  JetDumper* pdumplc = new JetDumper("dumplc");
  fail |= pdumplc->setProperty("ContainerName", "PseudoJetLCTopo").isFailure();
  ToolHandle<IJetExecuteTool> hdumplc(pdumplc);
  //ToolStore::put(pdumplc);
  hrecs.push_back(hdumplc);

  // Jet property lists for dumping.
  NameList fvmoms;
  fvmoms.push_back("jetP4()");
  fvmoms.push_back("JetConstitScaleMomentum");
  NameList fmoms;
  fmoms.push_back("ActiveArea");
  if ( dowidth ) fmoms.push_back("Width");
  NameList fgmoms = fmoms;
  fgmoms.push_back("DRFilt");
  fgmoms.push_back("YFilt");
  fgmoms.push_back("MuFilt");
  NameList apvmoms;
  apvmoms.push_back("LCTopo");
  apvmoms.push_back("constituentLinks");
  apvmoms.push_back("GhostTruth");
  apvmoms.push_back("GhostTrack");
  apvmoms.push_back("GhostMuonSegment");
  NameList flvmoms;
  NameList flvmoms_orig;
  if (jvf){
      flvmoms.push_back(jvfname);
      flvmoms_orig.push_back("JVF");
  }

  if (jvt) {
    // fmoms.push_back(jvtname);
    //    fmoms.push_back("Jvt");
    //    fmoms_orig.push_back("JVT");
  }

  // Dump old jets.
  JetDumper* pdmp_akt4 = new JetDumper("dump_akt4");
  cout << "Tool dump_akt4: " << ToolStore::get("dump_akt4") << endl;
  fail |= pdmp_akt4->setProperty("ContainerName", "AntiKt4LCTopoJets").isFailure();
  fail |= pdmp_akt4->setProperty("Detail", 2).isFailure();
  fail |= pdmp_akt4->setProperty("FourVectorMoments", fvmoms).isFailure();
  fail |= pdmp_akt4->setProperty("FloatMoments", fmoms).isFailure();
  fail |= pdmp_akt4->setProperty("FloatVectorMoments", flvmoms_orig).isFailure();
  fail |= pdmp_akt4->setProperty("AssociatedParticleVectors", apvmoms).isFailure();
  //ToolStore::put(pdmp_akt4);
  cout << "Tool dump_akt4: " << ToolStore::get("dump_akt4") << endl;
  ToolHandle<IJetExecuteTool> hdmp_akt4(pdmp_akt4);
  cout << "Tool dump_akt4: " << ToolStore::get("dump_akt4") << endl;
  hrecs.push_back(hdmp_akt4);

  // Dump new jets.
  JetDumper* pdumpjets = new JetDumper("dumpjets");
  fail |= pdumpjets->setProperty("ContainerName", "MyAntiKt4LCTopoJets").isFailure();
  fail |= pdumpjets->setProperty("Detail", 2).isFailure();
  fail |= pdumpjets->setProperty("FourVectorMoments", fvmoms).isFailure();
  fail |= pdumpjets->setProperty("FloatMoments", fmoms).isFailure();
  fail |= pdumpjets->setProperty("FloatVectorMoments", flvmoms).isFailure();
  fail |= pdumpjets->setProperty("AssociatedParticleVectors", apvmoms).isFailure();
  ToolHandle<IJetExecuteTool> hdumpjets(pdumpjets);
  //ToolStore::put(pdumpjets);
  hrecs.push_back(hdumpjets);

  // Dump groomed jets.
  if ( groom ) {
    JetDumper* pdumpgroomedjets = new JetDumper("dumpgroomedjets");
    fail |= pdumpgroomedjets->setProperty("ContainerName", "AntiKt4SplitY40Jets").isFailure();
    fail |= pdumpgroomedjets->setProperty("Detail", 2).isFailure();
    fail |= pdumpgroomedjets->setProperty("FourVectorMoments", fvmoms).isFailure();
    fail |= pdumpgroomedjets->setProperty("FloatMoments", fgmoms).isFailure();
    fail |= pdumpgroomedjets->setProperty("FloatVectorMoments", flvmoms).isFailure();
    fail |= pdumpgroomedjets->setProperty("AssociatedParticleVectors", apvmoms).isFailure();
    ToolHandle<IJetExecuteTool> hdumpgroomedjets(pdumpgroomedjets);
    //ToolStore::put(pdumpgroomedjets);
    hrecs.push_back(hdumpgroomedjets);
  }

  // Create the jet scheduler.
  cout << "Creating jet runner." << endl;
  JetToolRunner jrun("jetrunner");
  fail |= jrun.setProperty("Tools", hrecs).isFailure();
  if ( verbose ) jrun.msg().setLevel(MSG::DEBUG);

  cout << "Initializing tools." << endl;
  fail |= jrun.initialize().isFailure();
  jrun.print();

  if ( fail ) { 
    cout << myname << "Tool initialialization failed!" << endl; 
    return 4;
  }

  // Loop over the events:
  for (Long64_t ient=0; ient<nevt; ++ient) {

    // Tell the object which entry to look at:
    eventstore.getEntry(ient);

    // Print some event information for fun:
    const xAOD::EventInfo* pei = 0;
    if ( ! eventstore.retrieve(pei, "EventInfo") ) {
      cout << myname << "Unable to retrieve entry " << ient << endl;
      return 3;
    }
    cout << myname << "Run " << pei->runNumber()
         << ", Event " << pei->eventNumber()
         << " [" << ient+1 << "/" << nevt << "]" << endl;

    jrun.execute();
  }

  return 0;
}
