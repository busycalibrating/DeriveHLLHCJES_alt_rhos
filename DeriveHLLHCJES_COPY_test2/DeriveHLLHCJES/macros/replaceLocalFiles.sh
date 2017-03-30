#!/bin/bash

localdir=/tmp/alkire/
eosdir=/afs/cern.ch/user/a/alkire/eos/atlas/user/a/alkire/JetWork/

version=$1
rtag=$2

mkdir ${localdir}/${version}
mkdir ${localdir}/${version}/calibrated
if [ ! -e "${localdir}${version}/${rtag}.root" ]
then 
    echo "${localdir}/${version}/${rtag}.root does not exist. Copying from eos..."
    cp ${eosdir}/${version}/${rtag}.root ${localdir}/${version}/${rtag}.root
fi

#if [ ! -e "${localdir}${version}/calibrated/CPU_${rtag}.root" ]
#then 
#    echo "${localdir}/${version}/calibrated/CPU_${rtag}.root does not exist. Copying from eos..."
#    
#    cp ${eosdir}/${version}/calibrated/CPU_${rtag}.root ${localdir}/${version}/calibrated/CPU_${rtag}.root
#    #cp ${eosdir}/${version}_round2/calibrated/CPU_${rtag}.root ${localdir}/${version}/calibrated/CPU_${rtag}.root
#fi
