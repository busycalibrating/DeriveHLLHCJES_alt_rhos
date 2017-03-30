#include "TCollection.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "TTree.h"
#include "TLorentzVector.h"
#include "TAxis.h"
#include "TEnv.h"
#include "TFile.h"
#include "CalibrationTools.h"

#include <iostream>

class ScriptEtaJESCorrection 
{


 public:
  //ScriptEtaJESCorrection();  
  //EtaJESCorrection(TEnv * config);
  //ScriptEtaJESCorrection(TEnv * config, TString jetAlgo);

  //virtual ~ScriptEtaJESCorrection();

  //virtual bool initializeTool(TEnv * config, TString jetAlgo);
  //void initializeTool(TEnv* JESconfig);

  //void calibrateImpl(float &pt, float &DETeta, float &phi, float &e, float &m,float &JESpt, float &JESeta, float &JESphi, float &JESe, float &JESm) const;
 
  //double getJES(double E_uncorr, double eta_det) const;
  //double getLowPtJES(double E_uncorr, double eta_det) const;
  //double getEtaCorr(double E_corr, double eta_det) const;
  //double getLogPolN(const double *factors, double x) const;
  //double getLogPolNSlope(const double *factors, double x) const;
  //int getEtaBin(double eta_det) const;

  TEnv * m_config;
  TEnv * m_JESconfig;
  TString m_jetAlgo;
  TString m_calibAreaTag;
  bool m_freezeJESatHighE;

  TString m_jesDesc;
  double m_minPt_JES, m_minPt_EtaCorr, m_maxE_EtaCorr;
  unsigned int m_lowPtExtrap;
  double m_lowPtMinR;
  bool m_useSecondaryminPt_JES;
  double m_etaSecondaryminPt_JES;
  double m_secondaryminPt_JES;

  TAxis * m_etaBinAxis;
  
  // 90 eta bins, and up to 9 parameter for the pol-fit
  const static unsigned int s_nEtaBins=90;
  const static unsigned int s_nParMin=1;
  const static unsigned int s_nParMax=9;
  unsigned int s_nPar; // number of parameters in config file
  double m_JESFactors[s_nEtaBins][s_nParMax];
  double m_JES_MinPt_Slopes[s_nEtaBins];
  double m_JES_MinPt_E[s_nEtaBins];
  double m_JES_MinPt_R[s_nEtaBins];
  //double m_JES_MinPt_Rmin[s_nEtaBins];
  double m_JES_MinPt_Param1[s_nEtaBins];
  double m_JES_MinPt_Param2[s_nEtaBins];
  double m_etaCorrFactors[s_nEtaBins][s_nParMax];
  double m_energyFreezeJES[s_nEtaBins];
  
  ScriptEtaJESCorrection()
    : m_config(NULL), m_jetAlgo(""), m_minPt_JES(10), m_minPt_EtaCorr(8), m_maxE_EtaCorr(2500),
    m_lowPtExtrap(0), m_lowPtMinR(0.25),
    m_etaBinAxis(NULL)
      { }

  ScriptEtaJESCorrection(TEnv * config, TString jetAlgo)
    : m_config(config), m_jetAlgo(jetAlgo), m_minPt_JES(10), m_minPt_EtaCorr(8), m_maxE_EtaCorr(2500),
    m_lowPtExtrap(0), m_lowPtMinR(0.25), m_etaBinAxis(NULL)
    { }

  void initializeTool(TEnv* JESconfig) {
    //TString calibFile(calibFileIn);
    m_JESconfig = JESconfig;

    m_minPt_JES = m_config->GetValue(m_jetAlgo+".MinPtForETAJES",10);
    //Which extrapolation method to use at low Et (Et < _minPt_JES)
    m_lowPtExtrap = m_config->GetValue("LowPtJESExtrapolationMethod",0);
    //For order 2 extrapolation only, set the minimum value of the response for Et = 0
    m_lowPtMinR = m_config->GetValue("LowPtJESExtrapolationMinimumResponse",0.25);
    //Allowing to use different minPt_JES depending on eta (only for extrapolation methon 1)
    m_useSecondaryminPt_JES = m_config->GetValue(m_jetAlgo+".UseSecondaryMinPtForETAJES", false);
    //Starting eta for secondary minPt_JES (Default |eta|>=1.9) (Used only if UseSecondaryMinPtForETAJES is true)
    m_etaSecondaryminPt_JES = m_config->GetValue(m_jetAlgo+".EtaSecondaryMinPtForETAJES", 1.9);
    //SecondaryminPt_JES (Default 7 GeV) (Used only if UseSecondaryMinPtForETAJES is true)
    m_secondaryminPt_JES = m_config->GetValue(m_jetAlgo+".SecondaryMinPtForETAJES",7);
    // Freeze JES correction at maximum values of energy for each eta bin
    m_freezeJESatHighE = m_config->GetValue(m_jetAlgo+".FreezeJEScorrectionatHighE", false);
  
    std::vector<double> JESetaBins = VectorizeD(m_config->GetValue("JES.EtaBins",""));
    if (JESetaBins.size()==0){ // default binning
      for (int i=0;i<=90; i++) 
	JESetaBins.push_back(0.1*i-4.5);
    }
    else if (JESetaBins.size()==0) {std::cout<<"JES.EtaBins incorrectly specified"<<std::endl;}
    else if (JESetaBins.size()>s_nEtaBins+1) {
      std::cout<< "JES.EtaBins has " << JESetaBins.size()-1 << " bins, can be maximally 90!"<<std::endl; 
    }
    
    std::cout<<"BIN0 "<<JESetaBins[0]<<std::endl;
    m_etaBinAxis = new TAxis(JESetaBins.size()-1,&JESetaBins[0]);
  
    for (uint ieta=0;ieta<JESetaBins.size()-1;++ieta) {
      TString JetAlgoSansJets(m_jetAlgo);
      JetAlgoSansJets.ReplaceAll("Jets","");
      //JetAlgoSansJets.ReplaceAll("My","");

      TString key=Form("JES.%s_Bin%d",JetAlgoSansJets.Data(),ieta);
      std::cout<<ieta<<" "<<key<<std::endl;
      std::vector<double> params = VectorizeD(m_JESconfig->GetValue(key,""));
      s_nPar = params.size();
      std::cout<<"NPAR "<<s_nPar<<std::endl;
      if (s_nPar<s_nParMin || s_nPar>s_nParMax) { std:;cout<<  "Cannot read JES calib constants " << std::endl; }
      for (uint ipar=0;ipar<s_nPar;++ipar){
	m_JESFactors[ieta][ipar] = params[ipar];
	std::cout<<"parms "<< params[ipar]<<std::endl;
      }
      //Protections for high order extrapolation methods at low Et (Et < _minPt_JES)
      if(m_lowPtExtrap > 0) {
	//Calculate the slope of the response curve at the minPt for each eta bin
	//Used in the GetLowPtJES method when Pt < minPt
	const double *factors = m_JESFactors[ieta];
	double Ecutoff;
	if(!m_useSecondaryminPt_JES) Ecutoff = m_minPt_JES*cosh(JESetaBins[ieta]);
	else {
	  if(fabs(JESetaBins[ieta]) < m_etaSecondaryminPt_JES) Ecutoff = m_minPt_JES*cosh(JESetaBins[ieta]);
	  else{ Ecutoff = m_secondaryminPt_JES*cosh(JESetaBins[ieta]);}
	}
	const double Rcutoff = getLogPolN(factors,Ecutoff);
	const double Slope = getLogPolNSlope(factors,Ecutoff);
	if(Slope > Rcutoff/Ecutoff) std::cout << "Slope of calibration curve at minimum ET is too steep for the JES factors of etabin " << ieta << ", eta = " << JESetaBins[ieta] << std::endl;
    
	m_JES_MinPt_E[ieta] = Ecutoff;
	m_JES_MinPt_R[ieta] = Rcutoff;
	m_JES_MinPt_Slopes[ieta] = Slope;
      
	//Calculate the parameters for a 2nd order polynomial extension to the calibration curve below minimum ET
	//Used in the GetLowPtJES method when Pt < minPt
	if(m_lowPtExtrap == 2) {
	  const double h = m_lowPtMinR;
	  const double Param1 = (2/Ecutoff)*(Rcutoff-h)-Slope;
	  const double Param2 = (0.5/Ecutoff)*(Slope-Param1);
	  //Slope of the calibration curve should always be positive
	  if( Param1 < 0 || Param1 + 2*Param2*Ecutoff < 0) std::cout<<"Polynomial extension to calibration curve below minimum ET is not monotonically increasing for etabin " << ieta << ", eta = " << JESetaBins[ieta] << std::endl;
	  m_JES_MinPt_Param1[ieta] = Param1;
	  m_JES_MinPt_Param2[ieta] = Param2;
	}
      }
    
      // Read in jet eta calibration factors
      key=Form("EtaCorr.%s_Bin%d",JetAlgoSansJets.Data(),ieta);
      params = VectorizeD(m_JESconfig->GetValue(key,""));
      if (params.size()!=s_nPar) { std::cout<< "Cannot read jet eta calib constants " << key << std::endl;}
      for (uint ipar=0;ipar<s_nPar;++ipar) m_etaCorrFactors[ieta][ipar] = params[ipar];
    
      if(m_freezeJESatHighE){ // Read starting energy values to freeze JES correction
	key=Form("EmaxJES.%s_Bin%d",JetAlgoSansJets.Data(),ieta);
	params = VectorizeD(m_JESconfig->GetValue(key,""));
	if (params.size()!=1) { std::cout<< "Cannot read starting energy for the freezing of JES correction " << key << std::endl; }
	for (uint ipar=0;ipar<1;++ipar) m_energyFreezeJES[ieta] = params[ipar];
      }
    }
    
  } 



  //void JEScorrection(float &pt, float &eta, float &phi, float &e, float &m,float &pt, float &eta, float &phi, float &e, float &m){
  //  
  //  
  //  
  //}

  void calibrateImpl(float &pt, float &DETeta, float &phi, float &e, float &m,float &JESpt, float &JESeta, float &JESphi, float &JESe, float &JESm) const {
    if(e < 0. || pt < 0.){
      std::cout<<"WHAT!"<<std::endl;
      JESpt = pt;
      JESe = e;
      JESphi = phi;
      JESeta = DETeta;
      JESm = m;
      return;
    }
    
    float jes = getJES(e, DETeta );
    float corrpt = jes*pt;
    float deta = getEtaCorr( e, DETeta );
    float correta = DETeta+deta;
    std::cout<<"deteta "<<DETeta<<"deta "<<deta<<"correta "<< correta << " "<< corrpt*cosh(DETeta) << " " << cosh(correta) << std::endl;
    TLorentzVector TLV;
    
    TLV.SetPtEtaPhiM( corrpt*cosh(DETeta)/cosh(correta),correta,phi,m );
    JESpt = TLV.Pt();
    JESe = TLV.E();
    JESphi = TLV.Phi();
    JESeta = TLV.Eta();
    JESm = TLV.M();
  }

  double getJES(double E_uncorr, double eta_det) const {

    double E = E_uncorr;//should already be in GeV /1000.; // E in GeV
    //Check if the Pt goes below the minimum value, if so use the special GetLowPtJES method
    if(m_useSecondaryminPt_JES){
      if(fabs(eta_det) < m_etaSecondaryminPt_JES && E/cosh(eta_det) < m_minPt_JES){
	double R = getLowPtJES(E,eta_det);
	return 1.0/R;
      }
      if(fabs(eta_det) >= m_etaSecondaryminPt_JES && E/cosh(eta_det) < m_secondaryminPt_JES){
	double R = getLowPtJES(E,eta_det);
	return 1.0/R;
      }
    }else{
      if ( E/cosh(eta_det) < m_minPt_JES ) {

	std::cout<<"EMIN "<< E << " "  << E/cosh(eta_det) << " " << m_minPt_JES <<std::endl;
	
	double R = getLowPtJES(E,eta_det);
	return 1.0/R;
      }
    }

    // Get the factors
    int ieta = getEtaBin(eta_det);
    const double *factors = m_JESFactors[ieta];
    std::cout<<"factor0 "<<factors[0]<<std::endl;
 

    // Freeze correction
    if(m_freezeJESatHighE && E>m_energyFreezeJES[ieta] && m_energyFreezeJES[ieta]!=-1) E = m_energyFreezeJES[ieta];
  
    // Calculate the jet response and then the JES as 1/R
    double R = getLogPolN(factors,E);
    std::cout<<"R "<<R<<std::endl;
    return 1.0/R;
  }

  double getLowPtJES(double E_uncorr, double eta_det) const {
    int ieta = getEtaBin(eta_det);
    if (m_lowPtExtrap == 0) {
      std::cout<<"got her"<<std::endl;
      const double *factors = m_JESFactors[ieta];
      double E = m_minPt_JES*cosh(eta_det);
      double R = getLogPolN(factors,E);
      return R;
    }
    else if (m_lowPtExtrap == 1) {
      double Ecutoff = m_JES_MinPt_E[ieta];
      double Rcutoff = m_JES_MinPt_R[ieta];
      double slope = m_JES_MinPt_Slopes[ieta];
      double R = slope*(E_uncorr-Ecutoff)+Rcutoff;
      return R;
    }
    else if(m_lowPtExtrap == 2) {
      double minR = m_lowPtMinR;
      double R = minR + m_JES_MinPt_Param1[ieta]*E_uncorr + m_JES_MinPt_Param2[ieta]*E_uncorr*E_uncorr;
      return R;
    }
    else std::cout<<"Incorrect specification of low Pt JES extrapolation, please check the value of the LowPtJESExtrapolationMethod config flag."<<std::endl;
    return 1;
  }

  double getEtaCorr(double E_corr, double eta_det) const {
    int ieta = getEtaBin(eta_det);
    const double *factors = m_etaCorrFactors[ieta];
    std::cout<<"etafactor0 "<<factors[0]<<std::endl;
  
    double E = E_corr;//Should already be in GeV /1000.;
    if ( E < m_minPt_EtaCorr*cosh(eta_det) ) 
      E = m_minPt_EtaCorr*cosh(eta_det);
    if ( E>m_maxE_EtaCorr ) E=m_maxE_EtaCorr;
  
    double etaCorr = getLogPolN(factors,E);
  
    // This is ( reco_eta - truth_eta )
    // to make it an additive correction return the negative value
    return -etaCorr;
  }

  double getLogPolN(const double *factors, double x) const {
    double y=0;
    for ( uint i=0; i<s_nPar; ++i )
      y += factors[i]*TMath::Power(log(x),Int_t(i));
    return y;
  }

  double getLogPolNSlope(const double *factors, double x) const {
    double y=0;
    for ( uint i=0; i<s_nPar; ++i )
      y += i*factors[i]*TMath::Power(log(x),Int_t(i-1))/x;
    return y;
  }

  int getEtaBin(double eta_det) const {
    int bin = m_etaBinAxis->FindBin(eta_det);
    std::cout<<"eta_det "<<eta_det<<" bin "<<bin<<std::endl;
    if (bin<=0) return 0;
    if (bin>m_etaBinAxis->GetNbins()) return bin-2; // overflow
    return bin-1;
  }
};
/*

  
};

*/
