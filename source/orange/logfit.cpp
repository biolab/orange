/*
    This file is part of Orange.

    Orange is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Orange is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Orange; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Authors: Martin Mozina, Janez Demsar, Blaz Zupan, 1996--2003
    Contact: janez.demsar@fri.uni-lj.si
*/

#include "logfit.ppp"
#include "converts.hpp"
#include "../external/logreg/logreg.hpp"

TLogisticFitterMinimization::TLogisticFitterMinimization()
{}

// set error values thrown by logistic fitter
const char *TLogisticFitterMinimization::errors[] =
	{"LogisticFitter: ngroups < 2, ndf < 0 -- not enough examples with so many attributes",
	               "LogisticFitter: n[i]<0",
				   "LogisticFitter: r[i]<0",
				   "LogisticFitter: r[i]>n[i]",
				   "LogisticFitter: constant variable",
				   "LogisticFitter: singularity",
				   "LogisticFitter: infinity in beta",
				   "LogisticFitter: no convergence" };


// function used only in Logistic fitter, that returns vector length n
// and filled with ones(1)
double *ones(int n) {
	// initialize vector
	double *ret = new double[n];

	// set values
	for (int i=0; i<n; i++) 
		ret[i]=1;
	return ret;
}


PFloatList TLogisticFitterMinimization::operator ()(PExampleGenerator gen, PFloatList &beta_se, float &likelihood) {
	// get all needed/necessarily attributes and set
	LRInput input = LRInput();
	LRInfo O = LRInfo();


	// fill input data
	input.data = generateDoubleXMatrix(gen, input.nn, input.k);
	input.success = generateDoubleYVector(gen);
	input.trials = ones(input.nn+1);

	// initialize output data
	O.nn = input.nn;
	O.k = input.k;
	O.beta = new double[input.k+1];
	O.se_beta = new double[input.k+1];
	O.fit = new double[input.nn+1]; 
	O.stdres = new double[input.nn+1]; 
	O.cov_beta = new double*[input.k+1]; 
	O.dependent = new int[input.k+1];
	int i;
	for(i = 0; i <= input.k; ++i) {
		O.cov_beta[i] = new double[input.k+1];
		O.dependent[i] = 0; // no dependence
	}

	// fit coefficients
	logistic(O.error, input.nn,input.data,input.k,input.success,input.trials,
		O.chisq, O.devnce, O.ndf, O.beta, O.se_beta,
		O.fit, O.cov_beta, O.stdres, O.dependent
	);

	// error at computing/fitting logistic regression model
	// error at computing/fitting logistic regression model
	if (O.error==7) 
		raiseWarning(errors[O.error-1]);
	else if (O.error>0) 
		raiseError(errors[O.error-1]);

	// tranfsorm *beta into a PFloatList
	PFloatList beta=PFloatList(mlnew TFloatList);
	beta_se=PFloatList(mlnew TFloatList);

	//TODO: obstaja konstruktor, ki pretvori iz navadnega arraya?
	for (i=0; i<input.k+1; i++) {
		beta->push_back(O.beta[i]);
		beta_se->push_back(O.se_beta[i]);
	}

	// Calculate likelihood
	likelihood = - O.devnce; // I am not sure if this is OK?. Added by: Martin Mozina, 9.10.2003

	return beta;
}


double **TLogisticFitter::generateDoubleXMatrix(PExampleGenerator gen, long &numExamples, long &numAttr) {
	double **matrix;
	// get number of instances and allocate number of rows
	numExamples=gen->numberOfExamples();
	numAttr=gen->domain->attributes->size();
	matrix = new double*[numExamples+1];

	// get number of attributes 
	// TODO
/*	PITERATE(TVarList, vli, gen->domain->attributes) {
		numAttr++;
	} */
	
	// copy gen to double matrix
	int n=0;
	matrix[n]= new double[numAttr+1];
	// iteration through examples
	PEITERATE(first, gen) {	  
		// row allocation
		matrix[n+1]= new double[numAttr+1];

		int at=0;
		// iteration through attributes
		PITERATE(TVarList, vli, gen->domain->attributes) {
			// copy att. value
			matrix[n+1][at+1]=(*first)[at].floatV;

			at++;
		}
		n++;
	}

	return matrix;
}

double *TLogisticFitter::generateDoubleYVector(PExampleGenerator gen) {
	// initialize vector
	double *Y = new double[gen->numberOfExamples()+1];

	// copy gen class to vector *Y
	int n=0;
	PEITERATE(ei, gen) {
		// copy class value
		Y[n+1]=(*ei).getClass().intV;

		n++;
	}    

	return Y;
}


LRInput::LRInput() {
	data=NULL;
	success=NULL;
	trials=NULL;
}

LRInput::~LRInput() {
	int i;
	if (data != NULL) {
		for (i=0; i <= nn; ++i)
			delete data[i];
		delete data;
	}
	if (success != NULL) {
		delete success;
	}
}

LRInfo::LRInfo() {
   beta = NULL;		
   se_beta = NULL;	
   fit = NULL;		
   cov_beta = NULL;	
   stdres = NULL;   
   dependent = NULL;
}

LRInfo::~LRInfo() {
	if (cov_beta!=NULL)
		for (int i = 0; i <= k; ++i)
			delete cov_beta[i];
	delete cov_beta;
	delete fit;
	delete beta;
	delete se_beta;
	delete stdres;
	delete dependent;
}

