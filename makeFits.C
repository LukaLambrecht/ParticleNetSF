// Perform the fits using Combine


// imports
#include <bits/stdc++.h>
#include <string>
#include <unistd.h>
#include "configuration.h"


// declare helper functions
void makeOneFit(std::string wordir, std::string era, std::string category,
        std::string wpmin, std::string wpmax, std::string binname);


// main function
void makeFits(std::string era, std::string sample,
        std::string wpmin, std::string wpmax,
        std::string outputDir="."){

  // make the configuration for this sample
  conf::configuration(sample);
  std::vector<TString> ptnames = conf::ptnames;
  TString category = conf::score_category;
  
  // define what to do based on sample
  // note: this is a relic from earlier development (?),
  //       in practice the sample is always 'tt1l'. 
  if(sample=="tt1l"){
      std::string outputSubDir = outputDir + "/templates1D";
      for(TString ptname: ptnames){
          std::cout << "Running makeFits on pt bin " << ptname << std::endl;
	      makeOneFit(outputSubDir, era, (std::string)category, wpmin, wpmax, (std::string)ptname);
	  } 
  }
  else{
      TString msg = "Sample " + sample + " not recognized.";
      throw std::runtime_error(msg);
  }
}


void makeOneFit(std::string workdir, std::string era, std::string category,
        std::string wpmin, std::string wpmax, std::string binname) {

  // check category
  std::string pos;
  if( category=="top" ){ pos = "tp3,tp2,tp1"; }
  else if( category=="w" ){ pos = "tp2,tp3,tp1"; }
  else{
      std::string msg = "ERROR in makeOneFit: category " + category + " not recognized.";
      throw std::runtime_error(msg);
  }

  // set path to datacard, workspace, and output name
  std::string fitdir = workdir + "/fitdir";
  std::string algolabel = "particlenet";
  std::string fullname = algolabel + "_tt1l_" + wpmin + "to" + wpmax + "_" + era + "_" + binname;
  std::string datacard = fitdir + "/datacard_" + fullname + ".txt";
  std::string workspace = fitdir + "/datacard_" + fullname + ".root";

  // make text2workspace command
  std::string txt2workspace = (std::string)"text2workspace.py -m 125"
      + " -P HiggsAnalysis.CombinedLimit.TagAndProbeExtended:tagAndProbe "
      + datacard + " --PO categories=" + pos;
 
  // make other commandsi
  std::string multidimfit = "combine -M MultiDimFit -m 125 -n _" + fullname + " "
      + workspace + " --algo=singles --robustFit 1 --cminDefaultMinimizerStrategy 0 --cminDefaultMinimizerTolerance 5. --setParameterRanges tp2jms=-3,3:tp2jmr=0,3:tp1jmr=0,3:tp1jms=-3,3:tp3jmr=0,3:tp3jms=-3,3";
  
  std::string fitdiagnostics = "combine -M FitDiagnostics -m 125 -n _" + fullname + " "
      + workspace + " --saveShapes --robustFit 1 --robustHesse 1 --saveWithUncertainties --cminDefaultMinimizerStrategy 0 --cminDefaultMinimizerTolerance 5. --setParameterRanges tp2jms=-3,3:tp2jmr=0,3:tp1jmr=0,3:tp1jms=-3,3:tp3jmr=0,3:tp3jms=-3,3";
  
  std::string mvmultidimfit = "mv higgsCombine_" + fullname + ".MultiDimFit.mH125.root " + fitdir + "/";
  std::string mvfitdiagnostics = "mv higgsCombine_" + fullname + ".FitDiagnostics.mH125.root " + fitdir + "/";
  std::string mvfitdiagnosticsextra  = "mv fitDiagnostics_" + fullname + ".root " + fitdir + "/";
  
  // type conversion of combine commands
  const char *command_txt2workspace     = txt2workspace.c_str();
  const char *command_multidimfit       = multidimfit.c_str();
  const char *command_fitdiagnostics    = fitdiagnostics.c_str();
  const char *command_mvmultidimfit     = mvmultidimfit.c_str(); 
  const char *command_mvfitdiagnostics  = mvfitdiagnostics.c_str();
  const char *command_mvfitdiagnosticsextra  = mvfitdiagnosticsextra.c_str();

  // run combine commands
  system(command_txt2workspace);
  system(command_multidimfit);
  system(command_fitdiagnostics);
  system(command_mvmultidimfit);
  system(command_mvfitdiagnostics);
  system(command_mvfitdiagnosticsextra);
  
  bool doImpacts = true; // maybe later add as an argument
  if(doImpacts){

  // make impacts commands
  std::string impacts_1 = "combineTool.py -M Impacts -n " + fullname + " -d " + workspace + " -m 125 --doInitialFit"
    + " --cminDefaultMinimizerStrategy 0 --robustFit 1 --cminDefaultMinimizerTolerance 5. --setParameterRanges tp2jms=-3,3:tp2jmr=0,3:tp1jmr=0,3:tp1jms=-3,3:tp3jmr=0,3:tp3jms=-3,3 --exclude 'rgx{prop.*}'";
  std::string impacts_2 = "combineTool.py -M Impacts -n " + fullname + " -d " + workspace + " -m 125 --doFits"
    + " --cminDefaultMinimizerStrategy 0 --robustFit 1 --cminDefaultMinimizerTolerance 5. --setParameterRanges tp2jms=-3,3:tp2jmr=0,3:tp1jmr=0,3:tp1jms=-3,3:tp3jmr=0,3:tp3jms=-3,3 --exclude 'rgx{prop.*}' --parallel 60";
  std::string impacts_3 = "combineTool.py -M Impacts -n " + fullname + " -d " + workspace + " -m 125 -o impacts_" + fullname + ".json"
    + " --cminDefaultMinimizerStrategy 0 --robustFit 1 --cminDefaultMinimizerTolerance 5. --setParameterRanges tp2jms=-3,3:tp2jmr=0,3:tp1jmr=0,3:tp1jms=-3,3:tp3jmr=0,3:tp3jms=-3,3 --exclude 'rgx{prop.*}'";
  std::string impacts_4 = "plotImpacts.py -i impacts_" + fullname + ".json -o impacts_" + fullname;
  std::string impacts_5 = "mv impacts_" + fullname + ".pdf " + fitdir;
  std::string impacts_6 = "mv impacts_" + fullname + ".json " + fitdir;
  std::string impacts_7 = "python3 $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/data/tutorials/longexercise/diffNuisances.py " + fitdir + "/fitDiagnostics_" + fullname + ".root -A -g plots.root";
  std::string impacts_8 = "mv plots.root " + fitdir + "/impact_plots_" + fullname + ".root";
 
  // type conversion for impacts commands
  const char *command_impacts1 = impacts_1.c_str();
  const char *command_impacts2 = impacts_2.c_str();
  const char *command_impacts3 = impacts_3.c_str();
  const char *command_impacts4 = impacts_4.c_str();
  const char *command_impacts5 = impacts_5.c_str();
  const char *command_impacts6 = impacts_6.c_str();
  const char *command_impacts7 = impacts_7.c_str();
  const char *command_impacts8 = impacts_8.c_str();

  // run impacts commands
  system(command_impacts1);
  system(command_impacts2);
  system(command_impacts3);
  system(command_impacts4);
  system(command_impacts5);
  system(command_impacts6);
  //system(command_impacts7);
  //system(command_impacts8);
  system(("mv higgsCombine_initialFit_" + fullname + "*.root "+fitdir).c_str());
  system(("mv higgsCombine_paramFit_" + fullname + "*.root "+fitdir).c_str());

  } // end of if(doImpacts)

  // move some additional files over and remove the rest
  system("rm combine_logger.out");
  
  // post fit plots
  TString cmd;
  cmd = "makePostFitPlots.C(\"'"+(TString)era+"'\",\"tt1l\",\"'"+(TString)wpmin+"'\",\"'"+(TString)wpmax+"'\",\""+(TString)binname+"\",\"pass\",\""+(TString)workdir+"\")";
  system("root -l -q \'" + cmd + "\'");
  cmd = "makePostFitPlots.C(\"'"+(TString)era+"'\",\"tt1l\",\"'"+(TString)wpmin+"'\",\"'"+(TString)wpmax+"'\",\""+(TString)binname+"\",\"fail\",\""+(TString)workdir+"\")";
  system("root -l -q \'" + cmd + "\'");
}
