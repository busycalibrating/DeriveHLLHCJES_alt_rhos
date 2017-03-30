{
   cout << endl << "******* TEST SIMPLE LIKELIHOOD FIT *******" << endl;

   gRandom->SetSeed(0);
   h = new TH1D("","",15000,0,3);
   h->Sumw2();
   // J0
   for (int i=0;i<100;++i) h->Fill(gRandom->Gaus(1,0.4),100); // J0
   for (int i=0;i<100;++i) h->Fill(gRandom->Gaus(1,0.4),1); // J1
   for (int i=0;i<100;++i) h->Fill(gRandom->Gaus(1,0.4),0.01); // J2
   TCanvas *c1 = new TCanvas();
   c1->cd();
   h->Draw();
   f = new TF1("","gaus",0,2);
   h->Fit(f,"RWL");

   printf("mean: %.3f +/- %.3f, err/mean = %.3f\n",
          f->GetParameter(1),f->GetParError(1),
          pow((f->GetParameter(1)-1)/f->GetParError(1),2));


   cout << endl << "******* TEST QUANTILES *******" << endl;

   h2 = new TH1D("","",15000,-3.,3.);
   //   h2->StatOverflows(kTRUE);
   //   h2->Sumw2();
   for (int i=0;i<1e7;++i) h2->Fill(gRandom->Gaus(0,1.),1); // J1


   Double_t entries = h2->GetEntries();

   // include underflow and overflow bin contents in the computation of the quantiles
   Int_t Nbins = h2->GetNbinsX();
   h2->SetBinContent( 1, h2->GetBinContent(0)+h2->GetBinContent(1) );
   h2->SetBinContent( Nbins, h2->GetBinContent(Nbins)+h2->GetBinContent(Nbins+1) );
   h2->ComputeIntegral();

   Double_t integral = h2->GetSumOfWeights();
   cout << "integral = " << integral << "  entries = " << entries << "  uf = " << h2->GetBinContent(0) << "  of =" << h2->GetBinContent(Nbins+1) << endl;

   Int_t nq = 4;
   Double_t xq[nq];  // position where to compute the quantiles in [0,1]
   Double_t yq[nq];  // array to contain the quantiles
   xq[0] = 2.27501319480000186e-02;
   xq[1] = 1.58655253931499984e-01;
   xq[2] = 1. - 1.58655253931499984e-01;
   xq[3] = 1. - 2.27501319480000186e-02;

   if( h2->GetBinContent(0)/integral > xq[0] ){ cout << "WARNING: low quantile(s) might be biassed. Consider extending the histogram." << endl; }
   if( h2->GetBinContent(Nbins+1)/integral > xq[0] ){ cout << "WARNING: high quantile(s) might be biassed. Consider extending the histogram." << endl; }

   h2->GetQuantiles(nq,yq,xq);

   // restore the original bin content
   h2->SetBinContent( 1, -h2->GetBinContent(0)+h2->GetBinContent(1) );
   h2->SetBinContent( Nbins, h2->GetBinContent(Nbins)-h2->GetBinContent(Nbins+1) );

   cout << endl << xq[0] << "  " << xq[1] << "  " << xq[2] << "  " << xq[3] << "  " << endl;
   cout << endl << yq[0] << "  " << yq[1] << "  " << yq[2] << "  " << yq[3] << "  " << endl;


   cout << endl << "******* TEST TRUTH BALANCE FITS *******" << endl;

   Double_t Emin = 0.85001, Emax = 1.14999;
   Int_t iMin = -1, iMax = -1;
   TFile *file = TFile::Open( "Pythia_EM_LC_correctPythiaDijets_OriginalBinning_ResponseHistograms.root" );
   //   file->ls();
   TH1D *h3 = new TH1D( *((TH1D*)file->Get("AntiKt4EMTopo_All/respVsPtAllAntiKt4EMTopo_absEta_0_0.3_NPV_0_50_pt_15_20")) );
   //   h3->Print("all");

   for( Int_t i=0; i<h3->GetNbinsX(); i++ ){
      /*
      if( h3->GetBinContent(i+1) < 1. ){
         cout << i << " " << h3->GetBinContent(i+1) << " " << h3->GetBinError(i+1) << "  " << h3->GetBinCenter(i+1) << "  " << h3->GetBinWidth(i+1) << endl;
         h3->SetBinError( i+1, 1. );
      }
      */
      if( h3->GetBinCenter(i+1) >= Emin  &&  iMin < 0 ){ iMin = i; }
      if( h3->GetBinCenter(i+1) <= Emax ){ iMax = i; }
   }
   cout << "iMin = " << iMin << "  iMax = " << iMax << endl;

   TCanvas *c3 = new TCanvas();
   c3->cd();
   h3->Draw();
   h3->GetXaxis()->SetRangeUser( Emin, Emax );
   f3 = new TF1("","gaus",0,2);
   h3->Fit(f3,"RWL");
   h3->Fit(f3,"MRWL");

   cout << endl << "-------------------------------------------------------------------------------------------------------------------------------------------------" << endl << endl;

   TH1D *h3C = new TH1D( "reducedCopy", "reducedCopy", iMax-iMin+1, h3->GetBinCenter(iMin+1) - h3->GetBinWidth(iMin+1)/2., h3->GetBinCenter(iMax+1) + h3->GetBinWidth(iMax+1)/2. );
   for( Int_t i=iMin; i<=iMax; i++ ){
      h3C->SetBinContent( i+1-iMin, h3->GetBinContent(i+1) );
      h3C->SetBinError( i+1-iMin, h3->GetBinError(i+1) );
   }
   //   h3C->Print("all");
   h3C->SetLineColor(4);
   h3C->Draw("same");
   f3C = new TF1("","gaus",0,2);
   h3C->Fit(f3C,"MRWLV");

}
