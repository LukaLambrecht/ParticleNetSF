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

  TString brX; int binsX; float minX, maxX;
  TString brY; int binsY; float minY, maxY;
  TString score_def;
  TString score_category; 
  
  std::vector<TString> ptnames; 
  std::vector<double>  ptmin; 
  std::vector<double>  ptmax;
  std::vector<TString> processes;
  std::vector<TString> process_names;
  std::vector<TString> processes_splitting;
  std::map<TString, std::vector<TString>> processes_grouping;
  std::vector<TString> syst;
  std::vector<TString> processes_in;

  std::map<TString, TString> legend_labels;
  std::map<TString, int> colors;

  TString path_2016preVFP;
  TString path_2016postVFP;
  TString path_2017;
  TString path_2018;
  TString path_2022preEE;
  TString path_2022postEE;
  TString path_2023preBPix;
  TString path_2023postBPix;
  TString path_2024;

  TString jet_radius;

  void configuration(TString sample) {
    // Helper function to define configuration parameters for a given sample.
    // Note: this is probably a relic from earlier development (?),
    //       and in practice only the sample name 'tt1l' is used, for all samples.

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
    //path_2024 = "/eos/user/z/zima/HadronicVH/MC1_loose_ULNanoV9_MassRegression_ak8_muon_2024/";
    //path_2024 = "/eos/user/z/zima/HadronicVH/MC_ULNanoV9_MassRegression_ak8_muon_2024/";
    //path_2024 = "/eos/user/z/zima/HadronicVH/ttCR_test_ak8_muon_2024/";
    path_2024 = "/eos/user/b/bribeiro/HadronicVH/treesScouting_Jun2_2024_ttCR/";
    // set jet radius
    // note: this is only used to make the split into top-merged, W-merged and non-merged at gen-level;
    //       the radius of the reco-level jets is already set in the input files.
    jet_radius = "0.8";
    
    // set input variables
    TString jet_prefix = "ak8_";
    brX = jet_prefix + "ParT_resonanceMass"; 
    //brX = jet_prefix + "sdmass"; 
    brY = jet_prefix + "pt";

    // clear any previously stored variables    
    processes.clear();
    process_names.clear();
    processes_in.clear();
    syst.clear();
    
    // define which processes to use for the 2D templates
    // note: processes should contain the file name
    //       (e.g. "ttbar-powheg" -> will look for ttbar-powheg.root).
    // note: process_names should contain the given name of the process,
    //       that will be used in all further steps.
    // note: with the current set of input files, the "qcd-mg" samples
    //       are missing from all years but 2016 (preVFP and postVFP),
    //       and the "ttv" samples are missing in 2016 and 2017,
    //       so disable them for now (for all years, for consistency). 
    processes.push_back("ttbar"); process_names.push_back("tt"); 
    processes.push_back("singletop");    process_names.push_back("st");
    //processes.push_back("ttv");          process_names.push_back("ttv");
    
    // don't have "w" for now
    processes.push_back("w2lnu");            process_names.push_back("wll");
    //processes.push_back("w2qq");            process_names.push_back("wll");
    
    processes.push_back("diboson");      process_names.push_back("vv");
    processes.push_back("qcd1");   process_names.push_back("qcd");
    processes.push_back("qcd2");   process_names.push_back("qcd");
    processes.push_back("qcd3");   process_names.push_back("qcd");
    processes.push_back("qcd4");   process_names.push_back("qcd");
    processes.push_back("qcd5");   process_names.push_back("qcd");
    processes.push_back("z2qq");       process_names.push_back("z2qq");
    
    // define which processes to split into p3, p2 and p1
    // note: must be subset of the process names defined above.
    processes_splitting.push_back("tt");
    processes_splitting.push_back("st");
    //processes_splitting.push_back("ttv");
    
    // define which processes to use for the 1D templates
    // note: the input process names must correspond to (a subset of)
    //       the process_names from the previous step,
    //       but with the signal processes (usually tt, st, ttv) split in categories.
    processes_grouping["tp3"] = {"tt_p3", "st_p3"}; // also ttv_p3 if the sample is used.
    processes_grouping["tp2"] = {"tt_p2", "st_p2"}; // also ttv_p2 if the sample is used.
    processes_grouping["tp1"] = {"tt_p1", "st_p1"}; // also ttv_p1 if the sample is used.
    processes_grouping["other"] = {"wll", "vv", "qcd", "z2qq"}; // also qcd if the sample is used.
    //processes_grouping["other"] = {"vv"};

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
    syst.push_back("lhepdf");
    
    } // end of if sample == 'tt1l'
    
    // score settings
    // note: score_def should be the branch name of the score to use
    // note: score_category should be "top" (tp3 is the main process of interest in the fit)
    //       or "w" (tp2 is the main process of interest in the fit)
    // note: historically, ParticleNet was used for the score,
    //       in particular the branch ak8_ParticleNetMD_XccVsQCD;
    //       now switching to ParticleTransformer.
    score_def = "ak8_ParT_HccVsQCD";
    score_category = "w"; 

    // binning
    // note: make sure the bin parameters are defined in a way
    //       to make convenient bin edges, e.g. multiples of 5 GeV,
    //       so convenient categories (e.g. pt 300 - 400) can be correctly defined.
    // note: x-axis is for the mass, y-axis is for the pt.
    binsX = 30; minX = 50;  maxX = 200.;
    binsY = 40; minY = 200; maxY = 800.; 
 
    // settings for pt bins
    // note: the pt bins defined here should not be contradicting
    //       with the ones from the previous step;
    //       for example: a pt bin of 300-325 GeV is not possible if the bins
    //       were defined with binsY = 4; minY = 200; maxY = 400;
    //merge the first two pT bins
    
    //ptnames.push_back("pt300to400"); ptmin.push_back(300.); ptmax.push_back(400.);
    ptnames.push_back("pt400to500"); ptmin.push_back(400.); ptmax.push_back(500.);
    ptnames.push_back("pt500to800"); ptmin.push_back(500.); ptmax.push_back(800.);
    ptnames.push_back("pt200to800"); ptmin.push_back(200.); ptmax.push_back(800.);
    ptnames.push_back("pt200to400"); ptmin.push_back(200.); ptmax.push_back(400.); 
    
    //ptnames.push_back("pt200to300"); ptmin.push_back(200.); ptmax.push_back(300.);
    //ptnames.push_back("pt300to400"); ptmin.push_back(300.); ptmax.push_back(400.);  
    //ptnames.push_back("pt400to500"); ptmin.push_back(400.); ptmax.push_back(500.);
    //ptnames.push_back("pt500to600"); ptmin.push_back(500.); ptmax.push_back(600.);
    //ptnames.push_back("pt600to700"); ptmin.push_back(600.); ptmax.push_back(700.);
    //ptnames.push_back("pt700to800"); ptmin.push_back(700.); ptmax.push_back(800.);
    //ptnames.push_back("pt200to800"); ptmin.push_back(200.); ptmax.push_back(800.);
    
    legend_labels["tp3"] = "Top-merged";
    legend_labels["tp2"] = "W-merged";
    legend_labels["tp1"] = "Non-merged";
    legend_labels["other"] = "Other";

    colors["tp3"] = 4;
    colors["tp2"] = 7;
    colors["tp1"] = 595;
    colors["other"] = 6;
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
