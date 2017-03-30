#!/bin/sh

pyCalibFile=$1
#echo $pyCalibFile
calibs=$2

#nbins=$3
#echo $calibs
jetRs="4"
corrs="energy eta"

for calib in $calibs; do
    for jetR in $jetRs; do
	for corr in $corrs; do
	    #echo "AntiKt${jetR}_${calib}_${corr}"
	    echo ; echo
	    echo "  #########"
	    echo "  # ${calib}+JES calibration factors for"
	    echo "  # jet ${corr} correction"
	    echo "  #"
	    pre=JES
	    [[ $corr = eta ]] && pre=EtaCorr
	    
            grep -A90 "${corr}CorrDict\[ 'AntiKt" $pyCalibFile | tail -n +2> tmp_calib_${corr}
	    #grep -A90 "${corr}CorrDict\[ 'AntiKt" $pyCalibFile > tmp_calib_${corr}
	    for i in {0..31} ; do
		c1=`head -$((i+1)) tmp_calib_${corr} | tail -1 | awk '{print $2}' | cut -f1 -d','`
		c2=`head -$((i+1)) tmp_calib_${corr} | tail -1 | awk '{print $3}' | cut -f1 -d','`
		c3=`head -$((i+1)) tmp_calib_${corr} | tail -1 | awk '{print $4}' | cut -f1 -d','`
		c4=`head -$((i+1)) tmp_calib_${corr} | tail -1 | awk '{print $5}' | cut -f1 -d','`
		c5=`head -$((i+1)) tmp_calib_${corr} | tail -1 | awk '{print $6}' | cut -f1 -d','`
		c6=`head -$((i+1)) tmp_calib_${corr} | tail -1 | awk '{print $7}' | cut -f1 -d','`
		c7=`head -$((i+1)) tmp_calib_${corr} | tail -1 | awk '{print $8}' | cut -f1 -d','`
		c8=`head -$((i+1)) tmp_calib_${corr} | tail -1 | awk '{print $9}' | cut -f1 -d','`
		c9=`head -$((i+1)) tmp_calib_${corr} | tail -1 | awk '{print $10}' | cut -f1 -d','`
		key="${pre}.${calib}_Bin${i}:"
		#[[ $calib = LC ]] && key="${pre}.AntiKt${jetR}LCTopoTrimmedPtFrac5SmallR20_Bin${i}:"
	        #echo $i
		printf "%-30s %12.4e %12.4e %12.4e %12.4e %12.4e %12.4e %12.4e %12.4e %12.4e\n" "$key" $c1 $c2 $c3 $c4 $c5 $c6 $c7 $c8 $c9
	    done
#	    grep $calib $pyCalibFile | grep AntiKt${jetR} | grep $corr | grep Topo | grep -v Tower
	done
    done
done

for calib in $calibs; do
    for jetR in $jetRs; do
	echo ; echo
	echo "  #########"
	echo "  # ${calib} Starting energy for the JES freezing"
	echo "  #"
	for i in {0..1} ; do
	    
	    key="EmaxJES.${calib}_Bin${i}:"
	    printf "%-30s 1500\n" "$key"
	done
	for i in {2..3} ; do
	    
	    key="EmaxJES.${calib}_Bin${i}:"
	    printf "%-30s 1000\n" "$key"
	done
	for i in {4..5} ; do
	    
	    key="EmaxJES.${calib}_Bin${i}:"
	    printf "%-30s 600\n" "$key"
	done
	for i in {6..9} ; do
	    
	    key="EmaxJES.${calib}_Bin${i}:"
	    printf "%-30s 300\n" "$key"
	done
	for i in {10..21} ; do
    
	    key="EmaxJES.${calib}_Bin${i}:"
	    printf "%-30s 200\n" "$key"
	done
	for i in {22..25} ; do
	    
	    key="EmaxJES.${calib}_Bin${i}:"
	    printf "%-30s 300\n" "$key"
	done
	for i in {26..27} ; do
	    
	    key="EmaxJES.${calib}_Bin${i}:"
	    printf "%-30s 600\n" "$key"
	done
	for i in {28..29} ; do
	    
	    key="EmaxJES.${calib}_Bin${i}:"
	    printf "%-30s 1000\n" "$key"
	done
	for i in {30..31} ; do
	    
	    key="EmaxJES.${calib}_Bin${i}:"
	    printf "%-30s 1500\n" "$key"
	done
    done
done
