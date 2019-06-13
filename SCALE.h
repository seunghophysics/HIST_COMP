#ifndef _SCALE_
#define _SCALE_

#include "TH1.h"

class RMS
{
	public:
	  
		TH1D *h_mc, *h_da;

		double operator()(double x);
};

double coefficient(TH1D *h1, TH1D *h2);

#endif
