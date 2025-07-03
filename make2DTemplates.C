// Make 2D histograms for data and MC
//
// The 2D histograms typically contain the mass (the variable of interest for the fit) on the x-axis,
// and the pt on the y-axis in order to make 1D projections (in the next step) binned in pt.


#include "setTDRStyle.h"
#include <sstream>
#include <fstream>
#include "TTree.h"
#include "TFile.h"
#include "TH1.h"
#include "TH1D.h"
#include <vector>
#include "Rtypes.h"
#include "TColor.h"
#include "TVectorF.h"
#include <cstdlib>
#include <math.h>
#include "configuration.h"


// declare helper functions
TH2D *create2Dhisto(TString sample, TTree *tree, TString intLumi, TString cuts,
        TString branchX, int binsX, float minX, float maxX,
        TString branchY, int binsY, float minY, float maxY,
        bool useLog, TString name, bool data);

void makeTemplatesTop(TString sample, TString era, TString wpmin, TString wpmax,
        TString score="fj_1_ParticleNetMD_XbbVsQCD", TString ptmin="0", TString ptmax="9999",
        TString outputDir=".");

void makeMCHistosTop(TString name, TString path, std::vector<TString> processes, std::vector<TString> process_names,
        TString sys, TString sysType, TString wgts, std::vector<TString> cuts,
        TString brX, int binsX, float minX, float maxX,
        TString brY, int binsY, float minY, float maxY,
        TFile *f_);

void setTDRStyle();

double massScale(double mass, double scaleVal=1.05) { return scaleVal*mass; }

double massSmear(double mass, unsigned long lumi, unsigned long event, double sigma=0.1) 
{
  TRandom3 rnd((lumi << 10) + event);
  return rnd.Gaus(1, sigma)*mass; 
}


// main function
void make2DTemplates(TString era, TString sample, TString wpmin, TString wpmax,
       TString outputDir="."){

    // make the configuration for this sample
    conf::configuration(sample);

    // decide what to do based on sample
    // note: this is a relic from earlier development (?),
    //       in practice every sample is 'tt1l'.
    if (sample == "tt1l"){
        // set ptmin and ptmax
        // note: do not use the pt bins defined in the configuration,
        //       as they are meant for the 1D templates;
        //       instead, the values are hard-coded here.
        TString ptmin = "200";
        TString ptmax = "1200";
        makeTemplatesTop(sample, era, wpmin, wpmax, conf::score_def, ptmin, ptmax, outputDir);    
    }
    else {
        TString msg = "Sample " + sample + " not recognized.";
        throw std::runtime_error(msg);
    }
}

void makeTemplatesTop(TString sample, TString era, TString wpmin, TString wpmax,
        TString score="fj_1_ParticleNetMD_XbbVsQCD", 
        TString ptmin="0==0",
        TString ptmax="0==0",
        TString outputDir=".") {

  // some generic ROOT settings
  setTDRStyle();
  gROOT->SetBatch(true);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(1);
  gStyle->SetPalette(1);
  TH1::SetDefaultSumw2(kTRUE);

  // make the configuration for this sample
  // todo: figure out if this actually does anything
  conf::configuration(sample);

  // get processes
  vector<TString> processes = conf::processes;
  vector<TString> process_names = conf::process_names;

  // get path to files and luminosity
  // (depending on the year)
  TString path;
  float intLumi;
  if (era == "2016preVFP") { path = conf::path_2016preVFP; intLumi = 19.52;  }
  if (era == "2016postVFP") { path = conf::path_2016postVFP; intLumi = 16.81;  }
  if (era == "2017") { path = conf::path_2017; intLumi = 41.53; }
  if (era == "2018") { path = conf::path_2018; intLumi = 59.74; }
  if (era == "2022preEE") { path = conf::path_2022preEE; intLumi = 7.98; }
  if (era == "2022postEE") { path = conf::path_2022postEE; intLumi = 26.67; }
  if (era == "2023preBPix") { path = conf::path_2023preBPix; intLumi = 18.08; }
  if (era == "2023postBPix") { path = conf::path_2023postBPix; intLumi = 9.69; }
  ostringstream tmpLumi; tmpLumi << intLumi; TString lumi = tmpLumi.str();
  std::cout << "Found following sample path: " << path << std::endl;
  std::cout << "Found following luminosity: " << intLumi << std::endl;

  // set directory to store the templates
  // todo: maybe add score in output directory or file name to avoid confusion
  outputDir += "/templates2D";
  TString nameoutfile = "particlenet_tt1l_"+wpmin+"to"+wpmax+"_"+era+"_"+ptmin+"to"+ptmax+"_templates";
  
  // make output directory
  const int dir_err = system("mkdir -p " + outputDir);
  if (-1 == dir_err) { printf("Error creating directory!n"); exit(1); }
 
  // make output file
  TString outfile = outputDir+"/"+nameoutfile+".root";
  TFile *fout = new TFile(outfile, "RECREATE");
  std::cout << "Will write output to " << outfile << std::endl;
  
  // cuts and matching definition
  TString c_base = TString("(passmetfilters && passMuTrig")
                   + " && fj_1_pt>=" + ptmin + " && fj_1_pt<" + ptmax
                   + " && abs(fj_1_eta)<2.4 && leptonicW_pt>150.)";
  TString c_p3 = "( (fj_1_dr_T_Wq_max<0.8) && (fj_1_dr_T_b<0.8) )";
  TString c_p2 = "((fj_1_T_Wq_max_pdgId==0 && fj_1_dr_W_daus<0.8) || (fj_1_T_Wq_max_pdgId!=0 && fj_1_dr_T_b>=0.8 && fj_1_dr_T_Wq_max<0.8))";     
  TString c_p1 = "(!("+c_p3+" || "+c_p2+"))";

  std::vector<TString> cuts; cuts.clear();
  cuts.push_back(c_base);
  cuts.push_back(c_base+" && "+c_p3);
  cuts.push_back(c_base+" && "+c_p2);
  cuts.push_back(c_base+" && "+c_p1);

  // WP selection
  TString wp_val;
  TString c_p = "("+score+">"+wpmin+" && "+score+"<="+wpmax+")";
  TString c_f = "(!"+c_p+")";

  cuts.push_back(c_p);
  cuts.push_back(c_f);

  // define axis ranges
  TString brX = conf::brX; int binsX = conf::binsX; float minX = conf::minX; float maxX = conf::maxX;
  TString brY = conf::brY; int binsY = conf::binsY; float minY = conf::minY; float maxY = conf::maxY;

  // define histogram name
  // todo: maybe add score in histogram name to avoid confusion
  TString name = sample+"_"+wpmin+"to"+wpmax+"_"+era;
  
  // make data histograms 
  TString datafile = path+"/data/singlemu_tree.root";
  std::cout << "Making data histogram from file " << datafile << std::endl;
  TFile *f_data  = TFile::Open(datafile, "READONLY");
  TTree *t_data  = (TTree*)f_data->Get("Events");
  TH2D *h_data_p = create2Dhisto(name, t_data, lumi, cuts[0]+" && "+cuts[4],
                     brX, binsX, minX, maxX,
                     brY, binsY, minY, maxY,
                     false, "h_"+name+"_data_p", true);
  TH2D *h_data_f = create2Dhisto(name, t_data, lumi, cuts[0]+" && "+cuts[5],
                     brX, binsX, minX, maxX,
                     brY, binsY, minY, maxY,
                     false, "h_"+name+"_data_f", true);
  fout->cd();
  h_data_p->Write("data_obs_pass");
  h_data_f->Write("data_obs_fail");
  
  // MC templates
  std::vector<TString> syst = conf::syst;
  TString name_, namesys_;
  // loop over systematics
  std::cout << "Looping over systematics for MC histograms" << std::endl;
  for (int i0=0; i0<syst.size(); ++i0){
      name_ = syst.at(i0); namesys_ = "";
      std::cout << "  - Systematic: " << name_ << std::endl;
      if (syst.at(i0)=="_"){
          // nominal
          name_ = "nom";
          namesys_ = "nom";
          makeMCHistosTop(name, path, processes, process_names, name_, namesys_, lumi, cuts,
                          brX, binsX, minX, maxX, brY, binsY, minY, maxY, fout);
      } else if(name_ == "lhescalemuf"){
          // special case for factorization scale
	      makeMCHistosTop(name,path,processes,process_names,name_,namesys_+"Up",
              lumi+"*(LHEScaleWeight[5]*LHEScaleWeightNorm[5])/(LHEScaleWeight[4]*LHEScaleWeightNorm[4])",
              cuts,brX,binsX,minX,maxX,brY,binsY,minY,maxY,fout);
	      makeMCHistosTop(name,path,processes,process_names,name_,namesys_+"Down",
              lumi+"*(LHEScaleWeight[3]*LHEScaleWeightNorm[3])/(LHEScaleWeight[4]*LHEScaleWeightNorm[4])",
              cuts,brX,binsX,minX,maxX,brY,binsY,minY,maxY,fout);
	  } else if(name_ == "lhescalemur"){
	      // special case for renormalization scale
	      makeMCHistosTop(name,path,processes,process_names,name_,namesys_+"Up",
              lumi+"*(LHEScaleWeight[7]*LHEScaleWeightNorm[7])/(LHEScaleWeight[4]*LHEScaleWeightNorm[4])",
              cuts,brX,binsX,minX,maxX,brY,binsY,minY,maxY,fout);
	      makeMCHistosTop(name,path,processes,process_names,name_,namesys_+"Down",
              lumi+"*LHEScaleWeight[1]*LHEScaleWeightNorm[1]/(LHEScaleWeight[4]*LHEScaleWeightNorm[4])",
              cuts,brX,binsX,minX,maxX,brY,binsY,minY,maxY,fout);
	  } else{
          // general systematics
	      makeMCHistosTop(name,path,processes,process_names,name_,namesys_+"Up",lumi,cuts,brX,binsX,minX,maxX,brY,binsY,minY,maxY,fout);
	      makeMCHistosTop(name,path,processes,process_names,name_,namesys_+"Down",lumi,cuts,brX,binsX,minX,maxX,brY,binsY,minY,maxY,fout);
	  }
  }

  fout->Close();
}


void makeMCHistosTop(TString name, TString path, std::vector<TString> processes, std::vector<TString> process_names,
             TString sys, TString sysType, TString wgts, std::vector<TString> cuts,
		     TString brX, int binsX, float minX, float maxX, TString brY, int binsY, float minY, float maxY, TFile *f_) { 

  // set up correct directory (depending on the systematic) inside the main directory   
  TString sys_type_dir_suffix = "/";
  if ( sysType == "Up" ) { sys_type_dir_suffix = "_up/"; }
  if ( sysType == "Down" ) { sys_type_dir_suffix ="_down/"; }
  TString sys_dir;
  if( sys == "nom" || sys == "pu" || sys == "jms" || sys == "jmr" ){ sys_dir = "/LHEWeight/"; }
  else if( sys.Contains("lhe") || sys.Contains("ps") ){ sys_dir = "/LHEWeight/"; }
  else{ sys_dir = "/" +sys +sys_type_dir_suffix; }

  // set up some more names
  if ( (sys == "nom") ) { name = name+"_"+sys; }
  else                  { name = name+"_"+sys+sysType; }
  TString name_b;
  if (sys == "nom")      { name_b = ""; }
  else                   { name_b = sys+sysType; }
  
  // loop over processes
  std::vector<TH2D*>   h2ds;       h2ds.clear();
  std::vector<TString> h2ds_names; h2ds_names.clear();
  for (unsigned int i0=0; i0<processes.size(); ++i0) {
    std::cout << "    - Process: " << processes[i0] << std::endl;
    
    // get the tree
    TFile *f = TFile::Open(path+"/"+sys_dir+processes[i0]+"_tree.root", "READONLY");
    TTree *t = (TTree*)f->Get("Events");

    // decide whether to split in categories (for signal processes)
    vector<TString> processes_splitting = conf::processes_splitting;
    bool doSplit = (std::find(processes_splitting.begin(), processes_splitting.end(), process_names[i0])
                    != processes_splitting.end());
    if( doSplit ){
    // special case for signal processes: split in categories (for both pass and fail)
	TH2D *h_p3_p = create2Dhisto(name,t,wgts,cuts[1]+"&&"+cuts[4],brX,binsX,minX,maxX,brY,binsY,minY,maxY,false,"h_"+name+"_"+process_names[i0]+"_p3_p",false); h2ds.push_back(h_p3_p); h2ds_names.push_back(process_names[i0]+"_p3_"+name_b+"_pass"); 
	TH2D *h_p2_p = create2Dhisto(name,t,wgts,cuts[2]+"&&"+cuts[4],brX,binsX,minX,maxX,brY,binsY,minY,maxY,false,"h_"+name+"_"+process_names[i0]+"_p2_p",false); h2ds.push_back(h_p2_p); h2ds_names.push_back(process_names[i0]+"_p2_"+name_b+"_pass");
	TH2D *h_p1_p = create2Dhisto(name,t,wgts,cuts[3]+"&&"+cuts[4],brX,binsX,minX,maxX,brY,binsY,minY,maxY,false,"h_"+name+"_"+process_names[i0]+"_p1_p",false); h2ds.push_back(h_p1_p); h2ds_names.push_back(process_names[i0]+"_p1_"+name_b+"_pass");
	
	TH2D *h_p3_f = create2Dhisto(name,t,wgts,cuts[1]+"&&"+cuts[5],brX,binsX,minX,maxX,brY,binsY,minY,maxY,false,"h_"+name+"_"+process_names[i0]+"_p3_f",false); h2ds.push_back(h_p3_f); h2ds_names.push_back(process_names[i0]+"_p3_"+name_b+"_fail");
	TH2D *h_p2_f = create2Dhisto(name,t,wgts,cuts[2]+"&&"+cuts[5],brX,binsX,minX,maxX,brY,binsY,minY,maxY,false,"h_"+name+"_"+process_names[i0]+"_p2_f",false); h2ds.push_back(h_p2_f); h2ds_names.push_back(process_names[i0]+"_p2_"+name_b+"_fail");
	TH2D *h_p1_f = create2Dhisto(name,t,wgts,cuts[3]+"&&"+cuts[5],brX,binsX,minX,maxX,brY,binsY,minY,maxY,false,"h_"+name+"_"+process_names[i0]+"_p1_f",false); h2ds.push_back(h_p1_f); h2ds_names.push_back(process_names[i0]+"_p1_"+name_b+"_fail");
    } else{
    // general case for all other processes: no further splitting in categories
	TH2D *h_p = create2Dhisto(name,t,wgts,cuts[0]+"&&"+cuts[4],brX,binsX,minX,maxX,brY,binsY,minY,maxY,false,"h_"+name+"_"+process_names[i0]+"_p",false); h2ds.push_back(h_p); h2ds_names.push_back(process_names[i0]+"_"+name_b+"_pass");
	TH2D *h_f = create2Dhisto(name,t,wgts,cuts[0]+"&&"+cuts[5],brX,binsX,minX,maxX,brY,binsY,minY,maxY,false,"h_"+name+"_"+process_names[i0]+"_f",false); h2ds.push_back(h_f); h2ds_names.push_back(process_names[i0]+"_"+name_b+"_fail");
    }
  } // end of loop over the processes
  
  // avoid empty or negative bins in mc
  for (unsigned int i0=0; i0<h2ds.size(); ++i0){
      for (unsigned int i1=0; i1<h2ds[i0]->GetNbinsX(); ++i1){
	      for (unsigned int i1y=0; i1y<h2ds[i0]->GetNbinsY(); ++i1y){
	          if (h2ds[i0]->GetBinContent(i1,i1y)<=0) { h2ds[i0]->SetBinContent(i1,i1y,0.001); h2ds[i0]->SetBinError(i1,i1y,0.001); }
	      }
      }
  }

  // write histos to file
  f_->cd();
  for (unsigned int i0=0; i0<h2ds.size(); ++i0) { h2ds[i0]->Write(h2ds_names[i0]); }
  
}


TH2D *create2Dhisto(TString sample, TTree *tree, TString intLumi, TString cuts,
                    TString branchX, int binsX, float minX, float maxX,
                    TString branchY, int binsY, float minY, float maxY,
                    bool useLog, TString name, bool data){
  // Make the actual 2D histogram for a given tree with given cuts

  // general ROOT settings
  TH1::SetDefaultSumw2(kTRUE);
  
  // set up correct pileup reweighting factor
  TString puWgt;
  if (name.Contains("puUp"))        { puWgt = "puWeightUp"; }
  else if (name.Contains("puDown")) { puWgt = "puWeightDown"; }
  else                              { puWgt = "puWeight"; }
  // temporary fix for the fact that puWeight does not seem to be a valid branch
  // (todo: find out which branch (if any) to use later)
  puWgt = "1.";

  // set up correct gen weight
  TString genWgt = "xsecWeight*genWeight";
  
  // set up correct top-pt reweighting factor
  TString ttWgt     = "1.";
  if (sample.Contains("tt1l")) { ttWgt = "topptWeight"; };

  // make the configuration for this sample
  conf::configuration(sample);
  
  // set up the correct processing string
  // (cut + reweighting)
  TString cut;
  if (data) { cut ="("+cuts+")"; } 
  else{
      if (name.Contains("tt")) { cut = "("+intLumi+"*"+puWgt+"*"+genWgt+"*"+ttWgt+")*("+cuts+")"; }
      else { cut = "("+intLumi+"*"+puWgt+"*"+genWgt+")*("+cuts+")"; }
  }
  
  // printouts for debugging
  std::cout << "      Process name: " << name << "\n";
  std::cout << "      Cut = " << cut << "\n";

  // initialize the histogram
  TH2D *hTemp = new TH2D(name,name,binsX,minX,maxX,binsY,minY,maxY);

  // set up parameters for templates with varied scale and smearing
  TString massScaleVal_ = "1.05"; if (name.Contains("Down")) { massScaleVal_ = "0.95"; }
  TString massSmearVal_ = "0.10"; if (name.Contains("Down")) { massSmearVal_ = "0."; }
  
  // fill the histograms
  if (name.Contains("jms")) 
    { 
      tree->Project(name, branchY+":(massScale("+branchX+","+massScaleVal_+"))", cut);
    }
  else if (name.Contains("jmr")) 
    { 
      tree->Project(name, branchY+":(massSmear("+branchX+",luminosityBlock,event,"+massSmearVal_+"))", cut); 
    }
  else
    {
      tree->Project(name,branchY+":"+branchX,cut); 
    }
  
  // ?
  for (unsigned int i0x=0; i0x<hTemp->GetNbinsX(); ++i0x) 
    {
      double error =0.; double integral = hTemp->IntegralAndError(i0x+1,i0x+1,hTemp->GetNbinsY(),hTemp->GetNbinsY()+1,error);
      hTemp->SetBinContent(i0x+1,hTemp->GetNbinsY(),integral);
      hTemp->SetBinError(i0x+1,hTemp->GetNbinsY(),error);
    } 
  
  return hTemp;
}
