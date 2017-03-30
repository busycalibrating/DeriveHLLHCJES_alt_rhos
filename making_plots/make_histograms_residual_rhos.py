import ctypes
import glob
import sys,os
import subprocess
import ROOT
from ROOT import TFile, gDirectory
from ROOT import TH1F, TH1D
from optparse import OptionParser

ROOT.gROOT.LoadMacro("/home/ajhorton/atlasstyle-00-03-04/AtlasStyle.C")
#ROOT.SetAtlasStyle()

##################################################
# Declare paramaters
#####

inputfilename = "r7725_r7676_p2794.root"

inputfile = ROOT.TFile(inputfilename)
mychain = gDirectory.Get("AntiKt4EMTopoJets;1")
entries = mychain.GetEntriesFast()

c_truth     = ROOT.TCanvas("Truth rho", "Truth rho", 174, 52, 700, 500);
c_global    = ROOT.TCanvas("Global rho", "Global rho", 174, 52, 700, 500);
c_strip_1   = ROOT.TCanvas("Strip rho 1R", "Strip rho 1R", 174, 52, 700, 500);
c_strip_2   = ROOT.TCanvas("Strip rho 2R", "Strip rho 2R", 174, 52, 700, 500);
c_strip_3   = ROOT.TCanvas("Strip rho 3R", "Strip rho 3R", 174, 52, 700, 500);
c_circle_1  = ROOT.TCanvas("Circle rho 1R", "Circle rho 1R", 174, 52, 700, 500);
c_circle_2  = ROOT.TCanvas("Circle rho 2R", "Circle rho 2R", 174, 52, 700, 500);
c_circle_3  = ROOT.TCanvas("Circle rho 3R", "Circle rho 3R", 174, 52, 700, 500);
c_dough_1_3 = ROOT.TCanvas("Doughnut rho 1R-3R", "Doughnut rho 1R-3R", 174, 52, 700, 500);
c_dough_1_4 = ROOT.TCanvas("Doughnut rho 1R-4R", "Doughnut rho 1R-4R", 174, 52, 700, 500);
c_dough_2_4 = ROOT.TCanvas("Doughnut rho 2R-4R", "Doughnut rho 2R-4R", 174, 52, 700, 500);
c_dough_2_5 = ROOT.TCanvas("Doughnut rho 2R-5R", "Doughnut rho 2R-5R", 174, 52, 700, 500);

h_truth     = ROOT.TH1D("Truth rho", "Truth rho", 100, 0, 50)
h_global    = ROOT.TH1D("Global rho", "Global rho", 100, 0, 50)
h_strip_1   = ROOT.TH1D("Strip rho 1R", "Strip rho 1R", 100, 0, 50)
h_strip_2   = ROOT.TH1D("Strip rho 2R", "Strip rho 2R", 100, 0, 50)
h_strip_3   = ROOT.TH1D("Strip rho 3R", "Strip rho 3R",  100, 0, 50)
h_circle_1  = ROOT.TH1D("Circle rho 1R", "Circle rho 1R", 100, 0, 50)
h_circle_2  = ROOT.TH1D("Circle rho 2R", "Circle rho 2R", 100, 0, 50)
h_circle_3  = ROOT.TH1D("Circle rho 3R", "Circle rho 3R", 100, 0, 50)
h_dough_1_3 = ROOT.TH1D("Doughnut rho 1R-3R", "Doughnut rho 1R-3R",100, 0, 50)
h_dough_1_4 = ROOT.TH1D("Doughnut rho 1R-4R", "Doughnut rho 1R-4R",100, 0, 50)
h_dough_2_4 = ROOT.TH1D("Doughnut rho 2R-4R", "Doughnut rho 2R-4R",100, 0, 50)
h_dough_2_5 = ROOT.TH1D("Doughnut rho 2R-5R", "Doughnut rho 2R-5R",100, 0, 50)

for jentry in xrange( entries ):
  ientry = mychain.LoadTree( jentry )
  if ientry < 0:
    break

  nb = mychain.GetEntry( jentry )
  if nb <= 0:
    continue

  truthrho = mychain.RHO
  h_truth.Fill(truthrho)

  globalrho = mychain.GLOBAL_RHO
  h_global.Fill(globalrho)

  rhostrip1 = mychain.RHOSTRIP_1R
  for i in range(0, rhostrip1.size()):
    h_strip_1.Fill(rhostrip1[i])

  rhostrip2 = mychain.RHOSTRIP_2R
  for i in range(0, rhostrip2.size()):
    h_strip_2.Fill(rhostrip2[i])

  rhostrip3 = mychain.RHOSTRIP_3R
  for i in range(0, rhostrip3.size()):
    h_strip_3.Fill(rhostrip3[i])

  rhocircle1 = mychain.RHOCIRCLE_1R
  for i in range(0, rhocircle1.size()):
    h_circle_1.Fill(rhocircle1[i])

  rhocircle2 = mychain.RHOCIRCLE_2R
  for i in range(0, rhocircle2.size()):
    h_circle_2.Fill(rhocircle2[i])

  rhocircle3 = mychain.RHOCIRCLE_3R
  for i in range(0, rhocircle3.size()):
    h_circle_3.Fill(rhocircle3[i])

  rhodough1 = mychain.RHODOUGHNUT_1_3R
  for i in range(0, rhodough1.size()):
    h_dough_1_3.Fill(rhodough1[i])

  rhodough2 = mychain.RHODOUGHNUT_1_4R
  for i in range(0, rhodough2.size()):
    h_dough_1_4.Fill(rhodough2[i])

  rhodough3 = mychain.RHODOUGHNUT_2_4R
  for i in range(0, rhodough3.size()):
    h_dough_2_4.Fill(rhodough3[i])

  rhodough4 = mychain.RHODOUGHNUT_2_5R
  for i in range(0, rhodough4.size()):
    h_dough_2_5.Fill(rhodough4[i])

c_truth.cd()
h_truth.SetTitle( "Truth #rho" )
h_truth.GetYaxis().SetTitle("Events")
X=h_truth.GetXaxis()
X.SetTitle("p_T/A")
h_truth.Draw()
c_truth.SaveAs("rho_truth.eps")

c_global.cd()
h_global.SetTitle( "Global #rho" )
h_global.GetYaxis().SetTitle("Events")
X=h_global.GetXaxis()
X.SetTitle("p_T/A")
h_global.Draw()
c_global.SaveAs("rho_global.eps")

##################################################
# strip
#####

c_strip_1.cd()
h_strip_1.SetTitle( "Strip #rho <1R" )
h_strip_1.GetYaxis().SetTitle("Events")
X=h_strip_1.GetXaxis()
X.SetTitle("p_T/A")
h_strip_1.Draw()
c_strip_1.SaveAs("rho_strip_1.eps")

c_strip_2.cd()
h_strip_2.SetTitle( "Strip #rho <2R" )
h_strip_2.GetYaxis().SetTitle("Events")
X=h_strip_2.GetXaxis()
X.SetTitle("p_T/A")
h_strip_2.Draw()
c_strip_2.SaveAs("rho_strip_2.eps")

c_strip_3.cd()
h_strip_3.SetTitle( "Strip #rho <3R" )
h_strip_3.GetYaxis().SetTitle("Events")
X=h_strip_3.GetXaxis()
X.SetTitle("p_T/A")
h_strip_3.Draw()
c_strip_3.SaveAs("rho_strip_3.eps")

##################################################
# circle
#####

c_circle_1.cd()
h_circle_1.SetTitle( "Circle #rho <1R" )
h_circle_1.GetYaxis().SetTitle("Events")
X=h_circle_1.GetXaxis()
X.SetTitle("p_T/A")
h_circle_1.Draw()
c_circle_1.SaveAs("rho_circle_1.eps")

c_circle_2.cd()
h_circle_2.SetTitle( "Circle #rho <2R" )
h_circle_2.GetYaxis().SetTitle("Events")
X=h_circle_2.GetXaxis()
X.SetTitle("p_T/A")
h_circle_2.Draw()
c_circle_2.SaveAs("rho_circle_2.eps")

c_circle_3.cd()
h_circle_3.SetTitle( "Circle #rho <3R" )
h_circle_3.GetYaxis().SetTitle("Events")
X=h_circle_3.GetXaxis()
X.SetTitle("p_T/A")
h_circle_3.Draw()
c_circle_3.SaveAs("rho_circle_3.eps")

##################################################
# doughnut
#####

c_dough_1_3.cd()
h_dough_1_3.SetTitle( "Dough #rho 1R - 3R" )
h_dough_1_3.GetYaxis().SetTitle("Events")
X=h_dough_1_3.GetXaxis()
X.SetTitle("p_T/A")
h_dough_1_3.Draw()
c_dough_1_3.SaveAs("rho_dough_1_3.eps")

c_dough_1_4.cd()
h_dough_1_4.SetTitle( "Dough #rho 1R - 4R" )
h_dough_1_4.GetYaxis().SetTitle("Events")
X=h_dough_1_4.GetXaxis()
X.SetTitle("p_T/A")
h_dough_1_4.Draw()
c_dough_1_4.SaveAs("rho_dough_1_4.eps")

c_dough_2_4.cd()
h_dough_2_4.SetTitle( "Dough #rho 2R - 4R" )
h_dough_2_4.GetYaxis().SetTitle("Events")
X=h_dough_2_4.GetXaxis()
X.SetTitle("p_T/A")
h_dough_2_4.Draw()
c_dough_2_4.SaveAs("rho_dough_2_4.eps")

c_dough_2_5.cd()
h_dough_2_5.SetTitle( "Dough #rho 2R - 5R" )
h_dough_2_5.GetYaxis().SetTitle("Events")
X=h_dough_2_5.GetXaxis()
X.SetTitle("p_T/A")
h_dough_2_5.Draw()
c_dough_2_5.SaveAs("rho_dough_2_5.eps")

