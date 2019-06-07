#include "TH1.h"
#include "Math/Functor.h"
#include "Math/BrentMinimizer1D.h"


double RMS::operator()(double x)
{
	TH1D *h1 = (TH1D*)h_mc->Clone();
	TH1D *h2 = (TH1D*)h_da->Clone();
	
	double diff;
	double var = 0.0;

	for(int i = 0; i < h1->GetNbinsX(); i++){
		diff = h2->GetBinContent(i) - x * h1->GetBinContent(i);
		var += diff * diff;
	}
	
	double rms = sqrt(var)/h2->GetMaximum();
	
	//cout << "scale: " << x << " rms: " << rms << endl;

	return rms;
}


double coefficient(TH1D *h1, TH1D *h2)
{
	RMS r;

	r.h_mc = (TH1D*)h1->Clone();
	r.h_da = (TH1D*)h2->Clone();
	
	ROOT::Math::Functor1D func(r);
	
	ROOT::Math::BrentMinimizer1D bm;
	
	bm.SetFunction(func, 0, 10);
	bm.SetNpx(1000);
	bm.Minimize(100);
	
	cout << "Iterations: " << bm.Iterations() << endl;
	cout << "Best-fit scale: " << bm.XMinimum() << endl;
	cout << "Minimum RMS: " << bm.FValMinimum() << endl;
	
	return bm.XMinimum();	  
}



