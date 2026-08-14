#include "setTDRStyle.h"
#include <sstream>
#include <fstream>
#include "TTree.h"
#include "TFile.h"
#include "TH1D.h"
#include <vector>
#include "Rtypes.h"
#include "TColor.h"
#include "TVectorF.h"
#include <cstdlib>
#include "configuration.h"
#include <string>
#include <unistd.h>


// declare helper functions
void setTDRStyle();
TH1F *rescaleXaxis(TH1F *inputhisto, float xmin, float xmax);
void rescaleXaxis(TGraphAsymmErrors *inputhisto, double xmin, double scale);
TH1F *getDataMCratio(TGraphAsymmErrors *indata, TH1F *inMC);
TH1F *getRelativeUncertaintyBand(TH1F *inMC);
void makeDataMCPlotFromCombine(TString path2file, TString era, TString wpmin, TString wpmax, TString name, TString passOrFail, 
			       float xmin, float xmax, int nbins,TString xaxisname, bool log, TString sample);


// main function
void makePostFitPlots(TString era, TString sample,
       TString wpmin, TString wpmax,
       TString binname, TString passOrFail,
       TString outputDir="."){

  // make the configuration for this sample
  conf::configuration(sample);
  
  if (sample == "tt1l"){
	  makeDataMCPlotFromCombine(outputDir, era, wpmin, wpmax, binname, passOrFail,
            conf::minX, conf::maxX, conf::binsX, "m_{ParT} [GeV]", false, sample);
  }
  else{
      TString msg = "Sample " + sample + " not recognized.";
      throw std::runtime_error(msg);
  }
}


void makeDataMCPlotFromCombine(TString workdir, TString era,
                   TString wpmin, TString wpmax, TString name, TString passOrFail, 
			       float xmin, float xmax, int nbins, TString xaxisname, bool log, TString sample =""){
  // Plotting function for data vs. MC plot starting from a combine output file
  // Note: the required input seems to be the output file from the FitDiagnostics method in combine.

  // general ROOT settings
  setTDRStyle();
  gROOT->SetBatch(true);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);
  gStyle->SetPalette(1);
  TH1::SetDefaultSumw2(kTRUE);

  // make the configuration for this sample
  conf::configuration(sample);

  // parse default x-axis name
  if (xaxisname == "mass") { xaxisname = "m_{ParT} [GeV]"; }

  // make output directory
  const int dir_err = system("mkdir -p "+(TString)workdir+"/plots_postfit");
  if (-1 == dir_err) { printf("Error creating directory!n"); exit(1); } 

  // read input file
  TString algolabel = "particlenet";
  TString fullname = algolabel + "_" + sample + "_" + wpmin + "to" + wpmax + "_" +era + "_" +name;
  TString fdiag_ = workdir+"/fitdir/fitDiagnostics_" + fullname + ".root";
  std::cout << "INFO in makeDataMCPlotsFromCombine: reading input file " << fdiag_ << std::endl;
  TFile *fdiag = TFile::Open(fdiag_, "READONLY");

  // read processes, colors and legend entries from config
  std::map<TString, std::vector<TString>> processes_grouping = conf::processes_grouping;
  std::vector<TString> processes;
  for(const auto& pair: processes_grouping){
      processes.push_back(pair.first);
  }
  std::map<TString, int> colors = conf::colors;
  std::map<TString, TString> legend_labels = conf::legend_labels;

  // also add total
  // (hard-coded for now)
  processes.push_back("total");
  colors["total"] = 8;
  legend_labels["total"] = "Total";

  // get histograms
  TString prefitstr = "shapes_prefit";
  TTree *tree = (TTree*)fdiag->Get("tree_fit_sb");
  TH1F *h_fit_status = new TH1F("h_fit_status_","h_fit_status_",20,-10.,10.); 
  tree->Project("h_fit_status_","fit_status");
  TString postfitstr = "shapes_fit_s"; if (h_fit_status->GetMean()<0.) { postfitstr = "shapes_prefit"; }
  std::vector<TH1F*> h_prefit; h_prefit.clear();
  std::vector<TH1F*> h_postfit; h_postfit.clear();
  std::cout << "INFO in makeDataMCPlotsFromCombine: reading histograms:" << std::endl;
  for(TString process: processes){
      // get prefit histos
      TString h_prefit_name = prefitstr+"/"+passOrFail+"/"+process;
      std::cout << "  - " << h_prefit_name << " (prefit)" << std::endl;
      TH1F *h_prefit_ = (TH1F*)fdiag->Get(h_prefit_name);
      h_prefit_->SetName("h_pre_"+name+"_"+process+"_"+passOrFail+"_");
      TH1F *h_prefit__ = rescaleXaxis(h_prefit_,conf::minX,conf::maxX);
      h_prefit_->SetName("h_pre_"+name+"_"+process+"_"+passOrFail);
      h_prefit__->SetLineColor(colors.at(process));
      h_prefit__->SetLineStyle(2);
      h_prefit__->SetLineWidth(3);
      h_prefit__->SetMarkerSize(0);
      h_prefit.push_back(h_prefit__);
      
      // get postfit histograms
      TString h_postfit_name = postfitstr+"/"+passOrFail+"/"+process;
      std::cout << "  - " << h_postfit_name << " (postfit)" << std::endl;
      TH1F *h_postfit_ = (TH1F*)fdiag->Get(h_postfit_name);
      h_postfit_->SetName("h_post_"+name+"_"+process+"_"+passOrFail+"_");
      TH1F *h_postfit__ = rescaleXaxis(h_postfit_,conf::minX,conf::maxX);
      h_postfit_->SetName("h_post_"+name+"_"+process+"_"+passOrFail);
      h_postfit__->SetLineColor(colors.at(process));
      h_postfit__->SetLineStyle(1);
      h_postfit__->SetLineWidth(3);
      h_postfit__->SetMarkerSize(0);
      h_postfit.push_back(h_postfit__);      
  }
  
  // get data
  TString h_data_name = "shapes_prefit/"+passOrFail+"/data";
  std::cout << "  - " << h_data_name << " (data)" << std::endl;
  TGraphAsymmErrors *h_data = (TGraphAsymmErrors*)fdiag->Get(h_data_name);
  h_data->SetName("h_data_"+name+"_"+passOrFail);
  rescaleXaxis(h_data, conf::minX, (conf::maxX-conf::minX)/(float)conf::binsX);
  h_data->SetLineColor(1); h_data->SetLineWidth(3); 
  h_data->SetMarkerColor(1); h_data->SetMarkerStyle(20); h_data->SetMarkerSize(1.2); 

  int numOfMC = h_prefit.size();
  TH1F *h_r_prefit = getDataMCratio(h_data,h_prefit[numOfMC-1]); h_r_prefit->SetName("h_r_prefit_"+name+"_"+passOrFail);
  h_r_prefit->SetMarkerSize(0); h_r_prefit->SetLineColor(42); h_r_prefit->SetLineStyle(2);
  
  TH1F *h_r_postfit = getDataMCratio(h_data,h_postfit[numOfMC-1]); h_r_postfit->SetName("h_r_postfit_"+name+"_"+passOrFail);
  h_r_postfit->SetMarkerColor(1); h_r_postfit->SetLineColor(1);

  // relative post-fit uncertainty band on the total background, for the ratio panel
  TH1F *h_r_band = getRelativeUncertaintyBand(h_postfit[numOfMC-1]); h_r_band->SetName("h_r_band_"+name+"_"+passOrFail);
  h_r_band->SetFillColor(kGray+2);
  h_r_band->SetFillStyle(3345);
  h_r_band->SetLineColor(kGray+2);
  h_r_band->SetMarkerSize(0);

  // make legend (upper panel)
  TLegend* leg = new TLegend(0.50, 0.65, 0.94, 0.89);
  leg->SetNColumns(2);
  leg->SetFillStyle(0);
  leg->SetFillColor(0);
  leg->SetLineWidth(0);
  leg->SetTextFont(42);
  for(unsigned int i0=0; i0<h_prefit.size(); ++i0){
      leg->AddEntry(h_postfit[i0], legend_labels.at(processes[i0]), "L");
  }

  // make legend (lower panel)
  TLegend* leg_ratio = new TLegend(0.2, 0.82, 0.94, 0.95);
  leg_ratio->SetNColumns(2);
  leg_ratio->SetFillStyle(0);
  leg_ratio->SetFillColor(0);
  leg_ratio->SetLineWidth(0);
  leg_ratio->SetTextFont(42);
  leg_ratio->AddEntry(h_r_prefit, "Data / pre-fit", "L");
  leg_ratio->AddEntry(h_r_postfit, "Data / post-fit", "PE");
  leg_ratio->AddEntry(h_r_band, "Post-fit unc.", "F");

  // write CMS name
  TPaveText *pt_cms = new TPaveText(0.17, 0.82, 0.6, 0.9, "NDC");
  pt_cms->SetFillStyle(0);
  pt_cms->SetFillColor(0);
  pt_cms->SetLineWidth(0);
  pt_cms->SetTextAlign(12);
  pt_cms->AddText("CMS");

  // write extra text
  TPaveText *pt_preliminary = new TPaveText(0.17, 0.76, 0.6, 0.81, "NDC");
  pt_preliminary->SetFillStyle(0);
  pt_preliminary->SetFillColor(0);
  pt_preliminary->SetLineWidth(0);
  pt_preliminary->SetTextAlign(12);
  pt_preliminary->AddText("Preliminary");
  pt_preliminary->SetTextFont(52);

  // write extra info
  TPaveText *pt_info = new TPaveText(0.17, 0.5, 0.6, 0.75, "NDC");
  pt_info->SetFillStyle(0);
  pt_info->SetFillColor(0);
  pt_info->SetLineWidth(0);
  pt_info->SetTextAlign(12);
  pt_info->SetTextFont(42);
  pt_info->AddText("Year: " + era);
  pt_info->AddText("WP: " + wpmin + " - " + wpmax);
  pt_info->AddText("Bin: " + name);
  pt_info->AddText("Category: " + passOrFail);

  // draw lumi header
  TLatex lumiHeader;
  std::string lumiHeaderText = "";
  if (era=="2016preVFP") { lumiHeaderText = "2016-preVFP, 19.52 fb^{-1}, 13 TeV"; }
  if (era=="2016postVFP") { lumiHeaderText = "2016-postVFP, 16.81 fb^{-1}, 13 TeV"; }
  if (era=="2017") { lumiHeaderText = "2017, 41.53 fb^{-1}, 13 TeV"; }
  if (era=="2018") { lumiHeaderText = "2018, 59.74 fb^{-1}, 13 TeV"; }
  if (era=="2022preEE") { lumiHeaderText = "2022-preEE, 7.98  fb^{-1}, 13.6 TeV"; }
  if (era=="2022postEE") { lumiHeaderText = "2022-postEE, 26.67  fb^{-1}, 13.6 TeV"; }
  if (era=="2023preBPix") { lumiHeaderText = "2023-preBPix, 18.08  fb^{-1}, 13.6 TeV"; }
  if (era=="2023postBPix") { lumiHeaderText = "2023-postBPix, 9.69  fb^{-1}, 13.6 TeV"; }
  if (era=="2024") { lumiHeaderText = "2024, 109.08 fb^{-1}, 13.6 TeV"; }
  lumiHeader.SetTextSize(0.06);
  lumiHeader.SetTextFont(42);
  lumiHeader.SetTextAlign(31);
  
  TString logstr = "lin"; if (log) { logstr = "log"; } 
  TCanvas *c = new TCanvas("c_"+name, "c_"+name, 600, 600); 
  c->SetName("c_"+name);

  TPad *pMain  = new TPad("pMain_"+name,"pMain+"+name,0.0,0.35,1.0,1.0);
  pMain->SetRightMargin(0.05);
  pMain->SetLeftMargin(0.17);
  pMain->SetBottomMargin(0.05);
  pMain->SetTopMargin(0.1);
  TPad *pRatio = new TPad("pRatio_"+name,"pRatio_"+name,0.0,0.03,1.0,0.37);
  pRatio->SetRightMargin(0.05);
  pRatio->SetLeftMargin(0.17);
  pRatio->SetTopMargin(0.03);
  pRatio->SetBottomMargin(0.35);
  
  pMain->Draw();
  pRatio->Draw();
  pMain->cd();

  float maxyld = h_postfit[numOfMC-1]->GetMaximum(); 
  if (h_prefit[numOfMC-1]->GetMaximum()>h_postfit[numOfMC-1]->GetMaximum()) { maxyld = h_prefit[numOfMC-1]->GetMaximum(); }
  if (log) { gPad->SetLogy(); h_prefit[numOfMC-1]->GetYaxis()->SetRangeUser(0.1,10.*maxyld); } else { h_prefit[numOfMC-1]->GetYaxis()->SetRangeUser(0.,1.8*maxyld); }
  h_prefit[numOfMC-1]->GetXaxis()->SetLabelSize(0.);

  // format y-axis (upper panel)
  h_prefit[numOfMC-1]->GetYaxis()->SetTitle("Events / Bin");
  h_prefit[numOfMC-1]->GetYaxis()->SetTitleOffset(1.0);
  h_prefit[numOfMC-1]->GetYaxis()->SetTitleSize(0.06);

  h_prefit[numOfMC-1]->Draw("HIST E0");
  for (unsigned int i0=0; i0<h_prefit.size(); ++i0)  { h_prefit[i0]->Draw("HIST E0 sames"); }
  for (unsigned int i0=0; i0<h_postfit.size(); ++i0) { h_postfit[i0]->Draw("HIST E0 sames"); }
  h_data->Draw("P sames");
  leg->Draw("sames");
  pt_cms->Draw("sames");
  pt_preliminary->Draw("sames");
  pt_info->Draw("sames");
  lumiHeader.DrawLatexNDC(0.95, 0.92, lumiHeaderText.c_str());
  c->RedrawAxis();
  pRatio->cd();

  // format y-axis (lower panel)
  h_r_postfit->GetYaxis()->SetTitle("Data / Pred.");
  h_r_postfit->GetYaxis()->SetRangeUser(0.61,1.39);
  h_r_postfit->GetYaxis()->SetTitleOffset(0.6);
  h_r_postfit->GetYaxis()->SetTitleSize(0.1);
  h_r_postfit->GetYaxis()->SetLabelSize(0.1);

  // format x-axis (lower panel)
  h_r_postfit->GetXaxis()->SetTitle(xaxisname);
  h_r_postfit->GetXaxis()->SetTitleOffset(1.0);
  h_r_postfit->GetXaxis()->SetTitleSize(0.12);
  h_r_postfit->GetXaxis()->SetLabelSize(0.1);

  // define unit ratio line
  TLine* line = new TLine(xmin, 1, xmax, 1);
  line->SetLineColor(kRed);
  line->SetLineWidth(2);

  // draw all elements of lower pad in correct order
  h_r_postfit->Draw("AXIS");
  h_r_band->Draw("E2 sames");
  line->Draw("sames");
  h_r_prefit->Draw("HIST sames");
  h_r_postfit->Draw("P E0 sames");
  leg_ratio->Draw("sames");
  c->RedrawAxis();
 
  if (log) 
    {
      c->Print(workdir+"/plots_postfit/" + fullname + "_" + passOrFail + "_log.pdf");
      c->Print(workdir+"/plots_postfit/" + fullname + "_" + passOrFail + "_log.png");
    } 
  else 
    {
      c->Print(workdir+"/plots_postfit/" + fullname + "_" + passOrFail + "_lin.pdf");
      c->Print(workdir+"/plots_postfit/" + fullname + "_" + passOrFail + "_lin.png");
    } 
}


TH1F *rescaleXaxis(TH1F *inputhisto, float xmin, float xmax) {
  // Helper function to copy a histogram and modify its x-axis,
  // to run uniformly from xmin to xmax
  // (in as many bins as there were in the original histogram).
  TH1::SetDefaultSumw2(kTRUE);

  int nbins = inputhisto->GetNbinsX();
  TString name = inputhisto->GetName();
  TH1F *outputhisto = new TH1F(name,name,nbins,xmin,xmax);

  for (unsigned int i0=0; i0<nbins; ++i0) {
    outputhisto->SetBinContent(i0+1,inputhisto->GetBinContent(i0+1));
    outputhisto->SetBinError(i0+1,inputhisto->GetBinError(i0+1));
  }

  return outputhisto;
}


void rescaleXaxis(TGraphAsymmErrors *g, double xmin, double scaleX) {
  // Helper function to modify the x-axis of a graph.
  // todo: figure out what the use of xmin and scaleX is exactly.
  TH1::SetDefaultSumw2(kTRUE);

  int N=g->GetN();
  double *x=g->GetX();
  double *xerrh=g->GetEXhigh();
  double *xerrl=g->GetEXlow();
  int i=0; while(i<N) {
    x[i]=xmin+x[i]*scaleX;
    xerrh[i]=0.;
    xerrl[i]=0.;
    i=i+1;
  }
  g->GetHistogram()->Delete();
  g->SetHistogram(0);
}


TH1F *getDataMCratio(TGraphAsymmErrors *indata, TH1F *inMC) {
  // Helper function to get the data-to-MC ratio.
  // Note: the ratio error is taken from the data (statistical) error only;
  // the uncertainty on the prediction itself is shown separately as a band
  // (see getRelativeUncertaintyBand), so it is not folded in here to avoid
  // double-counting it in the data points' error bars.
  TH1::SetDefaultSumw2(kTRUE);
  TH1F *h_data = (TH1F*)inMC->Clone("h_data"); h_data->SetName("h_data");
  for (unsigned int i0=0; i0<inMC->GetNbinsX(); ++i0)
    {
      h_data->SetBinContent(i0+1, indata->GetY()[i0]);
      h_data->SetBinError(i0+1, (indata->GetEYlow()[i0]+indata->GetEYhigh()[i0])/2.);
    }

  TH1F *h_ratio = (TH1F*)h_data->Clone("h_ratio");
  h_ratio->Divide(h_data,inMC);
  for (unsigned int i0=0; i0<inMC->GetNbinsX(); ++i0)
    {
      double mc = inMC->GetBinContent(i0+1);
      double dataerr = h_data->GetBinError(i0+1);
      h_ratio->SetBinError(i0+1, (mc>0) ? dataerr/mc : 0.);
    }
  h_ratio->SetMarkerStyle(20); h_ratio->SetMarkerSize(1);
  h_ratio->SetLineWidth(2); h_ratio->SetLineStyle(1);

  return h_ratio;
}


TH1F *getRelativeUncertaintyBand(TH1F *inMC) {
  // Helper function to build a band centered at 1 whose half-width is the
  // relative (post-fit) uncertainty of inMC, for overlaying on a ratio plot.
  TH1F *h_band = (TH1F*)inMC->Clone("h_band");
  for (unsigned int i0=0; i0<inMC->GetNbinsX(); ++i0)
    {
      double val = inMC->GetBinContent(i0+1);
      double err = inMC->GetBinError(i0+1);
      h_band->SetBinContent(i0+1, 1.);
      h_band->SetBinError(i0+1, (val>0) ? err/val : 0.);
    }
  return h_band;
}
