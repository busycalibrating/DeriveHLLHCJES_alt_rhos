import ctypes, glob, sys, os, subprocess #,numpy
from optparse import OptionParser

import ROOT
from ROOT import *

#ROOT.gROOT.LoadMacro("/home/ajhorton/atlasstyle-00-03-04/AtlasStyle.C")
#ROOT.SetAtlasStyle()

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
#ptbins = numpy.arange(0, 52, 2)

ptbins = [5,10,20,30,45,60,80,110,160]

Rhos = (localC.GetValue("Tag", "FINE")).split(" ")
JetRhos = (localC.GetValue("JetRho"," ")).split(" ")

###########################################################################
# Make array of all of the histograms to look at

TreeList = []
#TreeListLocal = []

for algo in JetAlgos:
  TreeList.append("{0}Uncalibrated".format(algo))
  for rho in Rhos:
    for correction in Corrections:
      TreeList.append("{0}{1}{2}".format(algo,rho,correction))
  for jetrho in JetRhos:
    for correction in Corrections:
      TreeList.append("{0}{1}{2}".format(algo,jetrho,correction))

f = open('treelist.txt','w')
for tree in TreeList:
  f.write(tree + '\n')
f.close()

