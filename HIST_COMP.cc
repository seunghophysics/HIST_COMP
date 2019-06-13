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
	TCanvas *c_pile = new TCanvas(h_name.c_str());

	h1->SetLineColor(kRed);
	//h1->SetLineWidth(2);
	h1->SetLineStyle(2);
	
	h1->SetStats(0);
	h2->SetStats(0);

	h2->SetLineColor(kBlue);
	//h2->SetLineWidth(3);

	TLegend *legend = new TLegend(0.6, 0.6, 0.85, 0.85);

	h1->GetXaxis()->SetTitle(h_name.substr(1).c_str());
	h1->Draw("hist");
	h2->Draw("same hist");

	legend->AddEntry(h2, "Data", "l");
	legend->AddEntry(h1, "Normalized MC", "l");
	
	legend->Draw();
	c_pile->Update();

	return c_pile;
}



// 	Write MC and Data histograms in each directory
void write_histograms(vector<string> var, TDirectory *MC, TDirectory *DA, const char* option = "")
{
	TH1D *h_mc_mcn, *h_mc_bkg, *h_mc, *h_da;
	int number_of_hists = var.size();
	
	TCanvas *c_pile[number_of_hists];

	for(int i = 0; i < number_of_hists; i++){
		if(!strcmp(option, "other")){
			h_mc = (TH1D*)MC->Get((var[i] + "_all").c_str());
			h_da = (TH1D*)DA->Get((var[i] + "_all").c_str());
		}
		else{
			h_mc_mcn = (TH1D*)MC->Get((var[i] + "_mcn").c_str());
			h_mc_bkg = (TH1D*)MC->Get((var[i] + "_bkg").c_str());
	
			h_da = (TH1D*)DA->Get((var[i] + "_bkg").c_str());
	
			h_mc = (TH1D*)h_mc_mcn->Clone();
			h_mc->Add(h_mc_bkg);
		}
	
		// 	Normalization
		if(h_mc->GetEntries() > 0)
			h_mc->Scale(h_da->GetEntries() / h_mc->GetEntries());

		c_pile[i] = canvas_pile(var[i], h_mc, h_da);
		c_pile[i]->Write();
	}
	
	// 	Combine 4 canvases into 1
	for(int i = 0; i < number_of_hists / 4 + 1; i++){
		TCanvas *C = new TCanvas(Form("%s_canvas_%d", option, i + 1));
		C->Divide(2, 2);
		for(int j = 0; j < 4; j++){
			C->cd(j + 1);
			if(4 * i + j < number_of_hists)
				c_pile[4 * i + j]->DrawClonePad();
		}
		C->Write();
		C->SaveAs(Form("pdf/%s_canvas_%d.pdf", option, i + 1));
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
	string i_var[14] = {"hN10n", "hNcS", "hN300S", "hnDwall", "hnTwall", "hlnAccep", "hllrCAng", "hBeta1", "hBeta2", "hBeta3", "hBeta4", "hBeta5", "htSD", "htSDMin6"};
	string o_var[17] = {"hnp", "hnn", "hn10", "hnnout", "hctime_ps", "hctime_nn", "hnpdist_ps", "hnpdist_nn", "hmcttotmcn", "hmctnpdist_ps", "hmctnpdist_nn", "hnvtxdist_ps", "hnvtxdist_nn", "hmctdt0_ps", "hmctdt0_nn", "hctimeL_ps", "hctimeL_nn"};

	vector<string> in_var(i_var, i_var + 14);
	vector<string> other_var(o_var, o_var + 17);

	// 	Output
	TFile *out = new TFile("output.root", "recreate");
	
	TDirectory *PS_OUT = out->mkdir("InVarsPS");
	TDirectory *NN_OUT = out->mkdir("InVarsNN");
	TDirectory *XX_OUT = out->mkdir("OtherVars");
	TDirectory *PS_NN = out->mkdir("PS_NN_COMPARISON");

	// 	Write histograms
	PS_OUT->cd();
	write_histograms(in_var, MC_PS, DA_PS, "PS");
	
	NN_OUT->cd();
	write_histograms(in_var, MC_NN, DA_NN, "NN");

	XX_OUT->cd();
	write_histograms(other_var, MC_XX, DA_XX, "other");
	
	TCanvas *C_PS, *C_NN;

	for(int i = 0; i < 14; i++){
		TCanvas *C = new TCanvas(in_var[i].c_str());

		C_PS = (TCanvas*)PS_OUT->Get(in_var[i].c_str());
		C_NN = (TCanvas*)NN_OUT->Get(in_var[i].c_str());
		
		C->Divide(2);

		C->cd(1);
		C_PS->DrawClonePad();
		
		C->cd(2);
		C_NN->DrawClonePad();
	}

	PS_NN->cd();
	C->Write();

	out->Close();
}


int main()
{
	hist_compare();
	return 0;
}


