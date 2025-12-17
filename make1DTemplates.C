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
TH1D *h1DHistoFrom2DTemplates(TString path2file,TString h2dname,TString name,double ymin, double ymax,int color,bool isdata);
TCanvas *makeCanvasWithRatio(TH1D* hdata, std::vector<TH1D*> h1d, TString name, TString xname, TString yname, TLegend *leg);
void makeDataMCFrom2DTemplatesTop(TString path2file, TString nameoutfile, TString ptname, TString sample, double ymin, double ymax, TString outputDir=".");


// main function
void make1DTemplates(TString era, TString sample, TString wpmin, TString wpmax, TString outputDir="."){

  // make the configuration for this sample
  conf::configuration(sample);
  std::vector<TString> ptnames = conf::ptnames;
  std::vector<double>  ptmin = conf::ptmin;
  std::vector<double>  ptmax = conf::ptmax; 
  
  if (sample == "tt1l"){
      TString inputSubDir = outputDir + "/templates2D";
      TString outputSubDir = outputDir + "/templates1D";
      // loop over pt bins
	  for (int i0=0; i0<ptnames.size(); ++i0){ 
	      std::cout << "Now running on pt bin " << ptnames[i0] << std::endl;
          TString inputFile = inputSubDir + "/particlenet_"+sample+"_"+wpmin+"to"+wpmax+"_"+era+"_200to1200_templates.root";
          TString outputName = "particlenet_"+sample+"_"+wpmin+"to"+wpmax+"_"+era;
	      makeDataMCFrom2DTemplatesTop(inputFile, outputName, ptnames[i0], sample, ptmin[i0], ptmax[i0], outputSubDir);
	  }
  }
  else{
      TString msg = "Sample " + sample + " not recognized.";
      throw std::runtime_error(msg);
  }
}


void makeDataMCFrom2DTemplatesTop(TString path2file, TString nameoutfile, TString name, TString sample,
        double ymin, double ymax,
        TString outputDir="."){

  // standard ROOT settings
  TH1::SetDefaultSumw2(kTRUE);
  setTDRStyle();
  gROOT->SetBatch(true);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);
  gStyle->SetPalette(1);
  conf::configuration(path2file);

  // prepare directories
  const int dir_err = system("mkdir -p " + outputDir);
  if (-1 == dir_err) { printf("Error creating directory!n"); exit(1); }
  nameoutfile = nameoutfile+"_"+name;

  // get data templates 
  TH1D *h_data_p  = h1DHistoFrom2DTemplates(path2file,"data_obs_pass",name,ymin,ymax,1,true);
  TH1D *h_data_f  = h1DHistoFrom2DTemplates(path2file,"data_obs_fail",name,ymin,ymax,1,true);

  // settings for MC templates
  std::vector<TString> syst = conf::syst;
  std::map<TString, std::vector<TString>> processes_grouping = conf::processes_grouping;
  std::map<TString, TString> legend_labels = conf::legend_labels;
  std::map<TString, int> colors = conf::colors;

  std::vector<TH1D*> hist_out_p; hist_out_p.clear();
  std::vector<TH1D*> hist_out_f; hist_out_f.clear();
  std::vector<TH1D*> hist_out_nom_p; hist_out_nom_p.clear();
  std::vector<TH1D*> hist_out_nom_f; hist_out_nom_f.clear();
  std::vector<TString> category; 
  category.push_back("pass"); 
  category.push_back("fail");

  // convert the legend labels map in an (ordered) vector
  std::vector<TString> labels;
  for( const auto& pair: processes_grouping ){
      TString process_out = pair.first;
      labels.push_back(legend_labels.at(process_out));
  }

  // loop over pass and fail categories 
  for(unsigned int ic=0; ic<category.size(); ++ic){
      TString catstr_;
      if (category[ic] == "pass") { catstr_ = "pass"; }
      else { catstr_ = "fail"; }

      // loop over systematics and processes
      for(unsigned int is=0; is<syst.size(); ++is){
      for( const auto& pair: processes_grouping ){
          TString process_out = pair.first;
          std::vector<TString> processes_in = pair.second;

          // handle nominal case
	      if (syst[is]=="_"){

		      TH1D *hist = h1DHistoFrom2DTemplates(path2file, processes_in[0]+"_"+syst[is]+catstr_,
                                  name, ymin, ymax, 1, false);
              for(unsigned int ip=1; ip<processes_in.size(); ++ip){
		          TH1D* htemp = h1DHistoFrom2DTemplates(path2file, processes_in[ip]+"_"+syst[is]+catstr_,
                                  name, ymin, ymax, 1, false);
		          hist->Add(htemp);
		      }
          
		      hist->SetName(process_out);
              hist->SetLineColor(colors.at(process_out));
              hist->SetFillColor(colors.at(process_out));
		      if (category[ic] == "pass") { hist_out_p.push_back(hist); hist_out_nom_p.push_back(hist); }
		      else                        { hist_out_f.push_back(hist); hist_out_nom_f.push_back(hist); } 
		  }

          // handle systematic case
	      else{
              // set the correct name for the systematic
              // note: this determines the correlation of the systematic between processes!
              //       maybe later add in config fille.
		      TString nameSyst = syst[is];
		      if ( (syst[is].Contains("jms")) || (syst[is].Contains("jmr")) ){
		          nameSyst = process_out+syst[is];
		      }
              
              // up variation
              TH1D *hist_up = h1DHistoFrom2DTemplates(path2file, processes_in[0]+"_"+syst[is]+"Up_"+catstr_,
                                     name, ymin, ymax, 1, false);
		      for(unsigned int ip=1; ip<processes_in.size(); ++ip){
		          TH1D *htemp_up = h1DHistoFrom2DTemplates(path2file, processes_in[ip]+"_"+syst[is]+"Up_"+catstr_,
                                     name, ymin, ymax, 1, false);
		          hist_up->Add(htemp_up);
              }
		      hist_up->SetName(process_out+"_"+nameSyst+"Up");
              hist_up->SetLineColor(colors.at(process_out));
              hist_up->SetFillColor(colors.at(process_out));
		      if (category[ic] == "pass") { hist_out_p.push_back(hist_up); }
              else { hist_out_f.push_back(hist_up); }

              // down variation
              TH1D *hist_down = h1DHistoFrom2DTemplates(path2file, processes_in[0]+"_"+syst[is]+"Down_"+catstr_,
                                     name, ymin, ymax, 1, false);
              for(unsigned int ip=1; ip<processes_in.size(); ++ip){ 
                  TH1D *htemp_down = h1DHistoFrom2DTemplates(path2file, processes_in[ip]+"_"+syst[is]+"Down_"+catstr_,
                                     name, ymin, ymax, 1, false);
                  hist_down->Add(htemp_down);
              }
              hist_down->SetName(process_out+"_"+nameSyst+"Down");
              hist_down->SetLineColor(colors.at(process_out));
              hist_down->SetFillColor(colors.at(process_out)); 
              if (category[ic] == "pass") { hist_out_p.push_back(hist_down); }
              else { hist_out_f.push_back(hist_down); }
		  }
      } // end of loop over the processes
      } // end of loop over the syst 
  } // end of loop over the categories
 
  // make a legend
  TLegend* leg = new TLegend(0.5, 0.6, 0.92, 0.85);
  leg->SetNColumns(2);
  leg->SetFillStyle(0);
  leg->SetFillColor(0);
  leg->SetLineWidth(0);
  leg->SetTextFont(42);
  leg->AddEntry(h_data_p, "Data", "PL");
  for(unsigned int i0=0; i0<hist_out_nom_p.size(); ++i0){
      leg->AddEntry(hist_out_nom_p[i0], labels[i0], "F");
  }

  // make a plot
  TString xname = "m_{scoutGloParT} [GeV]";
  TString yname = "Events / Bin";
  TCanvas *c_pass = makeCanvasWithRatio(h_data_p, hist_out_nom_p, nameoutfile+"_pass", xname, yname, leg);  
  c_pass->Print(outputDir+"/"+nameoutfile+"_pass.png"); 
  TCanvas *c_fail = makeCanvasWithRatio(h_data_f, hist_out_nom_f, nameoutfile+"_fail", xname, yname, leg);
  c_fail->Print(outputDir+"/"+nameoutfile+"_fail.png");
  
  // write templates to ROOT file
  TFile *fout_p = new TFile(outputDir+"/"+nameoutfile+"_templates_p.root","RECREATE");
  h_data_p->Write("data_obs");
  for (unsigned int i0=0; i0<hist_out_p.size(); ++i0) 
    {
      hist_out_p[i0]->Write(hist_out_p[i0]->GetName());
    }  
  fout_p->Close();
  
  TFile *fout_f = new TFile(outputDir+"/"+nameoutfile+"_templates_f.root","RECREATE");
  h_data_f->Write("data_obs");
  for (unsigned int i0=0; i0<hist_out_f.size(); ++i0)
    {
      hist_out_f[i0]->Write(hist_out_f[i0]->GetName());
    }
  fout_f->Close();
  
  hist_out_p.clear(); hist_out_nom_p.clear();
  hist_out_f.clear(); hist_out_nom_f.clear();
  
}


TCanvas *makeCanvasWithRatio(TH1D* hdata, std::vector<TH1D*> h1d, TString name, TString xname, TString yname, TLegend *leg) {

  // standard ROOT settings
  TH1::SetDefaultSumw2(kTRUE);
  setTDRStyle();
  gROOT->SetBatch(true);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);
  gStyle->SetPalette(1);
  gStyle->SetErrorX(0);

  // get some properties from the provided name
  // (depends on naming convention, but only used for displaying on canvas)
  std::stringstream namestream = (std::stringstream)(std::string)name;
  std::string temp;
  std::vector<std::string> parts;
  char split = '_';
  while( std::getline(namestream, temp, split) ){
      parts.push_back(temp);
  }
  std::string wprange = parts[2];
  wprange = std::regex_replace(wprange, std::regex("to"), " - ");
  std::string era = parts[3];
  std::string ptrange = parts[4];
  ptrange = std::regex_replace(ptrange, std::regex("to"), " - ");
  ptrange = std::regex_replace(ptrange, std::regex("pt"), "");
  std::string cat = parts[5];

  // make stack of predictions
  THStack *hs = new THStack("hs_"+name, "hs_"+name);
  std::vector<TH1D*> histos; histos.clear();
  for (int i0=0; i0<h1d.size(); ++i0){ 
      TString count = std::to_string(i0);
      hs->Add(h1d[i0]);
      TH1D* h_ = (TH1D*)h1d[i0]->Clone("h_"+count);
      h_->SetFillColor(0);
      h_->SetLineStyle(2);
      histos.push_back(h_);
    }

  // make sum of predictions (to use later in ratio) 
  TH1D *h_sum = (TH1D*)h1d[0]->Clone("h_sum");
  h_sum->Reset("ICES");
  for (int i0=0; i0<h1d.size(); ++i0){ h_sum->Add(h1d[i0]); }

  // initialize canvas and pads
  TCanvas *c_ = new TCanvas("c_"+name,"c_"+name,600,600); c_->SetName("c_"+name);
  TPad *pMain  = new TPad("pMain_"+name, "pMain+"+name, 0.0, 0.35, 1.0, 1.0);
  pMain->SetRightMargin(0.05);
  pMain->SetLeftMargin(0.17);
  pMain->SetBottomMargin(0.05);
  pMain->SetTopMargin(0.1);
  TPad *pRatio = new TPad("pRatio_"+name, "pRatio_"+name, 0.0, 0.03, 1.0, 0.37);
  pRatio->SetRightMargin(0.05);
  pRatio->SetLeftMargin(0.17);
  pRatio->SetTopMargin(0.05);
  pRatio->SetBottomMargin(0.39);
  pMain->Draw();
  pRatio->Draw();
 
  // switch to upper pad
  pMain->cd();

  // upper pad axis layout
  hdata->GetXaxis()->SetLabelSize(0);
  hdata->GetYaxis()->SetTitle(yname);
  hdata->GetYaxis()->SetTitleSize(0.06);
  hdata->GetYaxis()->SetTitleOffset(1);
  hdata->GetYaxis()->SetLabelSize(0.05);

  // upper pad y-axis range
  if (name.Contains("pass")) { hdata->GetYaxis()->SetRangeUser(0.,2*hdata->GetMaximum()); }
  else { hdata->GetYaxis()->SetRangeUser(0.,2*hdata->GetMaximum()); }
  hdata->Draw("P E0 X0");
  
  // make data mc ratio                                                                           
  TH1F *h_r = (TH1F*)hdata->Clone("h_"+name+"_r"); h_r->Divide(hdata,h_sum);

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
  pt_info->AddText(("Year: " + era).c_str());
  pt_info->AddText(("WP: " + wprange).c_str());
  pt_info->AddText(("pt: " + ptrange).c_str());
  pt_info->AddText(("Cat: " + cat).c_str());

  // draw lumi header
  TLatex lumiHeader;
  std::string lumiHeaderText = "";
  if (era=="2016preVFP") { lumiHeaderText = "2016-preVFP, 19.52 fb^{-1}, 13 TeV"; }
  else if (era=="2016postVFP") { lumiHeaderText = "2016-postVFP, 16.81 fb^{-1}, 13 TeV"; }
  else if (era=="2017") { lumiHeaderText = "2017, 41.53 fb^{-1}, 13 TeV"; }
  else if (era=="2018") { lumiHeaderText = "2018, 59.74 fb^{-1}, 13 TeV"; }
  else if (era=="2022preEE") { lumiHeaderText = "2022-preEE, 7.98 fb^{-1}, 13.6 TeV"; }
  else if (era=="2022postEE") { lumiHeaderText = "2022-postEE, 26.67 fb^{-1}, 13.6 TeV"; }
  else if (era=="2023preBPix") { lumiHeaderText = "2023-preBPix, 18.08 fb^{-1}, 13.6 TeV"; }
  else if (era=="2023postBPix") { lumiHeaderText = "2023-postBPix, 9.69 fb^{-1}, 13.6 TeV"; }
  else if (era=="2024") { lumiHeaderText = "2024, 109.08 fb^{-1}, 13.6 TeV"; }
  else { lumiHeaderText = "<year>, xx fb^{-1}, yy TeV"; }
  lumiHeader.SetTextSize(0.06);
  lumiHeader.SetTextFont(42);
  lumiHeader.SetTextAlign(31);
  
  // upper pad: draw all objects in correct order
  hs->Draw("HIST E0 sames");
  hdata->Draw("P E0 X0 sames");
  leg->Draw("sames");
  pt_cms->Draw("sames");
  pt_preliminary->Draw("sames");
  pt_info->Draw("sames");
  lumiHeader.DrawLatexNDC(0.95, 0.92, lumiHeaderText.c_str());
  c_->RedrawAxis();
  pMain->RedrawAxis();

  // switch to lower pad
  pRatio->cd();

  // ratio hist layout
  h_r->SetMarkerSize(1.);

  // lower pad axis layout
  h_r->GetYaxis()->SetTitleOffset(0.7);
  h_r->GetYaxis()->SetTitleSize(0.1);
  h_r->GetYaxis()->SetTitle("Data / MC");
  h_r->GetYaxis()->SetLabelSize(0.1);
  h_r->GetXaxis()->SetTitleSize(0.15);
  h_r->GetXaxis()->SetTitleOffset(1.);
  h_r->GetXaxis()->SetTitle(xname);
  h_r->GetXaxis()->SetLabelSize(0.12);
  h_r->GetYaxis()->SetRangeUser(0.31,1.69);
  h_r->Draw("P E0 X0");

  // draw horizontal line at unit ratio
  Double_t xmin = h_r->GetXaxis()->GetXmin(); 
  Double_t xmax = h_r->GetXaxis()->GetXmax();
  TLine* line = new TLine(xmin,1,xmax,1);
  line->SetLineColor(kRed+2);
  line->SetLineStyle(9);
  line->SetLineWidth(2);
  line->Draw("same");

  // re-draw the data to bring it to the foreground
  h_r->Draw("same P E0 X0");
  return c_;  
}


TH1D *h1DHistoFrom2DTemplates(TString path2file, TString h2dname, TString name, double ymin, double ymax, int color, bool isdata) {
  // Make a 1D projection of a 2D histogram

  // general ROOT settings
  TH1::SetDefaultSumw2(kTRUE);
  setTDRStyle();
  gROOT->SetBatch(false);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);
  gStyle->SetPalette(1);

  // get the 2D histogram
  TFile *f2d = TFile::Open(path2file, "READONLY");
  TH2D *h2d = (TH2D*)f2d->Get(h2dname);
  if(!h2d){
      // if an object with the requested name is not found, return a nullptr
      std::string msg = "WARNING in h1DHistoFrom2DTemplates:";
      msg += " 2D histogram with name " + (std::string)h2dname + " requested,";
      msg += " but not found in file " + (std::string)path2file;
      std::cerr << msg << std::endl;
      TH1D* res;
      return res;
  }
  h2d->SetDirectory(0);

  // make the projection
  TH1D  *h1d = h2d->ProjectionX("h1d_"+h2dname, h2d->GetYaxis()->FindBin(ymin), h2d->GetYaxis()->FindBin(ymax), "e");
  h1d->SetDirectory(0);
  
  // settings for plotting
  h1d->SetName(h2dname+"_"+name);
  h1d->SetLineColor(color);
  if (isdata)
    {
      h1d->SetFillColor(0); 
      h1d->SetMarkerStyle(20); h1d->SetMarkerSize(1.2); 
      h1d->SetLineWidth(3); h1d->SetLineColor(color);
    }
  else
    {
      h1d->SetFillColor(color);
      h1d->SetMarkerStyle(20); h1d->SetMarkerSize(0.);
      h1d->SetLineWidth(3); h1d->SetLineColor(color);
    }
  
  f2d->Close();
  
  return h1d;
}
