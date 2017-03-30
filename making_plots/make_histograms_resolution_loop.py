import ctypes, glob, sys, os, subprocess #, numpy
from optparse import OptionParser

import ROOT
from ROOT import *
from array import array

ROOT.gROOT.SetBatch(True)
ROOT.gROOT.Macro('$ROOTCOREDIR/scripts/load_packages.C')
ROOT.gStyle.SetOptStat(0)
fitter = ROOT.JES_BalanceFitter(1.6)


#ROOT.gROOT.LoadMacro("/home/ajhorton/atlasstyle-00-03-04/AtlasStyle.C")
#ROOT.SetAtlasStyle()


###########################################################################
# Get input tree

parser = OptionParser()
parser.add_option('--InputTree', help = "Specify which tree we are printing things for", default = "" )
options, args = parser.parse_args()

tree = options.InputTree



###########################################################################
# Declare files and parameters to use

LocalConfig = "/cluster/warehouse/ddobre/thesis/DeriveHLLHCJES_test2/DeriveHLLHCJES/macros/Local.config"
InputFileLocation = "/cluster/warehouse/ddobre/thesis/pythia_whole_08.03.17/calibrated/C0_r7725_r7676_p2794.root"
JetAlgos = ["AntiKt4EMTopoJets"]
Corrections = ["Area", "AreaResidual", "AreaResidualConst"]



###########################################################################
# Extract names of calculated rhos

localC = ROOT.TEnv(LocalConfig)

# etabins =  numpy.array( map( float, (localC.GetValue("ResidualAbsEtaBins","0 2 4 6")).split(" ") ) )
etabins =  [0, 1.2, 2.1]

# ptbins = numpy.arange(0, 52, 2)
ptbins = [5.0,10.0,20.0,30.0,45.0,60.0,80.0,110.0,160.0]

Rhos = (localC.GetValue("Tag", "FINE")).split(" ")
JetRhos = (localC.GetValue("JetRho"," ")).split(" ")



###########################################################################
# Make array of all of the histograms to look at

TreeList = []

for algo in JetAlgos:
  TreeList.append("{0}Uncalibrated".format(algo))
  for rho in Rhos:
    for correction in Corrections:
      TreeList.append("{0}{1}{2}".format(algo,rho,correction))
  for jetrho in JetRhos:
    for correction in Corrections:
      TreeList.append("{0}{1}{2}".format(algo,jetrho,correction))

# initialize input and output root files
outputfile = ROOT.TFile("resolution_plots/root/{0}.root".format(tree), "recreate")
inputfile = ROOT.TFile(InputFileLocation)
# plotTree = ROOT.TTree("plots", "plots")
# plotsTDir = outputfile.mkdir("plots")

# initialize canvas
canvas = ROOT.TCanvas("hist", "hist", 174, 52, 700, 500)



###########################################################################
# Loop over all of the trees in the ROOT file
print "Loop: input tree = " + tree

# Make temporary lists storing hist info and its name
HistList = []
HistListNames = []

# Fill these lists with associated pt and eta ranges
print "--Creating hists with associated pt and eta ranges:"
etabins_string = [] # Use this to simplify splitting the stats by eta ranges later

for i in xrange(0,len(etabins)-1):
  etabins_string.append("{0}_{1}".format(etabins[i],etabins[i+1]))

  for j in xrange(0,len(ptbins)-1):
    tempname = "{0}_pt_{1}_{2}_eta_{3}_{4}".format(tree,ptbins[j],ptbins[j+1],etabins[i],etabins[i+1])
    print "--> " + tempname
    HistListNames.append(tempname)
    HistList.append( ROOT.TH1D("{0}".format(tempname), "{0}".format(tempname), 100, 0, 2) )



###########################################################################
# Get events from specificed tree
print "Loop: getting events from specified tree"

mychain = gDirectory.Get("{0};1".format(tree))
entries = mychain.GetEntriesFast()

for jentry in xrange( entries ):
  ientry = mychain.LoadTree( jentry )
  if ientry < 0:
    break

  nb = mychain.GetEntry( jentry )
  if nb <= 0:
    continue

  if jentry%20000 == 0:
    print "----> Processed {0} events".format(jentry)
  v = std.vector(float)()

  # Get pt, tpt, and eta to do calculations
  truth_pt = mychain.tpt
  truth_eta = mychain.teta
  calibrated_pt = mychain.pt

  # Ensure consistency between the obtained vectors
  if truth_pt.size() == calibrated_pt.size():
    for i in xrange(0,truth_pt.size()): # Fill the recopt/truthpt vector
      v.push_back(calibrated_pt[i]/truth_pt[i])

    # Get the cut values for eta and pt from the filename
    for hist in xrange(0,len(HistListNames)):
      etalow = float(HistListNames[hist].split("_")[-2])
      etahigh = float(HistListNames[hist].split("_")[-1])
      ptlow = float(HistListNames[hist].split("_")[-5])
      pthigh = float(HistListNames[hist].split("_")[-4])

      # Fill hists, with the appropriate cut for the selected histogram
      for i in xrange(0,truth_pt.size()):
        if truth_pt[i] < pthigh and truth_pt[i] > ptlow and truth_eta[i] < etahigh and truth_eta[i] > etalow:
          HistList[hist].Fill(v[i])



###########################################################################
# Initialize the resolution and response hists/trees
print "Loop: done filling hists. Now printing hists:"

##### This section is setting up the outputs for the stats
# myFiles = []
resolutionHists = []
responseHists = []

# resolutionTDir = outputfile.mkdir("resolution")
# responseTDir = outputfile.mkdir("response")

# resolutionTree = ROOT.TTree("resolution","resolution")
# responseTree = ROOT.TTree("response","response")

for etabin in etabins_string: # Formatting in the rho name (FINE, GlobalRho, etc..) and eta_bins
  resolutionHists.append(ROOT.TH1D("Resolution_{0}_eta_{1}".format(tree.split("Jets")[1],etabin), "Resolution_{0}_eta_{1}".format(tree.split("Jets")[1],etabin) , (len(ptbins)-1), array('d',ptbins) ) )
  responseHists.append(ROOT.TH1D("Response_{0}_eta_{1}".format(tree.split("Jets")[1],etabin), "Response_{0}_eta_{1}".format(tree.split("Jets")[1],etabin) , (len(ptbins)-1), array('d',ptbins) ) )
  # myFiles.append(open("resolution_plots/stats/{0}_eta_{1}.txt".format(tree,etabin), "a"))
####



###########################################################################
# Plot basic hists

for hist in xrange(0,len(HistListNames)):
  canvas.cd()

  # get cut values, again from the filename
  etalow = float(HistListNames[hist].split("_")[-2])
  etahigh = float(HistListNames[hist].split("_")[-1])
  ptlow = float(HistListNames[hist].split("_")[-5])
  pthigh = float(HistListNames[hist].split("_")[-4])              #                                           Jetalgo v                           eta bins v
  nametemp = (HistListNames[hist].split("_")[0]).split("Jets")[1] # Make sure that your filename has something like xxxxJets[$RhoTagAreaResidualConst]_pt_xxxx

  # Hist lables
  HistList[hist].SetTitle( "{0} : Resolution for p_{{T}} slice {1}-{2} GeV, eta slice {3}-{4}".format(nametemp,ptlow,pthigh,etalow,etahigh) )
  HistList[hist].GetYaxis().SetTitle("Number of Events")
  HistList[hist].GetXaxis().SetTitle("p_{T}^{reco}/p_{T}^{truth}")

  # Draw hist, and add fit
  HistList[hist].Draw()
  fitter.FitAndDraw(HistList[hist])

  canvas.SaveAs("resolution_plots/base_plots/{0}.eps".format(HistListNames[hist]))
  outputfile.cd()
  HistList[hist].Write()
  print "-->Saved: " + HistListNames[hist]



###########################################################################
# Filling resolution stuff below:

  # Get fit parameters (mean, width, errors)
  mean_temp = fitter.GetMean()
  mean_error_temp = fitter.GetMeanError()
  sigma_temp = fitter.GetSigma()
  sigma_error_temp = fitter.GetSigmaError()
  # Get hist bin:
  thisbin = -1
  for i in xrange(0,len(ptbins)-1):
    if ptbins[i] == ptlow:
      thisbin = i

  # Fill associated bin with relevant response and resolution stats
  if thisbin != -1:
    for i in xrange(0,len(etabins_string)):
      if etabins_string[i] == HistListNames[hist].split("eta_")[1]:
        resolutionHists[i].SetBinContent( thisbin , sigma_temp/mean_temp )
        resolutionHists[i].SetBinError( thisbin , sigma_error_temp/mean_temp )
        responseHists[i].SetBinContent( thisbin , mean_temp )
        responseHists[i].SetBinError( thisbin , mean_error_temp )
        # myFiles[i].write(tree.split("Jets")[1] + "\t{0}_{1}\t{2}\t{3}\n".format(ptlow, pthigh, mean_temp, error_temp))



###########################################################################
# Print off the resolution hists

for i in xrange(0,len(etabins_string)):
  canvas.cd()
  resolutionHists[i].SetTitle( "{0} : Resolution for eta slice {1}".format(tree.split("Jets")[1],etabins_string[i]) )
  resolutionHists[i].GetYaxis().SetTitle(" #sigma_{p_{T}^{reco}/p_{T}^{truth}}/<p_{T}^{reco}/p_{T}^{truth}>")
  resolutionHists[i].GetXaxis().SetTitle(" p_{T}^{truth} (GeV) ")
  resolutionHists[i].Draw()
  canvas.SaveAs( "resolution_plots/resolution/{0}_eta_{1}.eps".format(tree.split("Jets")[1], etabins_string[i]) )

  outputfile.cd()
  resolutionHists[i].Write()

  canvas.cd()
  responseHists[i].SetTitle( "{0} : Response for eta slice {1}".format(tree.split("Jets")[1],etabins_string[i]) )
  responseHists[i].GetYaxis().SetTitle(" <p_{T}^{reco}/p_{T}^{truth}>")
  responseHists[i].GetXaxis().SetTitle(" p_{T}^{truth} (GeV) ")
  responseHists[i].Draw()
  canvas.SaveAs( "resolution_plots/response/{0}_eta_{1}.eps".format(tree.split("Jets")[1], etabins_string[i]) )
  outputfile.cd()
  responseHists[i].Write()



###########################################################################
# Done - wrap up

print "Done loop. Proceeding to next tree..."
print "--------------------DONE--------------------"
