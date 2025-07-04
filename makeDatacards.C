// Make datacards


// imports
#include <string>
#include "configuration.h"


// declare helper functions
void makeOneDatacard(TString workdir, TString wpmin, TString wpmax, TString name, TString sample, TString era);

// main function
void makeDatacards(TString era, TString sample, TString wpmin, TString wpmax, TString outputDir){

  // make the configuration for this sample
  conf::configuration(sample);
  std::vector<TString> ptnames  = conf::ptnames;

  // define what to do based on sample
  // note: this is a relic from earlier development (?),
  //       in practice the sample is always 'tt1l'.
  if(sample=="tt1l"){
      outputDir += "/templates1D";
      for(TString ptname: ptnames){
          std::cout << "Running makeDatacards on pt bin " << ptname << std::endl;
	      makeOneDatacard(outputDir, wpmin, wpmax, ptname, sample, era);
	  }
  }
  else{
      TString msg = "Sample " + sample + " not recognized.";
      throw std::runtime_error(msg);
  }
}


void makeOneDatacard(TString workdir, TString wpmin, TString wpmax, TString name, TString sample, TString era) {
  // Make a single datacard
  
  // make the configuration for this sample
  conf::configuration(sample);

  // get the shape systematics to include from the configuration
  // note: normalization systematics are not included in this list;
  //       to modify them, just edit the code below (for now).
  // note: this is not yet used in practice (for now),
  //       as it would be not super trivial to define which systematics
  //       to include for which process.
  //       so for now just edit the code below to make changes.
  std::vector<TString> systematics = conf::syst;

  // set name and label
  TString algolabel = "particlenet";
  
  // make output directory
  const int dir_err = system("mkdir -p " + workdir + "/fitdir/");
  if (-1 == dir_err) { printf("Error creating directory!n"); exit(1); }

  // initialize the datacard
  std::ofstream out(workdir + "/fitdir/datacard_"+algolabel+"_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_"+name+".txt");
  std::streambuf *coutbuf = std::cout.rdbuf();
  std::cout.rdbuf(out.rdbuf());

  // get the histograms for pass 
  TFile *f_p = TFile::Open(workdir+"/"+algolabel+"_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_"+name+"_templates_p.root","READONLY");
  TH1D  *h_obs_p    = (TH1D*)f_p->Get("data_obs"); 
  TH1D  *h_top_p1_p = (TH1D*)f_p->Get("tp1");
  TH1D  *h_top_p2_p = (TH1D*)f_p->Get("tp2");
  TH1D  *h_top_p3_p = (TH1D*)f_p->Get("tp3");
  TH1D  *h_other_p  = (TH1D*)f_p->Get("other");
  
  // get the histograms for fail
  TFile *f_f = TFile::Open(workdir+"/"+algolabel+"_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_"+name+"_templates_f.root","READONLY");
  TH1D  *h_obs_f    = (TH1D*)f_f->Get("data_obs");   
  TH1D  *h_top_p1_f = (TH1D*)f_f->Get("tp1");
  TH1D  *h_top_p2_f = (TH1D*)f_f->Get("tp2");
  TH1D  *h_top_p3_f = (TH1D*)f_f->Get("tp3");
  TH1D  *h_other_f  = (TH1D*)f_f->Get("other");
  
  std::cout << "imax 2  number of channels\n";
  std::cout << "jmax 3  number of processes -1\n";
  std::cout << "kmax *  number of nuisance parameters (sources of systematical uncertainties)\n";
  std::cout << "------------\n";

  std::cout << "shapes  *  pass  " << workdir << "/"+algolabel+"_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_" << name << "_templates_p.root  $PROCESS $PROCESS_$SYSTEMATIC\n";
  std::cout << "shapes  *  fail  " << workdir << "/"+algolabel+"_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_" << name << "_templates_f.root  $PROCESS $PROCESS_$SYSTEMATIC\n";
  std::cout << "------------\n";


  std::cout << "bin             pass fail    \n";
  std::cout << "observation      " << h_obs_p->Integral(1,h_obs_p->GetNbinsX()) << " " << h_obs_f->Integral(1,h_obs_f->GetNbinsX()) << "\n";
  std::cout << "------------\n";
  std::cout << "# now we list the expected events for signal and all backgrounds in that bin\n";
  std::cout << "# the second 'process' line must have a positive number for backgrounds, and 0 for signal\n";
  std::cout << "# then we list the independent sources of uncertainties, and give their effect (syst. error)\n";
  std::cout << "# on each process and bin\n";
  std::cout << "bin             pass pass pass pass     fail fail fail fail  \n";
  std::cout << "process         tp3  tp2  tp1  other    tp3  tp2  tp1  other  \n";  
  std::cout << "process         4 3 6 -7      4 3 6 -7   \n";
  std::cout << "rate            -1 -1 -1 -1     -1 -1 -1 -1  \n"; 
  std::cout << "------------\n";

  // todo: find out why these exact numbers (4, 3, 6, -7) were chosen for the process list.
  //       presumably the numbers don't matter as the scaling for each category is defined
  //       in the custom physics model (TagAndProbeExtended.py), but to double check.


  std::cout << "lumi            lnN      1.025 1.025 1.025 1.025     1.025 1.025 1.025 1.025 \n";
  std::cout << "\n";
    
  std::cout << "tp3_xsec     lnN      1.05 - - -    1.05 - - - \n";
  std::cout << "tp2_xsec     lnN      - 1.05 - -    - 1.05 - - \n";
  std::cout << "tp1_xsec     lnN      - - 1.05 -    - - 1.05 - \n";
  std::cout << "other_xsec   lnU      - - - 2.00    - - - 2.00 \n";
  
  std::cout << "\n";

  /*std::cout << "tp3jms       shapeU    1 - - -     1 - - - \n";  
  std::cout << "tp2jms       shapeU    - 1 - -     - 1 - - \n";
  std::cout << "tp1jms       shapeU    - - 1 -     - - 1 - \n";
  std::cout << "otherjms     shapeU    - - - 1     - - - 1 \n";

  std::cout << "tp3jmr       shapeU    1 - - -     1 - - - \n";  
  std::cout << "tp2jmr       shapeU    - 1 - -     - 1 - - \n";
  std::cout << "tp1jmr       shapeU    - - 1 -     - - 1 - \n";
  std::cout << "otherjmr     shapeU    - - - 1     - - - 1 \n";
  
  std::cout << "pu          shape    1 1 1 1     1 1 1 1 \n";
  std::cout << "jes         shape    1 1 1 1     1 1 1 1 \n";
  std::cout << "jer         shape    1 1 1 1     1 1 1 1 \n";
  std::cout << "met         shape    1 1 1 1     1 1 1 1 \n";
  std::cout << "lhescalemuf shape    1 1 1 1     1 1 1 1 \n";
  std::cout << "lhescalemur shape    1 1 1 1     1 1 1 1 \n";
  //std::cout << "lhepdf      shape    1 1 1 1     1 1 1 1 \n";*/
    
  std::cout << "norm_top    rateParam    pass    tp3      1   [0.,10.]\n";
  std::cout << "norm_top    rateParam    fail    tp3      1   [0.,10.]\n";
  std::cout << "norm_top    rateParam    pass    tp2      1   [0.,10.]\n";
  std::cout << "norm_top    rateParam    fail    tp2      1   [0.,10.]\n";
  std::cout << "norm_top    rateParam    pass    tp1      1   [0.,10.]\n";
  std::cout << "norm_top    rateParam    fail    tp1      1   [0.,10.]\n";
  std::cout << "\n";

  std::cout << "*  autoMCStats  0\n";
}
