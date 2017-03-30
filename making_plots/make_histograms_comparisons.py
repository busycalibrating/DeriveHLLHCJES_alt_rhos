import ctypes, glob, sys, os, subprocess
import ROOT
from ROOT import *
from array import array

ROOT.gROOT.SetBatch(True)
ROOT.gROOT.Macro('$ROOTCOREDIR/scripts/load_packages.C')
ROOT.gStyle.SetOptStat(0)

#ROOT.gROOT.LoadMacro("/home/ajhorton/atlasstyle-00-03-04/AtlasStyle.C")
#ROOT.SetAtlasStyle()

###########################################################################
# Initialize parameters

LocalConfig = "/cluster/warehouse/ddobre/thesis/DeriveHLLHCJES_test2/DeriveHLLHCJES/macros/Local.config"

# define which algos you are using
JetAlgos = ["AntiKt4EMTopoJets"]

# define what correction steps you are doing
Corrections = ["Area", "AreaResidual", "AreaResidualConst"]

#define colour list
Colours = [kBlack, kRed, kGreen, kBlue, kYellow, kMagenta, kCyan, kOrange  , kSpring  , kTeal    , kAzure   , kViolet  , kPink]
Markers = [kDot  , kDot, kDot  , kDot , kDot   , kDot    , kDot , kMultiply, kMultiply, kMultiply, kMultiply, kMultiply, kMultiply]

# define y axis ranges used for response of allrhos plots
yaxis_ranges_allrhos = [ "0.0_0.8", "0.0_0.8", "0.6_0.9" ]

###########################################################################
# Extract names of calculated rhos

localC = ROOT.TEnv(LocalConfig)

# etabins =  numpy.array( map( float, (localC.GetValue("ResidualAbsEtaBins","0 2 4 6")).split(" ") ) )
etabins =  [0, 1.2, 2.1]
ptbins = [5.0,10.0,20.0,30.0,45.0,60.0,80.0,110.0,160.0]

Rhos = (localC.GetValue("Tag", "FINE")).split(" ")
JetRhos = (localC.GetValue("JetRho"," ")).split(" ")
Rhos.extend(JetRhos)



###########################################################################
# Make array of all of the histograms to look at

TreeList = []

for algo in JetAlgos:
  TreeList.append("{0}Uncalibrated".format(algo))
  for rho in Rhos:
    for correction in Corrections:
      TreeList.append("{0}{1}{2}".format(algo,rho,correction))

etabins_string = [] # Use this to simplify splitting the stats by eta ranges later

for i in xrange(0,len(etabins)-1):
  etabins_string.append("{0}_{1}".format(etabins[i],etabins[i+1]))



###########################################################################
# Make lists of comparisons:

################
# compare all rhos between correction stages:
compare_allrhos_diffcorr_list = []

for algo in JetAlgos:
  for i in xrange(0, len(Corrections)):
    compare_allrhos_diffcorr_list.append([])
    for rho in Rhos:
      compare_allrhos_diffcorr_list[i].append("{0}{1}{2}".format(algo,rho,Corrections[i]))

################
# compare all correction stages for each rho:
compare_allcorr_diffrhos_list = []

for algo in JetAlgos:
  for i in xrange(0, len(Rhos)):
    compare_allcorr_diffrhos_list.append([])
    for corr in Corrections:
      compare_allcorr_diffrhos_list[i].append("{0}{1}{2}".format(algo,Rhos[i],corr))



###########################################################################
# Begin looping over the comparison lists
# Formatting for the hists = [Resolution/Response]_[correction]_[etalow]_[etahigh]
# looping over comparisons between all rhos and different corrections
#   make a histogram list for resolution and response
#   each of these lists will have a list corresponding to all of the plots of one eta bin

##################################################
# Compare all rhos for each correction
##################################################

for i in xrange(0, len(compare_allrhos_diffcorr_list)):
  resolutionHistList = []
  responseHistList = []

  # set up how many expected sets of hists to merge
  for bins in etabins_string:
    resolutionHistList.append([])
    responseHistList.append([])

  # gather the root files generated in the previous step
  for target in compare_allrhos_diffcorr_list[i]:
    myfile = ROOT.TFile("resolution_plots/root/{0}.root".format(target))

    # fill the index of the HistLists (eta bin) with the set of histograms to overlay
    for j in xrange(0,len(etabins_string)):
      tempHist = myfile.Get("Resolution_{0}_eta_{1}".format(target.split("Jets")[1],etabins_string[j]))
      tempHist.SetDirectory(0)
      resolutionHistList[j].append(tempHist)

      tempHist = myfile.Get("Response_{0}_eta_{1}".format(target.split("Jets")[1],etabins_string[j]))
      tempHist.SetDirectory(0)
      responseHistList[j].append(tempHist)

    myfile.Close()

  #########################
  # done getting hists - printing histograms

  # Resolution
  ResolutionCanvas = ROOT.TCanvas("resolution", "resolution", 174, 52, 700, 500)

  for bins in xrange(0,len(resolutionHistList)):
    leg = ROOT.TLegend(0.7,0.6,0.9,0.9)
    resolutionHistList[bins][0].Draw()
    resolutionHistList[bins][0].GetYaxis().SetRangeUser(0.05,0.3)

    resolutionHistList[bins][0].SetLineColor(kBlack)
    resolutionHistList[bins][0].SetMarkerColor(kBlack)

    leg.AddEntry(resolutionHistList[bins][0], Rhos[0],"lp")

    for k in xrange(1,len(resolutionHistList[bins])):
      resolutionHistList[bins][k].Draw("same")
      resolutionHistList[bins][k].SetLineColor(Colours[k])
      resolutionHistList[bins][k].SetMarkerColor(Colours[k])
      resolutionHistList[bins][k].SetMarkerStyle(Markers[k])

      leg.AddEntry(resolutionHistList[bins][k], Rhos[k],"lp")
      ResolutionCanvas.Update()

    leg.Draw()
    ResolutionCanvas.SaveAs("resolution_plots/overlay/resolution_allrhos_{0}_{1}.eps".format(Corrections[i], etabins_string[bins] ))
    ResolutionCanvas.Delete()

  # Response
  ResponseCanvas = ROOT.TCanvas("response", "response", 174, 52, 700, 500)

  for bins in xrange(0,len(responseHistList)):
    leg = ROOT.TLegend(0.7,0.6,0.9,0.9 )
    responseHistList[bins][0].Draw()
    ymin = float(yaxis_ranges_allrhos[i].split("_")[0])
    ymax = float(yaxis_ranges_allrhos[i].split("_")[1])

    responseHistList[bins][0].GetYaxis().SetRangeUser(ymin,ymax)
    responseHistList[bins][0].SetLineColor(kBlack)
    responseHistList[bins][0].SetMarkerColor(kBlack)

    leg.AddEntry(responseHistList[bins][0], Rhos[0],"lp")

    for k in xrange(1,len(responseHistList[bins])):
      responseHistList[bins][k].Draw("same")
      responseHistList[bins][k].SetLineColor(Colours[k])
      responseHistList[bins][k].SetMarkerColor(Colours[k])
      responseHistList[bins][k].SetMarkerStyle(Markers[k])

      leg.AddEntry(responseHistList[bins][k], Rhos[k],"lp")
      ResponseCanvas.Update()

    leg.Draw()
    ResponseCanvas.SaveAs("resolution_plots/overlay/response_allrhos_{0}_{1}.eps".format(Corrections[i], etabins_string[bins]))
    ResponseCanvas.Delete()


##################################################
# Compare all corrections for each rho
##################################################

for i in xrange(0, len(compare_allcorr_diffrhos_list)):
  resolutionHistList = []
  responseHistList = []

  # set up how many expected sets of hists to merge
  for bins in etabins_string:
    resolutionHistList.append([])
    responseHistList.append([])

  # gather the root files generated in the previous step
  for target in compare_allcorr_diffrhos_list[i]:
    myfile = ROOT.TFile("resolution_plots/root/{0}.root".format(target))
    print target

    # fill the index of the HistLists (eta bin) with the set of histograms to overlay
    for j in xrange(0,len(etabins_string)):
      tempHist = myfile.Get("Resolution_{0}_eta_{1}".format(target.split("Jets")[1],etabins_string[j]))
      tempHist.SetDirectory(0)
      resolutionHistList[j].append(tempHist)

      tempHist = myfile.Get("Response_{0}_eta_{1}".format(target.split("Jets")[1],etabins_string[j]))
      tempHist.SetDirectory(0)
      responseHistList[j].append(tempHist)
    myfile.Close()

  #########################
  # done getting hists - printing histograms

  # Resolution
  ResolutionCanvas = ROOT.TCanvas("resolution", "resolution", 174, 52, 700, 500)

  for bins in xrange(0,len(resolutionHistList)):
    leg = ROOT.TLegend(0.6,0.75,0.9,0.9 )
    resolutionHistList[bins][0].Draw()

    resolutionHistList[bins][0].SetLineColor(kBlack)
    resolutionHistList[bins][0].SetMarkerColor(kBlack)

    resolutionHistList[bins][0].GetYaxis().SetRangeUser(0.0, 0.6)
    leg.AddEntry(resolutionHistList[bins][0], Corrections[0],"lp")

    for k in xrange(1,len(resolutionHistList[bins])):
      resolutionHistList[bins][k].Draw("same")
      resolutionHistList[bins][k].SetLineColor(Colours[k])
      resolutionHistList[bins][k].SetMarkerColor(Colours[k])
      resolutionHistList[bins][k].SetMarkerStyle(Markers[k])

      leg.AddEntry(resolutionHistList[bins][k], Corrections[k],"lp")
      ResolutionCanvas.Update()

    leg.Draw()
    ResolutionCanvas.SaveAs("resolution_plots/overlay/resolution_allcorr_{0}_{1}.eps".format(Rhos[i], etabins_string[bins] ))
    ResolutionCanvas.Delete()

  # Response
  ResponseCanvas = ROOT.TCanvas("response", "response", 174, 52, 700, 500)
  
  for bins in xrange(0,len(responseHistList)):
    leg = ROOT.TLegend(0.6,0.75,0.9,0.9 )
    responseHistList[bins][0].Draw()

    responseHistList[bins][0].SetLineColor(kBlack)
    responseHistList[bins][0].SetMarkerColor(kBlack)

    responseHistList[bins][0].GetYaxis().SetRangeUser(0.3,1.2)
    leg.AddEntry(responseHistList[bins][0], Corrections[0],"lp")

    for k in xrange(1,len(responseHistList[bins])):
      responseHistList[bins][k].Draw("same")
      responseHistList[bins][k].SetLineColor(Colours[k])
      responseHistList[bins][k].SetMarkerColor(Colours[k])
      responseHistList[bins][k].SetMarkerStyle(Markers[k])

      leg.AddEntry(responseHistList[bins][k], Corrections[k],"lp")
      ResponseCanvas.Update()

    leg.Draw()
    ResponseCanvas.SaveAs("resolution_plots/overlay/response_allcorr_{0}_{1}.eps".format(Rhos[i], etabins_string[bins]))
    ResponseCanvas.Delete()
