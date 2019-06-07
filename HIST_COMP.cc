#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <cmath>

#include "SCALE.h"

#include "TStyle.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TROOT.h"

//	Superimpose histograms from MC and Data outputs onto a canvas
//	Author: HAN, Seungho
//	han@icrr.u-tokyo.ac.jp

using namespace std;


// 	Canvas with two histograms piled up
TCanvas *canvas_pile(string h_name, TH1D *h1, TH1D *h2)
{
	gStyle->SetOptStat(0);
	TCanvas *c_pile = new TCanvas(h_name.c_str());

	double h1_max = h1->GetMaximum();
	double h2_max = h2->GetMaximum();
	double h1_min = h1->GetMinimum();
	double h2_min = h2->GetMinimum();

	double h_min = h1_min < h2_min ? h1_min : h2_min;
	double h_max = h1_max > h2_max ? h1_max : h2_max;

	// 	Setting the appropriate Y range for the histograms
	//h1->GetYaxis()->SetRangeUser(h_min * 0.8, h_max * 1.2);
	//h2->GetYaxis()->SetRangeUser(h_min * 0.8, h_max * 1.2);
	
	h1->SetLineColor(kRed);
	h2->SetLineColor(kBlue);
	
	//TH1D *h_ratio = h2->Clone();
	//h_ratio->Divide(h1);

	//double mean = h_ratio->GetMean();
	//double stdev = h_ratio->GetRMS();

	//cout << "mean: " << mean << " stdev: " << stdev << endl;
	h2->SetLineWidth(3);
	h2->Draw("hist");
	//h1->Draw("same hist");

	TLegend *legend = new TLegend(0.7, 0.6, 0.9, 0.9);
	//legend->AddEntry(h1, "MC", "l");
	legend->AddEntry(h2, "Data", "l");

	if(h2->GetMaximum() > 0.0){
		TH1D *h3 = (TH1D*)h1->Clone();
		TH1D *h_err = (TH1D*)h2->Clone();
		
		//cout << "h1: " << h1 << " h2: " << h2 << endl;
		h3->Scale(coefficient(h1, h2));
		cout << "hname: " << h_name << endl;
		cout << "data max: " << h2->GetMaximum() << " mc max: " << h1->GetMaximum() << endl;
		cout << "data max bin: " << h2->GetMaximumBin() << " mc max bin: " << h1->GetMaximumBin() << endl;
		
		h3->SetLineColor(kRed);
		h3->SetLineWidth(2);
		h3->SetLineStyle(2);
		h_err->Add(h1, -1);
	
		h3->Draw("same hist");
		//h2->Draw("same hist");
		legend->AddEntry(h3, "Normalized MC", "l");
		h1->SetStats(0);
		h2->SetStats(0);
		h3->SetStats(0);
		c_pile->Update();

	//	TLatex *l = new TLatex();
	//	l->DrawLatex(2, 0.3, Form("Error size: %f", h_err->GetRMS()));
	}

	legend->Draw();
	gStyle->SetOptStat(0);

	return c_pile;
}



// 	Write MC and Data histograms in each directory
void write_histograms(string *names, TDirectory *MC, TDirectory *DA)
{
	for(int i = 0; i < 28; i++){
		TH1D *h_mc = (TH1D*)MC->Get(names[i].c_str());
		TH1D *h_da = (TH1D*)DA->Get(names[i].c_str());
	
		TCanvas *c_pile = canvas_pile(names[i], h_mc, h_da);
		c_pile->Write();
	}
}



void hist_compare()
{
	gROOT->SetBatch(1);	//	Omit graphics
	
	TFile *MC_ROOT = TFile::Open("../mc.Dec2016.af075512.cntr.root");	//	MC
	TFile *DA_ROOT = TFile::Open("../da.Dec2016.075502.cntr.root");	//	Data

	//	InVarsPS: distributions of variables before Neural Net
	//	InvarsNN: distributions of variables after Neural Net
	//	OtherVars: other variables that are not used for Neural Net
	TDirectory *MC_PS = MC_ROOT->GetDirectory("InVarsPS"); 
	TDirectory *DA_PS = DA_ROOT->GetDirectory("InVarsPS");
	TDirectory *MC_NN = MC_ROOT->GetDirectory("InVarsNN"); 
	TDirectory *DA_NN = DA_ROOT->GetDirectory("InVarsNN");
	TDirectory *MC_XX = MC_ROOT->GetDirectory("OtherVars");
	TDirectory *DA_XX = DA_ROOT->GetDirectory("OtherVars");
	
	//	Names of the histograms
	string hist_mcn[14] = {"hN10n_mcn", "hNcS_mcn", "hN300S_mcn", "hnDwall_mcn", "hnTwall_mcn", "hlnAccep_mcn", "hllrCAng_mcn", "hBeta1_mcn", "hBeta2_mcn", "hBeta3_mcn", "hBeta4_mcn", "hBeta5_mcn", "htSD_mcn", "htSDMin6_mcn"};
	string hist_bkg[14];
	string hist_name[28];

	//	Setting the array "hist_name" 
	for(int i = 0; i < 14; i++){
		hist_mcn[i] = hist_mcn[i].substr(0, hist_mcn[i].length() - 4);
		hist_bkg[i] = hist_mcn[i];
		hist_bkg[i] += "_bkg";
		hist_mcn[i] += "_mcn";
		
		hist_name[i] = hist_mcn[i];
		hist_name[i + 14] = hist_bkg[i];
	}
	
	// 	Output
	TFile *out = new TFile("output.root", "recreate");
	
	TDirectory *PS_OUT = out->mkdir("InVarsPS");
	TDirectory *NN_OUT = out->mkdir("InVarsNN");
	
	// 	Write histograms
	PS_OUT->cd();
	write_histograms(hist_name, MC_PS, DA_PS);
	
	NN_OUT->cd();
	write_histograms(hist_name, MC_NN, DA_NN);

	out->Close();
}


int main()
{
	hist_compare();
}


