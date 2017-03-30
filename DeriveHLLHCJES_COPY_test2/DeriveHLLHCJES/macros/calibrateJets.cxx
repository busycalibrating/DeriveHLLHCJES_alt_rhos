#include "CalibrationTools.h"
#include <typeinfo>

//void print_vec(const std::vector<int>& v){
//  for(unsigned i = 0; i< v.size(); ++i) {
//    std::cout << v[i] << std::endl;
//  }
//}


void calibrateJets(const char* tag = "C0_", const char* singleRtag = "", const char* customMain = "", const char* ver=""){
  TString downloadC("Download.config"); //used for organizing the local work
  TString localC("Local.config"); //used for custom tree variables

  TString customMainS(customMain);
  TString mainC("");
  if(customMainS.EqualTo("")) mainC="Main.config"; //used as a template (to do--use to force a calibration)
  else mainC = customMain;

  TEnv* downloadE = new TEnv(downloadC);
  std::vector<TString> Rtags;
  TString singleRtagS(singleRtag);
  if(singleRtagS==""){
    Rtags = Vectorize(downloadE->GetValue("Rtags","")); // debugging // Looks like it's getting rtag from downloads. This isn't the same as the push backed rtags from tonsofresiduals
  }else{
    Rtags.push_back(singleRtagS);
  }

  TEnv* localE = new TEnv(localC); //directing the local calibration

  TEnv* mainE = new TEnv(mainC);
  TString jesS = mainE->GetValue("AbsoluteJES.CalibFile","JES.config");
  TEnv* jesE = new TEnv(jesS);
  //--Control Output
  bool doUncalibrated = localE->GetValue("doUncalibrated",0);
  bool doArea = localE->GetValue("doArea",0);
  bool doAreaResidual = localE->GetValue("doAreaResidual",0);
  bool doAreaResidualConst = localE->GetValue("doAreaResidualConst",0);
  bool doAreaResidualJES = localE->GetValue("doAreaResidualJES",0);
  bool useOriginCorrection = localE->GetValue("useOriginCorrection",1);

  TString verS(ver);
  TString Version = downloadE->GetValue("Version","VERSION");


  TString OutputFolder = localE->GetValue("OutputFolder","/tmp/alkire/CHOOSEANOUTPUTFOLDER");
  if(verS!=""){ Version = verS; OutputFolder = "/tmp/alkire/"+Version;}
  system(TString("mkdir "+OutputFolder+"/calibrated").Data());

  //--Calibration
  std::vector<TString> JetAlgos = Vectorize(localE->GetValue("JetAlgos","AntiKt4LCTopoJets AntiKt4EMTopoJets"));

  localE->Print();

  TString tagS = TString(tag);

  std::cout << "-------------------- CHECK 1: Got initial parameters." << std::endl;

  ///////////////////////////////////////////////////////////////////////////
  //
  // ---- LOOP 1: over rtags
  //        rtag loop over files - loops over the one rtag that I have?
  //
  ////////////////////

  for(int r = 0; r < Rtags.size(); ++r){

    //////////////////////////////////////////////////ddobre::11.03.17
    // debugging {
    std::cout<<""<<std::endl;
    std::cout<<"===================================================================================================="<<std::endl;
    std::cout<<"===== 1 ===== Loop 1 [Rtags] :: "<<Rtags.at(r)<<std::endl;
    std::cout<<"===================================================================================================="<<std::endl;
    std::cout<<""<<std::endl;
    ///////////////////////// }

    TFile * OutputFileF = new TFile(OutputFolder+"/calibrated/"+tagS+Rtags.at(r)+".root","recreate"); //Creates the output file

    double DefaultMuRef = mainE->GetValue("OffsetCorrection.DefaultMuRef", 0.);
    double DefaultNPVRef = mainE->GetValue("OffsetCorrection.DefaultNPVRef", 0.);
    double MuScaleFactor = mainE->GetValue("MuScaleFactor", 1.0);

    TString ResidualOffsetFileName(Version+Rtags.at(r));
    std::cout<<"Opening: "<<OutputFolder+"/residual/"+ResidualOffsetFileName+".config"<<std::endl; // What is this opening for?? This doesn't exist
    TEnv* residualE = new TEnv(OutputFolder+"/residual/"+ResidualOffsetFileName+".config");

    std::cout<<"residualE->print {"<<std::endl; //debugging::ddobre-11.03.17
    residualE->Print();
    std::cout<<"residualE->print }"<<std::endl; //debugging::ddobre-11.03.17
    //////

    TString InputFile = OutputFolder+"/"+Rtags.at(r)+".root"; //localE->GetValue("InputFile","CHOOSE AN INPUT FILE");
    TFile *InputFileF = new TFile(InputFile); //open the the file obtained from step 1 (calibration2016)

    ////////////////////
    // Define Calo jets
    TString CaloPt = localE->GetValue("CaloPt","");
    std::vector<float>* pt;
    TString CaloEta = localE->GetValue("CaloEta","");
    std::vector<float>* eta;
    TString CaloPhi = localE->GetValue("CaloPhi","");
    std::vector<float>* phi;
    TString CaloE = localE->GetValue("CaloE","");
    std::vector<float>*e;
    TString CaloM = localE->GetValue("CaloM","");
    std::vector<float>* m;

    ////////////////////
    // Define True jets
    TString TruePt = localE->GetValue("TruePt","");
    std::vector<float>* tpt;
    TString TrueEta = localE->GetValue("TrueEta","");
    std::vector<float>* teta;
    TString TruePhi = localE->GetValue("TruePhi","");
    std::vector<float>* tphi;
    TString TrueE = localE->GetValue("TrueE","");
    std::vector<float>* te;
    TString TrueM = localE->GetValue("TrueM","");
    std::vector<float>* tm;

    ////////////////////
    // Define Origin jets
    TString OrigPt = localE->GetValue("OrigPt","");
    std::vector<float>* opt;
    TString OrigEta = localE->GetValue("OrigEta","");
    std::vector<float>* oeta;
    TString OrigPhi = localE->GetValue("OrigPhi","");
    std::vector<float>* ophi;
    TString OrigE = localE->GetValue("OrigE","");
    std::vector<float>* oe;
    TString OrigM = localE->GetValue("OrigM","");
    std::vector<float>* om;

    TString CaloA = localE->GetValue("CaloA","");
    std::vector<float>* a;
    TString Mu = localE->GetValue("Mu","");
    float mu;
    TString NPV = localE->GetValue("NPV","");
    float npv;

    ////////////////////
    // Make vector of desired rhos
    std::vector<TString> Rhos = Vectorize(localE->GetValue("Rho","RHO"));
    std::vector<TString> Tags = Vectorize(localE->GetValue("Tag","NONE"));
    std::vector<TString> JetRhos = Vectorize(localE->GetValue("JetRho",""));
    std::vector<TString> JetTags = Vectorize(localE->GetValue("JetTag",""));
    ReplaceNONEs(Rhos);
    ReplaceNONEs(Tags);

    //float rho;
    TString EventWeight = localE->GetValue("EventWeight","EW");
    float w;

    Long64_t Nevents = localE->GetValue("Nevents",0);
    Long64_t Offset = localE->GetValue("Offset",0);

    //OutputFileF->cd();

    ///////////////////////////////////////////////////////////////////////////
    //
    // ---- LOOP 2: OVER THE ALGORITHMS IN Local.config
    //        Only have the one algorithm: AntiKT4EMTopoJets
    //
    ////////////////////

    for(int t = 0; t < JetAlgos.size(); ++t){

      //////////////////////////////////////////////////ddobre::11.03.17
      // debugging {
      std::cout<<""<<std::endl;
      std::cout<<"    ================================================================================================"<<std::endl;
      std::cout<<"    ====== 2 ===== Loop 2 [JetAlgos] :: "<<JetAlgos.at(t)<<std::endl;
      std::cout<<"    ================================================================================================"<<std::endl;
      std::cout<<""<<std::endl;
      ///////////////////////// }

      TTree * tree = (TTree*)InputFileF->Get(JetAlgos.at(t));
      tree->GetListOfBranches()->Print();
      Long64_t Nentry = Nevents > 0 ? Nevents : tree->GetEntries();

      if(CaloPt!=""){pt=0; tree->SetBranchAddress(CaloPt,&pt);}
      if(CaloEta!=""){eta=0; tree->SetBranchAddress(CaloEta,&eta);}
      if(CaloPhi!=""){phi=0; tree->SetBranchAddress(CaloPhi,&phi);}
      if(CaloE!=""){e=0; tree->SetBranchAddress(CaloE,&e);}
      if(CaloM!=""){m=0; tree->SetBranchAddress(CaloM,&m);}
      if(CaloA!=""){a=0; tree->SetBranchAddress(CaloA,&a);}

      if(TruePt!=""){tpt=0; tree->SetBranchAddress(TruePt,&tpt);}
      if(TrueEta!=""){teta=0; tree->SetBranchAddress(TrueEta,&teta);}
      if(TruePhi!=""){tphi=0; tree->SetBranchAddress(TruePhi,&tphi);}
      if(TrueE!=""){te=0; tree->SetBranchAddress(TrueE,&te);}
      if(TrueM!=""){tm=0; tree->SetBranchAddress(TrueM,&tm);}

      if(Mu!="") tree->SetBranchAddress(Mu,&mu);
      if(NPV!="") tree->SetBranchAddress(NPV,&npv);
      //if(Rho!="") tree->SetBranchAddress(Rho,&rho);
      if(EventWeight!="") tree->SetBranchAddress(EventWeight,&w);

      if(OrigPt!=""){opt=0; tree->SetBranchAddress(OrigPt,&opt);}
      if(OrigEta!=""){oeta=0; tree->SetBranchAddress(OrigEta,&oeta);}
      if(OrigPhi!=""){ophi=0; tree->SetBranchAddress(OrigPhi,&ophi);}
      if(OrigE!=""){oe=0; tree->SetBranchAddress(OrigE,&oe);}
      if(OrigM!=""){om=0; tree->SetBranchAddress(OrigM,&om);}

      OutputFileF->cd();
      //TTree* UncalibratedT, AreaT, AreaResidualT, AreaResidualJEST;


      TTree* event = new TTree("Event"+JetAlgos.at(t),"Event variables for "+JetAlgos.at(t));
      event->Branch("mu",&mu,"mu/F");
      event->Branch("npv",&npv,"npv/F");
      //event->Branch("rho",&rho,"rho/F");
      event->Branch("w",&w,"w/F");


      LoTree* UncalibratedL = 0;
      std::vector<LoTree*> AreaL;
      std::vector<LoTree*> AreaResidualL;
      std::vector<LoTree*> AreaResidualConstL;
      std::vector<LoTree*> AreaResidualJESL;
      if(doUncalibrated) UncalibratedL = new LoTree(new TTree(JetAlgos.at(t)+"Uncalibrated",JetAlgos.at(t)+"Uncalibrated"));



      float rho[Rhos.size()];
      std::vector<float>* jetrho[JetRhos.size()];
      for(int i = 0 ; i < JetRhos.size(); ++i) jetrho[i] = 0;
      //string s = typeid(jetrho).name()
      //std::cout << s << std::endl;
      //for(int i = 0 ; i < jetrho->size() ; ++i) std::cout << "--------------- jetrho array: " << jetrho->at(i) << std::endl;

      std::vector<TAxis *> AbsEtaBinsAxis;
      std::vector<TAxis *> ConstTermAbsEtaBinsAxis;
      std::vector< std::vector<double> > NPVTerm;
      std::vector< std::vector<double> > MuTerm;
      std::vector< std::vector<double> > ConstTerm;

      ///////////////////////////////////////////////////////////////////////////
      //
      // ---- LOOP 2.1: Looping over the global Rhos size
      //        Line 112. Comes from Local.config; RHOS = rho in there
      //
      ////////////////////

      for(int i = 0; i < Rhos.size(); ++i){



        if(doArea) AreaL.push_back(new LoTree(new TTree(JetAlgos.at(t)+Tags.at(i)+"Area",JetAlgos.at(t)+Tags.at(i)+"Area")));
	      if(doAreaResidual) AreaResidualL.push_back(new LoTree(new TTree(JetAlgos.at(t)+Tags.at(i)+"AreaResidual",JetAlgos.at(t)+Tags.at(i)+"AreaResidual")));
	      if(doAreaResidualConst) AreaResidualConstL.push_back(new LoTree(new TTree(JetAlgos.at(t)+Tags.at(i)+"AreaResidualConst",JetAlgos.at(t)+Tags.at(i)+"AreaResidualConst")));
	      if(doAreaResidualJES) AreaResidualJESL.push_back(new LoTree(new TTree(JetAlgos.at(t)+Tags.at(i)+"AreaResidualJES",JetAlgos.at(t)+Tags.at(i)+"AreaResidualJES")));
	      if(Rhos.at(i)!="0") {
	        tree->SetBranchAddress(Rhos.at(i),&rho[i]);
	        event->Branch(Rhos.at(i),&rho[i],Rhos.at(i)+"/F");
	      }
	      else rho[i] = 0;

	      TString ResidualOffsetName(Tags.at(i)+Version+Rtags.at(r)+JetAlgos.at(t));
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////// currently here
	      std::vector<double> AbsEtaBins = VectorizeD(residualE->GetValue(ResidualOffsetName+".AbsEtaBins","0 4.5")); // Getting a fuck load of 0s ????
	      AbsEtaBinsAxis.push_back(new TAxis(AbsEtaBins.size()-1,&AbsEtaBins[0]));

	      std::vector<double> ConstTermAbsEtaBins = VectorizeD(residualE->GetValue(ResidualOffsetName+".ConstTermAbsEtaBins","0 4.5"));
	      ConstTermAbsEtaBinsAxis.push_back(new TAxis(ConstTermAbsEtaBins.size()-1,&ConstTermAbsEtaBins[0]));
	      NPVTerm.push_back( VectorizeD(residualE->GetValue(ResidualOffsetName+".NPVTerm."+JetAlgos.at(t),"0 0 0 0 0")));
	      MuTerm.push_back( VectorizeD(residualE->GetValue(ResidualOffsetName+".MuTerm."+JetAlgos.at(t),"0 0 0 0 0")));
	      ConstTerm.push_back( VectorizeD(residualE->GetValue(ResidualOffsetName+".ConstTerm."+JetAlgos.at(t),"0 0 0 0 0")));

        //////////////////////////////////////////////////ddobre::11.03.17
        // debugging {
        std::cout<<""<<std::endl;
        std::cout<<"        ============================================================================================"<<std::endl;
        std::cout<<"        ===== 3 ===== Loop 2.1 [Global rhos] :: "<<Rhos.at(i)<<std::endl;
        std::cout<<"        ============================================================================================"<<std::endl;
        std::cout<<""<<std::endl;
        std::cout<<"        ResidualOffsetName: "<<ResidualOffsetName<<std::endl;
        std::cout<<"        Rho:                "<<rho[i]<<std::endl;
        std::cout<<"        Printing contents of vectors: "<<std::endl;


        std::cout<<"          ConstTermAbsEtaBins"<<std::endl;
        for(int counter= 0; counter< ConstTermAbsEtaBins.size(); ++counter){
          std::cout << ConstTermAbsEtaBins[counter] << std::endl;
        }

        std::cout<<"          NPVTerm:"<<std::endl;
        for(int counter= 0; counter< NPVTerm[i].size(); ++counter){
          std::cout << NPVTerm[i][counter] << std::endl;
        }

        std::cout<<"          MuTerm:"<<std::endl;
        for(int counter= 0; counter< MuTerm[i].size(); ++counter){
          std::cout << MuTerm[i][counter] << std::endl;
        }

        std::cout<<"          ConstTerm:"<<std::endl;
        for(int counter= 0; counter< ConstTerm[i].size(); ++counter){
          std::cout << ConstTerm[i][counter] << std::endl;
        }

        ///////////////////////// }

      }

      //

      std::vector<TAxis *> jetAbsEtaBinsAxis;
      std::vector<TAxis *> jetConstTermAbsEtaBinsAxis;
      std::vector< std::vector<double> > jetNPVTerm;
      std::vector< std::vector<double> > jetMuTerm;
      std::vector< std::vector<double> > jetConstTerm;


      ///////////////////////////////////////////////////////////////////////////
      //
      // ---- LOOP 2.2: Looping over the local rhos size
      //
      ////////////////////

      for(int i = 0; i < JetRhos.size(); ++i){


	      if(doArea) AreaL.push_back(new LoTree(new TTree(JetAlgos.at(t)+JetTags.at(i)+"Area",JetAlgos.at(t)+JetTags.at(i)+"Area")));
	      if(doAreaResidual) AreaResidualL.push_back(new LoTree(new TTree(JetAlgos.at(t)+JetTags.at(i)+"AreaResidual",JetAlgos.at(t)+JetTags.at(i)+"AreaResidual")));
	      if(doAreaResidualConst) AreaResidualConstL.push_back(new LoTree(new TTree(JetAlgos.at(t)+JetTags.at(i)+"AreaResidualConst",JetAlgos.at(t)+JetTags.at(i)+"AreaResidualConst")));
	      if(doAreaResidualJES) AreaResidualJESL.push_back(new LoTree(new TTree(JetAlgos.at(t)+JetTags.at(i)+"AreaResidualJES",JetAlgos.at(t)+JetTags.at(i)+"AreaResidualJES")));

	      tree->SetBranchAddress(JetRhos.at(i),&jetrho[i]);

        //std::cout<< jetrho[0] <<std::endl;

	      TString ResidualOffsetName(JetTags.at(i)+Version+Rtags.at(r)+JetAlgos.at(t));

        std::vector<double> AbsEtaBins = VectorizeD(residualE->GetValue(ResidualOffsetName+".AbsEtaBins","0 4.5"));
	      jetAbsEtaBinsAxis.push_back(new TAxis(AbsEtaBins.size()-1,&AbsEtaBins[0]));

	      std::vector<double> ConstTermAbsEtaBins = VectorizeD(residualE->GetValue(ResidualOffsetName+".ConstTermAbsEtaBins","0 4.5"));
	      jetConstTermAbsEtaBinsAxis.push_back(new TAxis(ConstTermAbsEtaBins.size()-1,&ConstTermAbsEtaBins[0]));
	      jetNPVTerm.push_back( VectorizeD(residualE->GetValue(ResidualOffsetName+".NPVTerm."+JetAlgos.at(t),"0 0 0 0 0")));
	      jetMuTerm.push_back( VectorizeD(residualE->GetValue(ResidualOffsetName+".MuTerm."+JetAlgos.at(t),"0 0 0 0 0")));
	      jetConstTerm.push_back( VectorizeD(residualE->GetValue(ResidualOffsetName+".ConstTerm."+JetAlgos.at(t),"0 0 0 0 0")));

        //////////////////////////////////////////////////ddobre::11.03.17
        // debugging {
        std::cout<<""<<std::endl;
        std::cout<<"        ============================================================================================"<<std::endl;
        std::cout<<"        ===== 3 ===== Loop 2.2 [Local rhos] :: "<<JetRhos.at(i)<<std::endl;
        std::cout<<"        ============================================================================================"<<std::endl;
        std::cout<<""<<std::endl;
        std::cout<<"        ResidualOffsetName: "<<ResidualOffsetName<<std::endl;

        std::cout<<"        Rho:                "<<jetrho[i]<<std::endl;
        std::cout<<"        Printing contents of vectors: "<<std::endl;

        std::cout<<"          jetNPVTerm:"<<std::endl;
        for(int counter = 0; counter < jetNPVTerm[i].size(); ++counter){
          std::cout << jetNPVTerm[i][counter] << std::endl;
        }

        std::cout<<"          jetMuTerm:"<<std::endl;
        for(int counter = 0; counter < jetMuTerm[i].size(); ++counter){
          std::cout << jetMuTerm[i][counter] << std::endl;
        }

        std::cout<<"          jetConstTerm:"<<std::endl;
        for(int counter = 0; counter < jetConstTerm[i].size(); ++counter){
          std::cout << jetConstTerm[i][counter] << std::endl;
        }

        ///////////////////////// }

      }
      //

      ///////////////////////////////////////////////////////////////////////////
      //
      // ---- LOOP 3: Looping over ALL EVENTS
      //
      ////////////////////

      //////////////////////////////////////////////////ddobre::11.03.17
      // debugging {
      std::cout<<""<<std::endl;
      std::cout<<"    ================================================================================================"<<std::endl;
      std::cout<<"    ===== 2 ===== Loop 3 [Event loop]" <<std::endl;
      std::cout<<"    ================================================================================================"<<std::endl;
      std::cout<<""<<std::endl;
      ///////////////////////// }

      for(Long64_t i = 0; i < Nentry; ++i){

	      tree->GetEntry(i+Offset);

        ///////////////////////////////////////////////////////////////////////////
        //
        // ---- LOOP 4: assuming e = energy; defined line 102
        //
        ////////////////////

        for(int j = 0; j < e->size(); ++j){ // INDEX: j = e size
	        TLorentzVector jet_calo;
	        TLorentzVector jet_orig;
	        TLorentzVector jet_area;
	        TLorentzVector jet_residual;
	        TLorentzVector jet_JES;
	        jet_calo.SetPtEtaPhiE(pt->at(j),eta->at(j),phi->at(j),e->at(j));
	        if(OrigE!="") {
            jet_orig.SetPtEtaPhiE(opt->at(j),oeta->at(j),ophi->at(j),oe->at(j));
            // std::cout<<"origin pt: "<<opt->at(j)<<" origin eta: "<< oeta->at(j)<<" origin phi: "<<ophi->at(j)<<" origin energy: "<<oe->at(j)<<std::endl; //debugging::12.03.2017
          }


          // Shove in the uncalibrated contents of the jets
	        if(doUncalibrated){
	          if(CaloPt!="") UncalibratedL->pt.push_back(pt->at(j));
	          if(CaloEta!="") UncalibratedL->eta.push_back(eta->at(j));
	          if(CaloPhi!="") UncalibratedL->phi.push_back(phi->at(j));
	          if(CaloE!="") UncalibratedL->e.push_back(e->at(j));
	          if(CaloM!="") UncalibratedL->m.push_back(m->at(j));

	          if(TruePt!="") UncalibratedL->tpt.push_back(tpt->at(j));
	          if(TrueEta!="") UncalibratedL->teta.push_back(teta->at(j));
	          if(TruePhi!="") UncalibratedL->tphi.push_back(tphi->at(j));
	          if(TrueE!="") UncalibratedL->te.push_back(te->at(j));
	          if(TrueM!="") UncalibratedL->tm.push_back(tm->at(j));

	          if(EventWeight!="") UncalibratedL->EW = w;
	        }


          ///////////////////////////////////////////////////////////////////////////
          //
          // ----- LOOP 5.1: global rhos
          //
          //////////

// JetPileupCorrection(float &pt, float &eta, float &phi, float &e, float &m, TLorentzVector &jet_calo, TLorentzVector &jet_origin, float AREA, float RHO, float offsetET, bool true){

	        for(int r = 0; r < Rhos.size(); ++r){ // INDEX: r = Rhos size

	          if(doArea){//add rho
	            float area_pt, area_eta, area_phi, area_e, area_m;

              JetPileupCorrection(area_pt,area_eta,area_phi,area_e,area_m,jet_calo,useOriginCorrection ? jet_orig : jet_calo,a->at(j),rho[r],0.,true);
	            AreaL.at(r)->pt.push_back(area_pt);
	            AreaL.at(r)->eta.push_back(area_eta);
	            AreaL.at(r)->phi.push_back(area_phi);
	            AreaL.at(r)->e.push_back(area_e);
	            AreaL.at(r)->m.push_back(jet_area.M());

	            if(TruePt!="") AreaL.at(r)->tpt.push_back(tpt->at(j));
	            if(TrueEta!="") AreaL.at(r)->teta.push_back(teta->at(j));
	            if(TruePhi!="") AreaL.at(r)->tphi.push_back(tphi->at(j));
	            if(TrueE!="") AreaL.at(r)->te.push_back(te->at(j));
	            if(TrueM!="") AreaL.at(r)->tm.push_back(tm->at(j));
	            if(EventWeight!="") AreaL.at(r)->EW = w;
	          }

            //////////////////////////////////////////////////
            // Do the disgnated corrections
            //  - The complete pile up corrections are given
            //    by the AreaResidualConst term

	          if(doAreaResidual){//add rho
	            float area_pt, area_eta, area_phi, area_e, area_m;
	            float residualoffset = GetResidualOffset(useOriginCorrection ? fabs(jet_orig.Eta()) : fabs(jet_calo.Eta()),mu,npv,DefaultMuRef,DefaultNPVRef,MuTerm.at(r),NPVTerm.at(r),AbsEtaBinsAxis.at(r));
	            JetPileupCorrection(area_pt,area_eta,area_phi,area_e,area_m,jet_calo,useOriginCorrection ? jet_orig : jet_calo,a->at(j),rho[r],residualoffset,true);
	            AreaResidualL.at(r)->pt.push_back(area_pt);
	            AreaResidualL.at(r)->eta.push_back(area_eta);
	            AreaResidualL.at(r)->phi.push_back(area_phi);
	            AreaResidualL.at(r)->e.push_back(area_e);
	            AreaResidualL.at(r)->m.push_back(jet_area.M());

	            if(TruePt!="") AreaResidualL.at(r)->tpt.push_back(tpt->at(j));
	            if(TrueEta!="") AreaResidualL.at(r)->teta.push_back(teta->at(j));
	            if(TruePhi!="") AreaResidualL.at(r)->tphi.push_back(tphi->at(j));
	            if(TrueE!="") AreaResidualL.at(r)->te.push_back(te->at(j));
	            if(TrueM!="") AreaResidualL.at(r)->tm.push_back(tm->at(j));

	            if(EventWeight!="") AreaResidualL.at(r)->EW = w;
	          }
	          if(doAreaResidualConst){//add rho
	            float area_pt, area_eta, area_phi, area_e, area_m;
	            float residualoffset = GetResidualOffset(useOriginCorrection ? fabs(jet_orig.Eta()) : fabs(jet_calo.Eta()),mu,npv,DefaultMuRef,DefaultNPVRef,MuTerm.at(r),NPVTerm.at(r),AbsEtaBinsAxis.at(r));
	            float constoffset = GetConstOffset(useOriginCorrection ? fabs(jet_orig.Eta()) : fabs(jet_calo.Eta()),ConstTerm.at(r),ConstTermAbsEtaBinsAxis.at(r));



              JetPileupCorrection(area_pt,area_eta,area_phi,area_e,area_m,jet_calo,useOriginCorrection ? jet_orig : jet_calo,a->at(j),rho[r],residualoffset+constoffset,true);
	            AreaResidualConstL.at(r)->pt.push_back(area_pt);
	            AreaResidualConstL.at(r)->eta.push_back(area_eta);
	            AreaResidualConstL.at(r)->phi.push_back(area_phi);
	            AreaResidualConstL.at(r)->e.push_back(area_e);
	            AreaResidualConstL.at(r)->m.push_back(jet_area.M());

	            if(TruePt!="") AreaResidualConstL.at(r)->tpt.push_back(tpt->at(j));
	            if(TrueEta!="") AreaResidualConstL.at(r)->teta.push_back(teta->at(j));
	            if(TruePhi!="") AreaResidualConstL.at(r)->tphi.push_back(tphi->at(j));
	            if(TrueE!="") AreaResidualConstL.at(r)->te.push_back(te->at(j));
	            if(TrueM!="") AreaResidualConstL.at(r)->tm.push_back(tm->at(j));
	            if(EventWeight!="") AreaResidualConstL.at(r)->EW = w;
	          }

          } // End of global rhos loop
          ///////////////////////////////////////////////////////////////////////////


          ///////////////////////////////////////////////////////////////////////////
          //
          // ----- LOOP 5.2: local rhos
          //
          //////////

      	  for(int r = 0; r < JetRhos.size(); ++r){
      	    if(doArea){//add rho
      	      float area_pt, area_eta, area_phi, area_e, area_m;
              // std::cout<< jetrho <<std::endl; //<<std::endl;// std::cout << useOriginCorrection  << std::endl;
              JetPileupCorrection(area_pt,area_eta,area_phi,area_e,area_m,jet_calo,useOriginCorrection ? jet_orig : jet_calo,a->at(j),jetrho[r]->at(j),0.,true); // Fails here: jetrho[r]->at(j) is null
      	      AreaL.at(Rhos.size()+r)->pt.push_back(area_pt);
      	      AreaL.at(Rhos.size()+r)->eta.push_back(area_eta);
      	      AreaL.at(Rhos.size()+r)->phi.push_back(area_phi);
      	      AreaL.at(Rhos.size()+r)->e.push_back(area_e);
      	      AreaL.at(Rhos.size()+r)->m.push_back(jet_area.M());


      	      if(TruePt!="") AreaL.at(Rhos.size()+r)->tpt.push_back(tpt->at(j));
      	      if(TrueEta!="") AreaL.at(Rhos.size()+r)->teta.push_back(teta->at(j));
      	      if(TruePhi!="") AreaL.at(Rhos.size()+r)->tphi.push_back(tphi->at(j));
      	      if(TrueE!="") AreaL.at(Rhos.size()+r)->te.push_back(te->at(j));
      	      if(TrueM!="") AreaL.at(Rhos.size()+r)->tm.push_back(tm->at(j));
      	      if(EventWeight!="") AreaL.at(Rhos.size()+r)->EW = w;

      	    }

      	    if(doAreaResidual){//add rho
      	      float area_pt, area_eta, area_phi, area_e, area_m;
      	      float residualoffset = GetResidualOffset(useOriginCorrection ? fabs(jet_orig.Eta()) : fabs(jet_calo.Eta()),mu,npv,DefaultMuRef,DefaultNPVRef,jetMuTerm.at(r),jetNPVTerm.at(r),jetAbsEtaBinsAxis.at(r));
      	      JetPileupCorrection(area_pt,area_eta,area_phi,area_e,area_m,jet_calo,useOriginCorrection ? jet_orig : jet_calo,a->at(j),jetrho[r]->at(j),residualoffset,true);
      	      AreaResidualL.at(Rhos.size()+r)->pt.push_back(area_pt);
      	      AreaResidualL.at(Rhos.size()+r)->eta.push_back(area_eta);
      	      AreaResidualL.at(Rhos.size()+r)->phi.push_back(area_phi);
      	      AreaResidualL.at(Rhos.size()+r)->e.push_back(area_e);
      	      AreaResidualL.at(Rhos.size()+r)->m.push_back(jet_area.M());

      	      if(TruePt!="") AreaResidualL.at(Rhos.size()+r)->tpt.push_back(tpt->at(j));
      	      if(TrueEta!="") AreaResidualL.at(Rhos.size()+r)->teta.push_back(teta->at(j));
      	      if(TruePhi!="") AreaResidualL.at(Rhos.size()+r)->tphi.push_back(tphi->at(j));
      	      if(TrueE!="") AreaResidualL.at(Rhos.size()+r)->te.push_back(te->at(j));
      	      if(TrueM!="") AreaResidualL.at(Rhos.size()+r)->tm.push_back(tm->at(j));
      	      if(EventWeight!="") AreaResidualL.at(Rhos.size()+r)->EW = w;

      	    }
      	    if(doAreaResidualConst){//add rho
      	      float area_pt, area_eta, area_phi, area_e, area_m;
      	      float residualoffset = GetResidualOffset(useOriginCorrection ? fabs(jet_orig.Eta()) : fabs(jet_calo.Eta()),mu,npv,DefaultMuRef,DefaultNPVRef,jetMuTerm.at(r),jetNPVTerm.at(r),jetAbsEtaBinsAxis.at(r));
      	      float constoffset = GetConstOffset(useOriginCorrection ? fabs(jet_orig.Eta()) : fabs(jet_calo.Eta()),jetConstTerm.at(r),jetConstTermAbsEtaBinsAxis.at(r));
      	      JetPileupCorrection(area_pt,area_eta,area_phi,area_e,area_m,jet_calo,useOriginCorrection ? jet_orig : jet_calo,a->at(j),jetrho[r]->at(j),residualoffset+constoffset,true);
      	      AreaResidualConstL.at(Rhos.size()+r)->pt.push_back(area_pt);
      	      AreaResidualConstL.at(Rhos.size()+r)->eta.push_back(area_eta);
      	      AreaResidualConstL.at(Rhos.size()+r)->phi.push_back(area_phi);
      	      AreaResidualConstL.at(Rhos.size()+r)->e.push_back(area_e);
      	      AreaResidualConstL.at(Rhos.size()+r)->m.push_back(jet_area.M());

      	      if(TruePt!="") AreaResidualConstL.at(Rhos.size()+r)->tpt.push_back(tpt->at(j));
      	      if(TrueEta!="") AreaResidualConstL.at(Rhos.size()+r)->teta.push_back(teta->at(j));
      	      if(TruePhi!="") AreaResidualConstL.at(Rhos.size()+r)->tphi.push_back(tphi->at(j));
      	      if(TrueE!="") AreaResidualConstL.at(Rhos.size()+r)->te.push_back(te->at(j));
      	      if(TrueM!="") AreaResidualConstL.at(Rhos.size()+r)->tm.push_back(tm->at(j));
      	      if(EventWeight!="") AreaResidualConstL.at(Rhos.size()+r)->EW = w;

      	    }

      	  } // End of local rhos loop
          ///////////////////////////////////////////////////////////////////////////

        } // End of loop 4 (e)
        ///////////////////////////////////////////////////////////////////////////

	      if(doUncalibrated){UncalibratedL->tree()->Fill(); UncalibratedL->clear();}
	      for(int r = 0 ; r < Rhos.size()+JetRhos.size(); ++r){
	        if(doArea){AreaL.at(r)->tree()->Fill(); AreaL.at(r)->clear();}
	        if(doAreaResidual){AreaResidualL.at(r)->tree()->Fill(); AreaResidualL.at(r)->clear();}
	        if(doAreaResidualConst){AreaResidualConstL.at(r)->tree()->Fill(); AreaResidualConstL.at(r)->clear();}
	        if(doAreaResidualJES){AreaResidualJESL.at(r)->tree()->Fill(); AreaResidualJESL.at(r)->clear();}
	      }
	      event->Fill();

      } // End of loop 3 (NEntrys)
      ///////////////////////////////////////////////////////////////////////////

      if(doUncalibrated) UncalibratedL->tree()->Write();

      for(int r = 0 ; r < Rhos.size()+JetRhos.size(); ++r){
	      if(doArea) AreaL.at(r)->tree()->Write();
	      if(doAreaResidual) AreaResidualL.at(r)->tree()->Write();
	      if(doAreaResidualConst) AreaResidualConstL.at(r)->tree()->Write();
	      if(doAreaResidualJES) AreaResidualJESL.at(r)->tree()->Write();
      }

      event->Write();

    } // End of loop 2 (JetAlgos - only one)
    ///////////////////////////////////////////////////////////////////////////

    OutputFileF->Close();
    InputFileF->Close();

    delete residualE;


  } // End of loop 1 (RTags)


}


