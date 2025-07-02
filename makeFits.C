// Perform the fits using Combine


// imports
#include <bits/stdc++.h>
#include <string>
#include <unistd.h>
#include "configuration.h"


// declare helper functions
void makeOneFit(std::string era, std::string category, std::string wpmin, std::string wpmax, std::string name);


// main function
void makeFits(std::string era, std::string category, std::string wpmin, std::string wpmax, TString sample) {

  // make the configuration for this sample
  conf::configuration(sample);
  std::vector<TString> name  = conf::name;
  
  // define what to do based on sample
  // note: this is a relic from earlier development (?),
  //       in practice the sample is always 'tt1l'. 
  if(sample=="tt1l"){
      for (int i0=0; i0<name.size(); ++i0){
          std::cout << "Running makeFits on pt bin " << name[i0] << std::endl; 
	      makeOneFit(era, category, wpmin, wpmax, (std::string)name[i0]);
	  } 
  }
  else{
      TString msg = "Sample " + sample + " not recognized.";
      throw std::runtime_error(msg);
  }
}


void makeOneFit(std::string era, std::string category, std::string wpmin, std::string wpmax, std::string name) {

  // check category
  std::string pos;
  if( category=="top" ){ pos = "tp3,tp2,tp1,other"; }
  else if( category=="w" ){ pos = "tp2,tp3,tp1,other"; }
  else{
      std::string msg = "ERROR in makeOneFit: category " + category + " not recognized.";
      throw std::runtime_error(msg);
  }

  // make text2workspace command
  std::string txt2workspace = (std::string)"text2workspace.py -m 125 -P HiggsAnalysis.CombinedLimit.TagAndProbeExtended:tagAndProbe"
      + " templates1D/fitdir/datacard_particlenet_tt1l_" + wpmin + "to" + wpmax + "_" + era + "_" + name + ".txt"
      + " --PO categories=" + pos;
 
  // make other commands
  std::string algolabel = "particlenet";
  std::string multidimfit       = "combine -M MultiDimFit -m 125 templates1D/fitdir/datacard_"+algolabel+"_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_"+name+".root  --algo=singles --robustFit=1 --cminDefaultMinimizerTolerance 5.";
  
  std::string fitdiagnostics    = "combine -M FitDiagnostics -m 125 templates1D/fitdir/datacard_"+algolabel+"_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_"+name+".root --saveShapes --saveWithUncertainties --robustFit=1 --cminDefaultMinimizerTolerance 5.";
  
  std::string mvmultidimfitfile = "mv higgsCombineTest.MultiDimFit.mH125.root templates1D/fitdir/multidimfit_"+algolabel+"_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_"+name+".root";
  std::string mvfitdiagnostics  = "mv fitDiagnosticsTest.root templates1D/fitdir/fitdiagnostics_"+algolabel+"_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_"+name+".root";
  
  // type conversion of combine commands
  const char *command_txt2workspace     = txt2workspace.c_str();
  const char *command_multidimfit       = multidimfit.c_str();
  const char *command_fitdiagnostics    = fitdiagnostics.c_str();
  const char *command_mvmultidimfitfile = mvmultidimfitfile.c_str(); 
  const char *command_mvfitdiagnostics  = mvfitdiagnostics.c_str();

  // run combine commands
  system(command_txt2workspace);
  system(command_multidimfit);
  system(command_fitdiagnostics);
  system(command_mvmultidimfitfile);
  system(command_mvfitdiagnostics);
  
  // make impacts commands
  std::string impacts_1 = "combineTool.py -M Impacts -d templates1D/fitdir/datacard_"+algolabel+"_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_"+name+".root -m 125 --doInitialFit --robustFit 1 --exclude 'rgx{prop.*}'";
  std::string impacts_2 = "combineTool.py -M Impacts -d templates1D/fitdir/datacard_"+algolabel+"_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_"+name+".root -m 125 --robustFit 1 --doFits --parallel 60 --exclude 'rgx{prop.*}'";
  std::string impacts_3 = "combineTool.py -M Impacts -d templates1D/fitdir/datacard_"+algolabel+"_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_"+name+".root -m 125 -o impacts.json --exclude 'rgx{prop.*}'";
  std::string impacts_4 = "plotImpacts.py -i impacts.json -o impacts";
  std::string impacts_5 = "mv impacts.pdf templates1D/fitdir/impacts_"+algolabel+"_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_"+name+".pdf";
  std::string impacts_6 = "mv impacts.json templates1D/fitdir/impacts_"+algolabel+"_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_"+name+".json";
  std::string impacts_7 = "mv combine_logger.out templates1D/fitdir/combine_logger_"+algolabel+"_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_"+name+".out";
  std::string impacts_8 = "python3 $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/data/tutorials/longexercise/diffNuisances.py templates1D/fitdir/fitdiagnostics_"+algolabel+"_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_"+name+".root -A -g plots.root";
  std::string impacts_9 = "mv plots.root templates1D/fitdir/impact_plots_"+algolabel+"_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_"+name+".root";
 
  // type conversion for impacts commands
  const char *command_impacts1 = impacts_1.c_str();
  const char *command_impacts2 = impacts_2.c_str();
  const char *command_impacts3 = impacts_3.c_str();
  const char *command_impacts4 = impacts_4.c_str();
  const char *command_impacts5 = impacts_5.c_str();
  const char *command_impacts6 = impacts_6.c_str();
  const char *command_impacts7 = impacts_7.c_str();
  const char *command_impacts8 = impacts_8.c_str();
  const char *command_impacts9 = impacts_9.c_str();

  // run impacts commands
  // disable for speed (maybe later add as an argument)
  /*system(command_impacts1);
  system(command_impacts2);
  system(command_impacts3);
  system(command_impacts4);
  system(command_impacts5);
  system(command_impacts6);
  system(command_impacts7);
  system(command_impacts8);
  system(command_impacts9);
  system("rm higgsCombine*.root");*/
  
  // post fit plots
  system("root -l -q \'make1DTemplates.C(\"'"+(TString)era+"'\",\"tt1l\",\"'"+(TString)wpmin+"'\",\"'"+(TString)wpmax+"'\",true,\"pass\")\'");
  system("root -l -q \'make1DTemplates.C(\"'"+(TString)era+"'\",\"tt1l\",\"'"+(TString)wpmin+"'\",\"'"+(TString)wpmax+"'\",true,\"fail\")\'");
  
}
