#ifndef CONFIGURATION_INCLUDE
#define CONFIGURATION_INCLUDE

#include <vector>
#include <string>
#include <unistd.h>
#include <iostream>
#include <bits/stdc++.h>
#include <unistd.h>
#include "TString.h"

namespace conf {

  struct process {
    
    TString name;
    TString legend_name;
    int     color;
    
  } qcd, tp2, tp3, tp1, other;
  
  TString brX; int binsX; float minX, maxX;
  TString brY; int binsY; float minY, maxY;
  TString algo;
  TString score_def; 
  TString category;
  
  std::vector<TString> name; 
  std::vector<double>  ptmin; 
  std::vector<double>  ptmax;
  std::vector<TString> processes;
  std::vector<TString> process_names;
  std::vector<TString> syst;
  std::vector<TString> processes_in;

  TString path_2016preVFP;
  TString path_2016postVFP;
  TString path_2017;
  TString path_2018;
  TString path_2022preEE;
  TString path_2022postEE;
  TString path_2023preBPix;
  TString path_2023postBPix;

  TString jetCone;

  void configuration(TString sample) {
    // Helper function to define configuration parameters for a given sample.
    // Note: this is probably a relic from earlier development (?),
    //       and in practice only the sample name 'tt1l' is used, for all samples.

    TString jet_prefix;

    // =================== area to modify - tune ===================== //
   
    if (sample == "tt1l"){
    // note: see above, this if-statement is probably a relic from earlier development,
    // and in practice it is true for every sample.

    // set path to input files
	path_2016preVFP = "/eos/user/b/bribeiro/HadronicVH/20250321_ULNanoV9_MassRegression_ak15_muon_2016preVFP/";
	path_2016postVFP = "/eos/user/b/bribeiro/HadronicVH/20250321_ULNanoV9_MassRegression_ak15_muon_2016postVFP/";
	path_2017 = "/eos/user/b/bribeiro/HadronicVH/20250321_ULNanoV9_MassRegression_ak15_muon_2017/";
	path_2018 = "/eos/user/b/bribeiro/HadronicVH/20250321_ULNanoV9_MassRegression_ak15_muon_2018/";
	path_2022preEE = "/eos/user/b/bribeiro/HadronicVH/20250321_ULNanoV9_MassRegression_ak15_muon_2022preEE/";
    path_2022postEE = "/eos/user/b/bribeiro/HadronicVH/20250321_ULNanoV9_MassRegression_ak15_muon_2022postEE/";
    path_2023preBPix = "/eos/user/b/bribeiro/HadronicVH/20250321_ULNanoV9_MassRegression_ak15_muon_2023preBPix/";
    path_2023postBPix = "/eos/user/b/bribeiro/HadronicVH/20250321_ULNanoV9_MassRegression_ak15_muon_2023postBPix/";
	
    // set input collection
	jetCone    = "ak15";
	jet_prefix = "fj_1_";
    
    // set input variables
	brX = jet_prefix + "regressed_mass"; 
	brY = jet_prefix + "pt";

    // clear any previously stored variables	
	processes.clear();
	process_names.clear();
	processes_in.clear();
	syst.clear();
	
	// define which processes to use for the 2D templates
	processes.push_back("ttbar-powheg"); process_names.push_back("tt"); 
	processes.push_back("singletop");    process_names.push_back("st");
	processes.push_back("ttv");          process_names.push_back("ttv");
	processes.push_back("w");            process_names.push_back("wll");
	processes.push_back("diboson");      process_names.push_back("vv");
	//processes.push_back("qcd-mg");       process_names.push_back("qcd");
	// (note: qcd seems to be absent from the input files, so cannot use)
	
	// define which processes to use for the 1D templates
	// note: must correspond to (a subset of) the process_names from the previous step,
	//       but with the signal processes (tt, st, ttv) split in categories.
	processes_in.push_back("tt_p3");
    processes_in.push_back("st_p3");
    processes_in.push_back("ttv_p3");
	processes_in.push_back("tt_p2");
    processes_in.push_back("st_p2");
    processes_in.push_back("ttv_p2"); 
	processes_in.push_back("tt_p1");
    processes_in.push_back("st_p1");
    processes_in.push_back("ttv_p1");
	processes_in.push_back("wll");
    processes_in.push_back("vv");
    //processes_in.push_back("qcd");
    // (note: qcd seems to be absent from the input files, so cannot use)
	
	// list of systematic uncertainties
	syst.push_back("_"); // this is for nominal
	syst.push_back("pu"); 
	syst.push_back("jes"); 
	syst.push_back("jer");
	syst.push_back("met"); 
	syst.push_back("jms");
	syst.push_back("jmr");
	syst.push_back("lhescalemuf");
	syst.push_back("lhescalemur");
	//syst.push_back("lhepdf");
    
    } // end of if sample == 'tt1l'
    
    // ParticleNet settings
    // todo: these settings might conflict with the value provided
    //       as a command-line argumgent to runFullChain.py, leading to bugs.
    //       better to remove this setting from the configuration,
    //       and consistently use the command-line argument everywhere.
    //algo      = "particlenet"; // nominal ParticleNet
    algo      = "particlenetmd"; // mass-decorrelated ParticleNet
    score_def = "fj_1_ParticleNetMD_XccVsQCD";

    // binning
    // note: make sure the bin parameters are defined in a way
    //       to make convenient bin edges, e.g. multiples of 5 GeV,
    //       so convenient categories (e.g. pt 300 - 400) can be correctly defined.
    // note: x-axis is for the mass, y-axis is for the pt.
    binsX = 34; minX = 50;  maxX = 220.;
    binsY = 40; minY = 200; maxY = 800.; 
 
    // settings for pt bins
    // note: the pt bins defined here should not be contradicting
    //       with the ones from the previous step;
    //       for example: a pt bin of 300-325 GeV is not possible if the bins
    //       were defined with binsY = 4; minY = 200; maxY = 400;
    //name.push_back("pt200to300"); ptmin.push_back(200.); ptmax.push_back(300.);
    //name.push_back("pt300to400"); ptmin.push_back(300.); ptmax.push_back(400.);  
    //name.push_back("pt400to800"); ptmin.push_back(400.); ptmax.push_back(800.);
    name.push_back("pt200to800"); ptmin.push_back(200.); ptmax.push_back(800.);
    
    // =================== end of area to modify - tune ===================== //

    qcd.name        = "qcd";
    qcd.legend_name = "QCD";
    qcd.color       = 92;
    
    tp2.name        = "tp2";
    tp2.legend_name = "W-merged";
    tp2.color       = 7;
    
    tp3.name        = "tp3";
    tp3.legend_name = "Top-merged";
    tp3.color       = 4;
    
    tp1.name        = "tp1";
    tp1.legend_name = "Non-merged";
    tp1.color       = 595;
    
    other.name        = "other";
    other.legend_name = "Other";
    other.color       = 6;
 
  }   

  TString convertFloatToTString(float input) {
    ostringstream tmpInputStr;
    tmpInputStr << input;
    TString inputStr = tmpInputStr.str();
    return inputStr;
  }

  std::string convertTStringToString(TString input) {
    std::string output;
    output = (string)input;
    return output;
  }

} 

#endif
