echo python make_histograms_comparisons.py

###########################################################################
# Define your parameters here:

residual_directory=residual

default_rho_MuTerm=MuTermFINEVPF1_JETM8_test_20170306r7725_r7676_p2794AntiKt4EMTopoJets.root
default_rho_NPVTerm=NPVTermFINEVPF1_JETM8_test_20170306r7725_r7676_p2794AntiKt4EMTopoJets.root

histogram_to_compare=area_0_0_clone_zx0_mean_myfit_par1_myfit_par0_myfit_par0

###########################################################################
# MuTerm:

for inputfile in $residual_directory/MuTerm*.root;
do
  python make_histograms_residual_comparisons.py MuTerm $histogram_to_compare $residual_directory/$default_rho_MuTerm $inputfile &
done

###########################################################################
# NPVTerm:

for inputfile in $residual_directory/NPVTerm*.root;
do
  python make_histograms_residual_comparisons.py NPVTerm $histogram_to_compare $residual_directory/$default_rho_NPVTerm $inputfile &
done
