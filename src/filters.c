/*################################################################################
##
##   R package rgarch by Alexios Ghalanos Copyright (C) 2009, 2010, 2011
##   This file is part of the R package rgarch.
##
##   The R package rgarch is free software: you can redistribute it and/or modify
##   it under the terms of the GNU General Public License as published by
##   the Free Software Foundation, either version 3 of the License, or
##   (at your option) any later version.
##
##   The R package rgarch is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.
##
#################################################################################*/
# include <R.h>
# include <math.h>
# include "filters.h"

void sgarchfilter(int *model, double *pars, int *idx, double *vexdata, double *e, int T, int i, double *h)
{
	int j;
	h[i] = h[i] + pars[idx[6]];
	if( model[14]>0 )
	{
		int ind=0;
		for( j=0; j<model[14]; j++ )
		{
			ind = i + ( T * j );
			h[i] = h[i] + pars[idx[14]+j]*vexdata[i];
		}
	}
	for( j=0; j<model[7]; j++ )
	{
		h[i] = h[i] + pars[idx[7]+j]*e[i-(j+1)];
	}
	for( j=0; j<model[8]; j++ )
	{
		h[i] = h[i] + pars[idx[8]+j]*h[i-(j+1)];
	}
}

void gjrgarchfilter(int *model, double *pars, int *idx, double *vexdata, double *nres, double *e, int T, int i, double *h)
{
	int j;
	h[i] = h[i] + pars[idx[6]];
	if( model[14]>0 )
	{
		int ind=0;
		for( j=0; j<model[14]; j++ )
		{
			ind = i + ( T * j );
			h[i] = h[i] + pars[idx[14]+j]*vexdata[i];
		}
	}
	for( j=0; j<model[7]; j++ )
	{
		h[i] = h[i] + pars[idx[7]+j]*e[i-(j+1)]+pars[idx[9]+j]*nres[i-(j+1)];
	}
	for( j=0; j<model[8]; j++ )
	{
		h[i] = h[i] + pars[idx[8]+j]*h[i-(j+1)];
	}
}

void aparchfilter(int *model, double *pars, int *idx, double *vexdata, double *res, int T, int i, double *h)
{
	int j;
	h[i] = h[i] + pars[idx[6]];
	if( model[14]>0 )
	{
		int ind=0;
		for( j=0; j<model[14]; j++ )
		{
			ind = i + ( T * j );
			h[i] = h[i] + pars[idx[14]+j]*vexdata[i];
		}
	}
	for( j=0; j<model[7]; j++ )
	{
		h[i]+= pars[idx[7]+j]*pow(fabs(res[i-(j+1)])-pars[idx[9]+j]*res[i-(j+1)], pars[idx[12]]);
	}
	for( j=0; j<model[8]; j++ )
	{
		h[i]+= pars[idx[8]+j]*pow( h[i-(j+1)], pars[idx[12]] );
	}
	h[i] = pow( h[i], 1.0/pars[idx[12]] );
}

void egarchfilter(int *model, double *pars, int *idx, double meanz, double *z, double *vexdata, int T, int i, double *h)
{
	int j;
	h[i] = h[i] +  pars[idx[6]];
	if( model[14]>0 )
	{
		int ind=0;
		for( j=0; j<model[14]; j++ )
		{
			ind = i + ( T * j );
			h[i] = h[i] + pars[idx[14]+j]*vexdata[i];
		}
	}
	for( j=0; j<model[7]; j++ )
	{

		h[i] = h[i] + pars[idx[7]+j]*z[i-(j+1)] + pars[idx[9]+j]*( fabs(z[i-(j+1)]) - meanz );
	}
	for( j=0; j<model[8]; j++ )
	{
		h[i] = h[i] +  pars[idx[8]+j]*log( h[i-(j+1)] );
	}
	h[i] = exp( h[i] );
}

void fgarchfilter(int *model, double *pars, int *idx, double kdelta, double *z, double *vexdata, int T, int i, double *h)
{
	int j;
	h[i] = h[i] +  pars[idx[6]];
	if( model[14]>0 )
	{
		int ind=0;
		for( j=0; j<model[14]; j++ )
		{
			ind = i + ( T * j );
			h[i] = h[i] + pars[idx[14]+j]*vexdata[i];
		}
	}
	for( j=0; j<model[7]; j++ )
	{
		h[i]= h[i] + pars[idx[7]+j] * ( pow(sqrt(pow(0.001,2) + pow( z[i-(j+1)] - pars[idx[11]+j],2) )- pars[idx[10]+j] *(z[i-(j+1)] - pars[idx[11]+j]), kdelta ) * pow( h[i-(j+1)], pars[idx[13]]) );
	}
	for( j=0; j<model[8]; j++ )
	{
		h[i]= h[i] + pars[idx[8]+j] * pow( h[i-(j+1)], pars[idx[13]] );
	}
	h[i] = pow( h[i], 1/pars[idx[13]] );
}


void arfimaxfilter(int* model, double *pars, int *idx, double *x, double *res,
		double *mexdata, double *zrf, double *constm, double *condm, double h,
		int m, int i, int T)
{
/* --------------------------------------------------------------------------------
 * ARFIMA Process :
 * (1-L)^(-darfima).e[t] = phi(1-L)(y[t] - mu[t]) - psi(L).e[t]
 * where mu[t] includes constant, external and arch-in-mean
 * L is the lag operator
 * --------------------------------------------------------------------------------
 * */
	/*0 constm, 1 condm, 2 res*/
	int j, k;
	constm[i] = pars[0];
	// GARCH-In-Mean Initialization (h is always the sigma, not sigma^2 so that h^model[4] is correct)
	if(model[4]>0)
	{
		constm[i]+=pars[idx[4]]*pow(h, model[4]);
	}
	// Exogenous Regressor Initialization
	if(model[5]>0)
	{
		int ind=0;
		for(k=0;k<model[5];k++)
		{
			ind=i+(T*k);
			constm[i]+=pars[idx[5]+k]*mexdata[ind];
		}
	}
	condm[i]+=constm[i];
	//ARMA initialization
	if(model[1]>0 || model[2]>0)
	{
		if(i<model[1])
		{
			condm[i]=x[i];
		}
		else
		{
			if(model[1]>0)
			{
				for(j=0; j<model[1];j++)
				{
					condm[i]+=pars[idx[1]+j]*(x[i-(j+1)]-constm[i-(j+1)]);
				}
			}
			if(model[2]>0)
			{
				for(j=0; j<model[2];j++)
				{
					if(i-j-1>=0)
					{
						condm[i]+=pars[idx[2]+j]*(x[i-(j+1)]-condm[i-(j+1)]);
					}
				}
			}
		}
	}
	res[i]=x[i]-condm[i];
	//arfima initialization
	if(model[3]>0)
	{
		if(i>0 && i<m)
		{
			double tmp=0;
			for(k=1;k<=i;k++)
			{
				tmp+=(zrf[i-k+1]*res[k-1]) ;
			}
			res[i]=-1.0 * tmp;
		}
		if(i>0 && i>=m)
		{
			double tmp=0;
			for(k=i;k>0;k--)
			{
				// quicker to count down
				tmp+=zrf[k]*(x[i-k] - condm[i-k]) ;
			}
			res[i]+= tmp;
		}
	}
}

/* We are passing (1-sum(ar))*mu in constm (this is not the case in fracsim).
 * r[t] = (1-ar)*mu[t] + ar*(r[t-1]) + ma*(res[t-1]) + res[t]
 * r[t] - mu[t]  = ar*(r[t-1]-mu[t]) + ma*(res[t-1]) + res[t]
 * */
void armaxsim(int *model, double *pars, int *idx, double *x, double *res, double *constm, int *m, int *T)
{
	int j, i;
	for(i=*m; i<*T; i++)
	{
		x[i] = constm[i];
		for( j=0; j<model[1]; j++ )
		{
			x[i]+= pars[idx[1]+j] * (x[i-(j+1)] - constm[i-(j+1)]);
		}
		for ( j=0; j<model[2]; j++ )
		{
			x[i]+= pars[idx[2]+j] * res[i-(j+1)];
		}
		x[i]+= res[i];
	}
}
