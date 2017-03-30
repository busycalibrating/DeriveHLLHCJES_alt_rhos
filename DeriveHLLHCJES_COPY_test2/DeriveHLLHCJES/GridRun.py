#!/usr/bin/python
import os, sys
param=sys.argv

Pre = "DeriveHLLHCJES"
##################################
#SETTINGS
##################################
Version = "VPF1_JETM8_20170308"
FileList = Pre+"/data/mc15_2016.txt"
##################################
##################################

#WARNING CONTENTS OF THIS DIRECTORY CAN BE DELETED
dir = "/cluster/warehouse/ddobre/thesis/DeriveHLLHCJES_test2/DeriveHLLHCJES/submit/"+Version

os.system("xAODrun --output=user.${USER}."+Version+" --submitDir="+dir+"/ --grid=true --inputFileList="+FileList)
