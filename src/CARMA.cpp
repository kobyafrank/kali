#if defined __APPLE__ && defined __MACH__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include <sys/time.h>
#include <limits>
#include <omp.h>
#include <complex>
#include <cmath>
#include <mathimf.h>
#include <mkl_types.h>
#define MKL_Complex8 std::complex<float>
#define MKL_Complex16 std::complex<double>
#include <mkl.h>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "boost/numeric/odeint.hpp"
#include "Constants.hpp"
#include "CARMA.hpp"

//#define TIMEALL
//#define TIMEPER
//#define TIMEFINE
//#define DEBUG
//#define DEBUG_LNLIKE
//#define DEBUG_FUNC
//#define DEBUG_CHECKARMAPARAMS
//#define DEBUG_SETCARMA
//#define DEBUG_SETCARMA_C
//#define DEBUG_SOLVECARMA_F
//#define DEBUG_SOLVECARMA_Q
//#define DEBUG_FUNCTOR
//#define DEBUG_SETCARMA_DEEP
//#define DEBUG_BURNSYSTEM
//#define WRITE_BURNSYSTEM
//#define DEBUG_OBSSYSTEM
//#define DEBUG_OBS
//#define DEBUG_CTORCARMA
//#define DEBUG_DTORCARMA
//#define DEBUG_ALLOCATECARMA
//#define DEBUG_DEALLOCATECARMA
//#define DEBUG_DEALLOCATECARMA_DEEP
//#define DEBUG_RESETSTATE
//#define DEBUG_CALCLNLIKE
//#define DEBUG_CALCLNLIKE2

using namespace std;

double calcCARMALnLike(const vector<double> &x, vector<double>& grad, void *p2Args) {
	/*! Used for computing good regions */
	if (!grad.empty()) {
		#pragma omp simd
		for (int i = 0; i < x.size(); ++i) {
			grad[i] = 0.0;
			}
		}

	int threadNum = omp_get_thread_num();

	LnLikeArgs *ptr2Args = reinterpret_cast<LnLikeArgs*>(p2Args);
	LnLikeArgs Args = *ptr2Args;
	CARMA *Systems = Args.Systems;
	double LnLike = 0;

	#ifdef DEBUG_CALCLNLIKE2
	printf("calcLnLike - threadNum: %d; Location: ",threadNum);
	#endif

	if (Systems[threadNum].checkCARMAParams(const_cast<double*>(&x[0])) == 1) {
		LnLike = 0.0;
		} else {
		LnLike = -HUGE_VAL;
		}

	#ifdef DEBUG_CALCLNLIKE2
	printf("LnLike: %f\n",LnLike);
	fflush(0);
	#endif

	return LnLike;

	}

double calcCARMALnLike(double *walkerPos, void *func_args) {
	/*! Used for computing good regions */

	int threadNum = omp_get_thread_num();

	LnLikeArgs *ptr2Args = reinterpret_cast<LnLikeArgs*>(func_args);
	LnLikeArgs Args = *ptr2Args;
	CARMA* Systems = Args.Systems;
	double LnLike = 0;

	if (Systems[threadNum].checkCARMAParams(walkerPos) == 1) {

		#ifdef DEBUG_FUNC2
		printf("calcLnLike = threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; Systems[threadNum].get_p() + Systems[threadNum].get_q() + 1; dimNum++) {
			printf("%f ",walkerPos[dimNum]);
			}
		printf("\n");
		printf("calcLnLike - threadNum: %d; System good!\n",threadNum);
		#endif

		LnLike = 0.0;
		} else {

		#ifdef DEBUG_FUNC2
		printf("calcLnLike = threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; dimNum < Systems[threadNum].get_p() + Systems[threadNum].get_q() + 1; dimNum++) {
			printf("%f ",walkerPos[dimNum]);
			}
		printf("\n");
		printf("calcLnLike - threadNum: %d; System bad!\n",threadNum);
		#endif

		LnLike = -HUGE_VAL;
		}
	return LnLike;

	}

double calcLnLike(const vector<double> &x, vector<double>& grad, void *p2Args) {
	if (!grad.empty()) {
		#pragma omp simd
		for (int i = 0; i < x.size(); ++i) {
			grad[i] = 0.0;
			}
		}

	int threadNum = omp_get_thread_num();

	LnLikeArgs *ptr2Args = reinterpret_cast<LnLikeArgs*>(p2Args);
	LnLikeArgs Args = *ptr2Args;
	LnLikeData *ptr2Data = Args.Data;
	CARMA *Systems = Args.Systems;
	double LnLike = 0;

	if (Systems[threadNum].checkCARMAParams(const_cast<double*>(&x[0])) == 1) {
		Systems[threadNum].setCARMA(const_cast<double*>(&x[0]));
		Systems[threadNum].solveCARMA();
		Systems[threadNum].resetState();

		#ifdef DEBUG_CALCLNLIKE
		#pragma omp critical
		{
			printf("calcLnLike - threadNum: %d; walkerPos: ",threadNum);
			for (int dimNum = 0; dimNum < Systems[threadNum].get_p() + Systems[threadNum].get_q() + 1; dimNum++) {
				printf("%+17.16e ", x[dimNum]);
				}
			printf("\n");
			printf("calcLnLike - threadNum: %d; System good!\n",threadNum);
			printf("calcLnLike - threadNum: %d; dt\n",threadNum);
			printf("%DISPLAY\n",Systems[threadNum].get_dt());
			printf("\n");
			printf("calcLnLike - threadNum: %d; A\n",threadNum);
			Systems[threadNum].printA();
			printf("\n");
			printf("calcLnLike - threadNum: %d; w\n",threadNum);
			Systems[threadNum].printw();
			printf("\n");
			printf("calcLnLike - threadNum: %d; expw\n",threadNum);
			Systems[threadNum].printexpw();
			printf("\n");
			printf("calcLnLike - threadNum: %d; vr\n",threadNum);
			Systems[threadNum].printvr();
			printf("\n");
			printf("calcLnLike - threadNum: %d; vrInv\n",threadNum);
			Systems[threadNum].printvrInv();
			printf("\n");
			printf("calcLnLike - threadNum: %d; B\n",threadNum);
			Systems[threadNum].printB();
			printf("\n");
			printf("calcLnLike - threadNum: %d; C\n",threadNum);
			Systems[threadNum].printC();
			printf("\n");
			printf("calcLnLike - threadNum: %d; F\n",threadNum);
			Systems[threadNum].printF();
			printf("\n");
			printf("calcLnLike - threadNum: %d; D\n",threadNum);
			Systems[threadNum].printD();
			printf("\n");
			printf("calcLnLike - threadNum: %d; Q\n",threadNum);
			Systems[threadNum].printQ();
			printf("\n");
			printf("calcLnLike - threadNum: %d; Sigma\n",threadNum);
			Systems[threadNum].printSigma();
			printf("\n");
			printf("calcLnLike - threadNum: %d; X\n",threadNum);
			Systems[threadNum].printX();
			printf("\n");
			printf("calcLnLike - threadNum: %d; P\n",threadNum);
			Systems[threadNum].printP();
			printf("\n");
			fflush(0);
		}
		#endif

		LnLike = Systems[threadNum].computeLnLike(ptr2Data);

		#ifdef DEBUG_CALCLNLIKE
		#pragma omp critical
		{
			printf("calcLnLike - threadNum: %d; walkerPos: ",threadNum);
			for (int dimNum = 0; dimNum < Systems[threadNum].get_p() + Systems[threadNum].get_q() + 1; dimNum++) {
				printf("%+17.16e ", x[dimNum]);
				}
			printf("\n");
			printf("calcLnLike - threadNum: %d; X\n",threadNum);
			Systems[threadNum].printX();
			printf("\n");
			printf("calcLnLike - threadNum: %d; P\n",threadNum);
			Systems[threadNum].printP();
			printf("\n");
			printf("calcLnLike - threadNum: %d; LnLike: %f\n",threadNum,LnLike);
			printf("\n");
		}
		#endif

		} else {
		LnLike = -HUGE_VAL;
		}
	return LnLike;

	}

double calcLnLike(double *walkerPos, void *func_args) {

	int threadNum = omp_get_thread_num();

	LnLikeArgs *ptr2Args = reinterpret_cast<LnLikeArgs*>(func_args);
	LnLikeArgs Args = *ptr2Args;

	LnLikeData *Data = Args.Data;
	CARMA *Systems = Args.Systems;
	LnLikeData *ptr2Data = Data;
	double LnLike = 0;

	if (Systems[threadNum].checkCARMAParams(walkerPos) == 1) {

		Systems[threadNum].setCARMA(walkerPos);
		Systems[threadNum].solveCARMA();
		Systems[threadNum].resetState();

		#ifdef DEBUG_CALCLNLIKE2
		#pragma omp critical
		{
			printf("calcLnLike - threadNum: %d; walkerPos: ",threadNum);
			for (int dimNum = 0; dimNum < Systems[threadNum].get_p() + Systems[threadNum].get_q() + 1; dimNum++) {
				printf("%+17.16e ", walkerPos[dimNum]);
				}
			printf("\n");
			printf("calcLnLike - threadNum: %d; System good!\n",threadNum);
			printf("calcLnLike - threadNum: %d; dt\n",threadNum);
			printf("%DISPLAY\n",Systems[threadNum].get_dt());
			printf("\n");
			printf("calcLnLike - threadNum: %d; A\n",threadNum);
			Systems[threadNum].printA();
			printf("\n");
			printf("calcLnLike - threadNum: %d; w\n",threadNum);
			Systems[threadNum].printw();
			printf("\n");
			printf("calcLnLike - threadNum: %d; expw\n",threadNum);
			Systems[threadNum].printexpw();
			printf("\n");
			printf("calcLnLike - threadNum: %d; vr\n",threadNum);
			Systems[threadNum].printvr();
			printf("\n");
			printf("calcLnLike - threadNum: %d; vrInv\n",threadNum);
			Systems[threadNum].printvrInv();
			printf("\n");
			printf("calcLnLike - threadNum: %d; B\n",threadNum);
			Systems[threadNum].printB();
			printf("\n");
			printf("calcLnLike - threadNum: %d; C\n",threadNum);
			Systems[threadNum].printC();
			printf("\n");
			printf("calcLnLike - threadNum: %d; F\n",threadNum);
			Systems[threadNum].printF();
			printf("\n");
			printf("calcLnLike - threadNum: %d; D\n",threadNum);
			Systems[threadNum].printD();
			printf("\n");
			printf("calcLnLike - threadNum: %d; Q\n",threadNum);
			Systems[threadNum].printQ();
			printf("\n");
			printf("calcLnLike - threadNum: %d; Sigma\n",threadNum);
			Systems[threadNum].printSigma();
			printf("\n");
			printf("calcLnLike - threadNum: %d; X\n",threadNum);
			Systems[threadNum].printX();
			printf("\n");
			printf("calcLnLike - threadNum: %d; P\n",threadNum);
			Systems[threadNum].printP();
			printf("\n");
			fflush(0);
		}
		#endif

		LnLike = Systems[threadNum].computeLnLike(ptr2Data);

		#ifdef DEBUG_CALCLNLIKE2
		#pragma omp critical
		{
			printf("calcLnLike - threadNum: %d; walkerPos: ",threadNum);
			for (int dimNum = 0; dimNum < Systems[threadNum].get_p() + Systems[threadNum].get_q() + 1; dimNum++) {
				printf("%+17.16e ", walkerPos[dimNum]);
				}
			printf("\n");
			printf("calcLnLike - threadNum: %d; X\n",threadNum);
			Systems[threadNum].printX();
			printf("\n");
			printf("calcLnLike - threadNum: %d; P\n",threadNum);
			Systems[threadNum].printP();
			printf("\n");
			printf("calcLnLike - threadNum: %d; LnLike: %f\n",threadNum,LnLike);
			printf("\n");
		}
		#endif

		} else {
		LnLike = -HUGE_VAL;
		}

	return LnLike;
	}

void zeroMatrix(int nRows, int nCols, int* mat) {
	for (int colNum = 0; colNum < nCols; ++colNum) {
		#pragma omp simd
		for (int rowNum = 0; rowNum < nRows; ++rowNum) {
			mat[rowNum + nRows*colNum] = 0;
			}
		}
	}

void zeroMatrix(int nRows, int nCols, double* mat) {
	for (int colNum = 0; colNum < nCols; ++colNum) {
		#pragma omp simd
		for (int rowNum = 0; rowNum < nRows; ++rowNum) {
			mat[rowNum + nRows*colNum] = 0.0;
			}
		}
	}

void zeroMatrix(int nRows, int nCols, complex<double>* mat) {
	for (int colNum = 0; colNum < nCols; ++colNum) {
		#pragma omp simd
		for (int rowNum = 0; rowNum < nRows; ++rowNum) {
			mat[rowNum + nRows*colNum] = 0.0;
			}
		}
	}

void viewMatrix(int nRows, int nCols, int* mat) {
	for (int i = 0; i < nRows; i++) {
		for (int j = 0; j < nCols; j++) {
			printf("%+d ",mat[j*nCols + i]);
			}
		printf("\n");
		}
	}

void viewMatrix(int nRows, int nCols, double* mat) {
	for (int i = 0; i < nRows; i++) {
		for (int j = 0; j < nCols; j++) {
			printf("%+8.7e ",mat[j*nCols + i]);
			}
		printf("\n");
		}
	}

void viewMatrix(int nRows, int nCols, vector<double> mat) {
	for (int i = 0; i < nRows; i++) {
		for (int j = 0; j < nCols; j++) {
			printf("%+8.7e ",mat[j*nCols + i]);
			}
		printf("\n");
		}
	}

void viewMatrix(int nRows, int nCols, complex<double>* mat) {
	for (int i = 0; i < nRows; i++) {
		for (int j = 0; j < nCols; j++) {
			printf("%+8.7e%+8.7ei ",mat[j*nCols + i].real(),mat[j*nCols + i].imag());
			}
		printf("\n");
		}
	}

double dtime() {
	double tseconds = 0.0;
	struct timeval mytime;
	gettimeofday(&mytime,(struct timezone*)0);
	tseconds = (double)(mytime.tv_sec + mytime.tv_usec*1.0e-6);
	return( tseconds );
	}

void kron(int m, int n, double* A, int p, int q, double* B, double* C) {
	int alpha = 0;
	int beta = 0;
	int mp = m*p;
	int nq = n*q;
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			for (int k = 0; k < p; k++) {
				#pragma omp simd
				for (int l = 0; l < q; l++) {
					alpha = p*i + k;
					beta = q*j + l;
					C[alpha + beta*nq] = A[i + j*n]*B[k + l*q];
					}
				}
			}
		}
	}

void expm(double xi, double* out) {
	/*#pragma omp simd
	for (int i = 0; i < p; ++i) {
		expw[i + i*p] = exp(dt*w[i]);
		}

	complex<double> alpha = 1.0+0.0i, beta = 0.0+0.0i;
	cblas_zgemm3m(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, &alpha, vr, p, expw, p, &beta, A, p);
	cblas_zgemm3m(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, &alpha, A, p, vrInv, p, &beta, AScratch, p);

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			out[rowCtr + colCtr*p] = FScratch[rowCtr + colCtr*p];
			}
		}*/
	}

CARMA::CARMA() {
	/*! Object that holds data and methods for performing C-ARMA analysis. DLM objects hold pointers to blocks of data that are set as required based on the size of the C-ARMA model.*/
	#ifdef DEBUG_CTORDLM
	int threadNum = omp_get_thread_num();
	printf("DLM - threadNum: %d; Address of System: %p\n",threadNum,this);
	#endif

	allocated = 0;
	isStable = 1;
	isInvertible = 1;
	isNotRedundant = 1;
	hasUniqueEigenValues = 1;
	hasPosSigma = 1;
	p = 0;
	q = 0;
	pSq = 0;
	dt = 0.0;

	ilo = nullptr; // len 1
	ihi = nullptr; // len 1
	abnrm = nullptr; // len 1

	// Arrays used to compute expm(A dt)
	w = nullptr; // len p
	expw = nullptr; // len pSq
	CARMatrix = nullptr; // len pSq
	CMAMatrix = nullptr; // len qSq
	CARw = nullptr; // len p
	CMAw = nullptr; //len p
	scale = nullptr;
	vr = nullptr;
	vrInv = nullptr;
	rconde = nullptr;
	rcondv = nullptr;
	ipiv = nullptr;

	Theta = nullptr;
	A = nullptr;
	ACopy = nullptr;
	AScratch = nullptr;
	AScratch2 = nullptr;
	B = nullptr;
	BScratch = nullptr;
	C = nullptr;
	I = nullptr;
	F = nullptr;
	Sigma = nullptr;
	//D = nullptr;
	Q = nullptr;
	T = nullptr;
	H = nullptr;
	R = nullptr;
	K = nullptr;
	X = nullptr;
	P = nullptr;
	XMinus = nullptr;
	PMinus = nullptr;
	VScratch = nullptr;
	MScratch = nullptr;

	#ifdef DEBUG_CTORDLM
	printf("DLM - threadNum: %d; Address of System: %p\n",threadNum,this);
	#endif

	}

CARMA::~CARMA() {

	#ifdef DEBUG_DTORDLM
	int threadNum = omp_get_thread_num();
	printf("~DLM - threadNum: %d; Address of System: %p\n",threadNum,this);
	#endif

	allocated = 0;
	isStable = 1;
	isInvertible = 1;
	isNotRedundant = 1;
	hasUniqueEigenValues = 1;
	hasPosSigma = 1;
	p = 0;
	q = 0;
	pSq = 0;
	dt = 0.0;

	ilo = nullptr;
	ihi = nullptr;
	abnrm = nullptr;
	w = nullptr;
	expw = nullptr;
	CARMatrix = nullptr;
	CMAMatrix = nullptr;
	CARw = nullptr;
	CMAw = nullptr;
	scale = nullptr;
	vr = nullptr;
	vrInv = nullptr;
	rconde = nullptr;
	rcondv = nullptr;
	ipiv = nullptr;

	Theta = nullptr;
	A = nullptr;
	ACopy = nullptr;
	AScratch = nullptr;
	AScratch2 = nullptr;
	B = nullptr;
	BScratch = nullptr;
	C = nullptr;
	I = nullptr;
	F = nullptr;
	//D = nullptr;
	Sigma = nullptr;
	Q = nullptr;
	T = nullptr;
	H = nullptr;
	R = nullptr;
	K = nullptr;
	X = nullptr;
	P = nullptr;
	XMinus = nullptr;
	PMinus = nullptr;
	VScratch = nullptr;
	MScratch = nullptr;

	#ifdef DEBUG_DTORDLM
	printf("~DLM - threadNum: %d; Address of System: %p\n",threadNum,this);
	#endif

	}

void CARMA::allocCARMA(int numP, int numQ) {

	#ifdef DEBUG_ALLOCATECARMA
	int threadNum = omp_get_thread_num();
	printf("allocDLM - threadNum: %d; Starting... Address of System: %p\n",threadNum,this);
	#endif

	if ((numQ >= numP) or (numQ < 0)) {
		printf("FATAL LOGIC ERROR: numP MUST BE > numQ >= 0!\n");
		exit(1);
		}
	p = numP;
	q = numQ;
	allocated = 0;
	pSq = p*p;
	qSq = q*q;

	#ifdef DEBUG_ALLOCATECARMA
	printf("allocDLM - threadNum: %d; Allocating ilo, ihi, abnrm, ipiv, scale, rconde, rcondv Address of System: %p\n",threadNum,this);
	#endif

	ilo = static_cast<lapack_int*>(_mm_malloc(1*sizeof(lapack_int),64));
	ihi = static_cast<lapack_int*>(_mm_malloc(1*sizeof(lapack_int),64));
	allocated += 2*sizeof(lapack_int);

	ilo[0] = 0;
	ihi[0] = 0;

	abnrm = static_cast<double*>(_mm_malloc(1*sizeof(double),64));
	allocated += sizeof(double);

	abnrm[0] = 0.0;

	ipiv = static_cast<lapack_int*>(_mm_malloc(p*sizeof(lapack_int),64));
	allocated += p*sizeof(lapack_int);

	scale = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	rconde = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	rcondv = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	allocated += 3*p*sizeof(double);

	#pragma omp simd
	for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
		ipiv[rowCtr] = 0;
		scale[rowCtr] = 0.0;
		rconde[rowCtr] = 0.0;
		rcondv[rowCtr] = 0.0;
		}

	#ifdef DEBUG_ALLOCATECARMA
	printf("allocDLM - threadNum: %d; Allocating w, CARw, B, BScratch Address of System: %p\n",threadNum,this);
	#endif

	w = static_cast<complex<double>*>(_mm_malloc(p*sizeof(complex<double>),64));
	CARw = static_cast<complex<double>*>(_mm_malloc(p*sizeof(complex<double>),64));
	B = static_cast<complex<double>*>(_mm_malloc(p*sizeof(complex<double>),64));
	BScratch = static_cast<complex<double>*>(_mm_malloc(p*sizeof(complex<double>),64));
	allocated += 4*p*sizeof(complex<double>);

	if (q > 0) {

		#ifdef DEBUG_ALLOCATECARMA
		printf("allocDLM - threadNum: %d; Allocating CMAw, CMAMatrix Address of System: %p\n",threadNum,this);
		#endif

		CMAw = static_cast<complex<double>*>(_mm_malloc(q*sizeof(complex<double>),64));
		CMAMatrix = static_cast<complex<double>*>(_mm_malloc(qSq*sizeof(complex<double>),64));
		allocated += q*sizeof(complex<double>);
		allocated += qSq*sizeof(complex<double>);

		for (int colCtr = 0; colCtr < q; ++colCtr) {
			CMAw[colCtr] = complexZero;
			#pragma omp simd
			for (int rowCtr = 0; rowCtr < q; ++rowCtr) {
				CMAMatrix[rowCtr + colCtr*q] = complexZero;
				}
			}
		}

	#ifdef DEBUG_ALLOCATECARMA
	printf("allocDLM - threadNum: %d; Allocating CARMatrix, expw, vr, vrInv, A, ACopy, AScratch, AScratch2 Address of System: %p\n",threadNum,this);
	#endif

	CARMatrix = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	expw = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	vr = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	vrInv = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	A = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	C = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	ACopy = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	AScratch = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	AScratch2 = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	//allocated += 7*pSq*sizeof(complex<double>);
	allocated += 8*pSq*sizeof(complex<double>);

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		w[colCtr] = complexZero;
		CARw[colCtr] = complexZero;
		B[colCtr] = 0.0;
		BScratch[colCtr] = 0.0;
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			CARMatrix[rowCtr + colCtr*p] = complexZero;
			expw[rowCtr + colCtr*p] = complexZero;
			vr[rowCtr + colCtr*p] = complexZero;
			vrInv[rowCtr + colCtr*p] = complexZero;
			A[rowCtr + colCtr*p] = complexZero;
			C[rowCtr + colCtr*p] = complexZero;
			ACopy[rowCtr + colCtr*p] = complexZero;
			AScratch[rowCtr + colCtr*p] = complexZero;
			AScratch2[rowCtr + colCtr*p] = complexZero;
			}
		}

	Theta = static_cast<double*>(_mm_malloc((p + q + 1)*sizeof(double),64));
	allocated += (p+q+1)*sizeof(double);

	for (int rowCtr = 0; rowCtr < p + q + 1; ++rowCtr) {
		Theta[rowCtr] = 0.0;
		}

	#ifdef DEBUG_ALLOCATECARMA
	printf("allocDLM - threadNum: %d; Allocating H, K, X, XMinus, VScratch, I, F, Sigma, Q, T, P, PMinus, MScratch Address of System: %p\n",threadNum,this);
	#endif

	//D = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	H = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	K = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	X = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	XMinus = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	VScratch = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	//allocated += 6*p*sizeof(double);
	allocated += 5*p*sizeof(double);

	I = static_cast<double*>(_mm_malloc(pSq*sizeof(double),64));
	F = static_cast<double*>(_mm_malloc(pSq*sizeof(double),64));
	Sigma = static_cast<double*>(_mm_malloc(pSq*sizeof(double),64));
	Q = static_cast<double*>(_mm_malloc(pSq*sizeof(double),64));
	T = static_cast<double*>(_mm_malloc(pSq*sizeof(double),64));
	P = static_cast<double*>(_mm_malloc(pSq*sizeof(double),64));
	PMinus = static_cast<double*>(_mm_malloc(pSq*sizeof(double),64));
	MScratch = static_cast<double*>(_mm_malloc(pSq*sizeof(double),64));
	allocated += 7*pSq*sizeof(double);

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		//D[colCtr] = 0.0;
		H[colCtr] = 0.0;
		K[colCtr] = 0.0;
		X[colCtr] = 0.0;
		XMinus[colCtr] = 0.0;
		VScratch[colCtr] = 0.0;
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			I[rowCtr + colCtr*p] = 0.0;
			F[rowCtr + colCtr*p] = 0.0;
			Sigma[rowCtr + colCtr*p] = 0.0;
			Q[rowCtr + colCtr*p] = 0.0;
			T[rowCtr + colCtr*p] = 0.0;
			P[rowCtr + colCtr*p] = 0.0;
			PMinus[rowCtr + colCtr*p] = 0.0;
			MScratch[rowCtr + colCtr*p] = 0.0;
			}
		}

	#ifdef DEBUG_ALLOCATECARMA
	printf("allocDLM - threadNum: %d; Allocating R Address of System: %p\n",threadNum,this);
	#endif

	R = static_cast<double*>(_mm_malloc(sizeof(double),64));
	allocated += sizeof(double);

	R[0] = 0.0;

	#pragma omp simd
	for (int i = 1; i < p; ++i) {
		A[i*p + (i - 1)] = complexOne;
		I[(i - 1)*p + (i - 1)] = 1.0;
		}
	I[(p - 1)*p + (p - 1)] = 1.0;

	#ifdef DEBUG_ALLOCATECARMA
	printf("allocDLM - threadNum: %d; Finishing... Address of System: %p\n",threadNum,this);
	#endif

	}

void CARMA::deallocCARMA() {

	#ifdef DEBUG_DEALLOCATECARMA
	int threadNum = omp_get_thread_num();
	printf("deallocDLM - threadNum: %d; Starting... Address of System: %p\n",threadNum,this);
	#endif

	if (ilo) {
		_mm_free(ilo);
		ilo = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated ilo Address of System: %p\n",threadNum,this);
	#endif

	if (ihi) {
		_mm_free(ihi);
		ihi = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated ihi Address of System: %p\n",threadNum,this);
	#endif

	if (abnrm) {
		_mm_free(abnrm);
		abnrm = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated abnrm Address of System: %p\n",threadNum,this);
	#endif

	if (ipiv) {
		_mm_free(ipiv);
		ipiv = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated ipiv Address of System: %p\n",threadNum,this);
	#endif

	if (scale) {
		_mm_free(scale);
		scale = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated scale Address of System: %p\n",threadNum,this);
	#endif

	if (rconde) {
		_mm_free(rconde);
		rconde = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated rconde Address of System: %p\n",threadNum,this);
	#endif

	if (rcondv) {
		_mm_free(rcondv);
		rcondv = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated rcondv Address of System: %p\n",threadNum,this);
	#endif

	if (w) {
		_mm_free(w);
		w = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated w Address of System: %p\n",threadNum,this);
	#endif

	if (CARw) {
		_mm_free(CARw);
		CARw = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated CARw Address of System: %p\n",threadNum,this);
	#endif

	if (B) {
		_mm_free(B);
		B = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated B Address of System: %p\n",threadNum,this);
	#endif

	if (BScratch) {
		_mm_free(BScratch);
		BScratch = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated BScratch Address of System: %p\n",threadNum,this);
	#endif

	if (CMAw) {
		_mm_free(CMAw);
		CMAw = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated CMAw Address of System: %p\n",threadNum,this);
	#endif

	if (CMAMatrix) {
		_mm_free(CMAMatrix);
		CMAMatrix = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated CMAMatrix Address of System: %p\n",threadNum,this);
	#endif

	if (CARMatrix) {
		_mm_free(CARMatrix);
		CARMatrix = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated CARMatrix Address of System: %p\n",threadNum,this);
	#endif

	if (expw) {
		_mm_free(expw);
		expw = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated expw Address of System: %p\n",threadNum,this);
	#endif

	if (vr) {
		_mm_free(vr);
		vr = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated vr Address of System: %p\n",threadNum,this);
	#endif

	if (vrInv) {
		_mm_free(vrInv);
		vrInv = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated vrInv Address of System: %p\n",threadNum,this);
	#endif

	if (A) {
		_mm_free(A);
		A = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated A Address of System: %p\n",threadNum,this);
	#endif

	if (ACopy) {
		_mm_free(ACopy);
		ACopy = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated ACopy Address of System: %p\n",threadNum,this);
	#endif

	if (AScratch) {
		_mm_free(AScratch);
		AScratch = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated AScratch Address of System: %p\n",threadNum,this);
	#endif

	if (AScratch2) {
		_mm_free(AScratch2);
		AScratch2 = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated AScratch2 Address of System: %p\n",threadNum,this);
	#endif

	if (C) {
		_mm_free(C);
		C = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated C Address of System: %p\n",threadNum,this);
	#endif

	if (Theta) {
		_mm_free(Theta);
		Theta = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated Theta Address of System: %p\n",threadNum,this);
	#endif

	/*if (D) {
		_mm_free(D);
		D = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated D Address of System: %p\n",threadNum,this);
	#endif*/

	if (H) {
		_mm_free(H);
		H = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated H Address of System: %p\n",threadNum,this);
	#endif

	if (K) {
		_mm_free(K);
		K = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated K Address of System: %p\n",threadNum,this);
	#endif

	if (X) {
		_mm_free(X);
		X = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated X Address of System: %p\n",threadNum,this);
	#endif

	if (XMinus) {
		_mm_free(XMinus);
		XMinus = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated XMinus Address of System: %p\n",threadNum,this);
	#endif

	if (VScratch) {
		_mm_free(VScratch);
		VScratch = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated VScratch Address of System: %p\n",threadNum,this);
	#endif

	if (I) {
		_mm_free(I);
		I = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated I Address of System: %p\n",threadNum,this);
	#endif

	if (F) {
		_mm_free(F);
		F = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated F Address of System: %p\n",threadNum,this);
	#endif

	if (Sigma) {
		_mm_free(Sigma);
		Sigma = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated Sigma Address of System: %p\n",threadNum,this);
	#endif

	if (Q) {
		_mm_free(Q);
		Q = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated Q Address of System: %p\n",threadNum,this);
	#endif

	if (T) {
		_mm_free(T);
		T = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated T Address of System: %p\n",threadNum,this);
	#endif

	if (P) {
		_mm_free(P);
		P = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated P Address of System: %p\n",threadNum,this);
	#endif

	if (PMinus) {
		_mm_free(PMinus);
		PMinus = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated PMinus Address of System: %p\n",threadNum,this);
	#endif

	if (MScratch) {
		_mm_free(MScratch);
		MScratch = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated MScratch Address of System: %p\n",threadNum,this);
	#endif

	if (R) {
		_mm_free(R);
		R = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated R Address of System: %p\n",threadNum,this);
	#endif

	#ifdef DEBUG_DEALLOCATECARMA
	printf("deallocDLM - threadNum: %d; Finishing... Address of System: %p\n",threadNum,this);
	#endif
	}

int CARMA::get_p() {
	return p;
	}

int CARMA::get_q() {
	return q;
	}

double CARMA::get_dt() {
	return dt;
	}

void CARMA::set_dt(double t_incr) {
	dt = t_incr;
	}

int CARMA::get_allocated() {
	return allocated;
	}

void CARMA::getCARRoots(complex<double>*& CARRoots) {
	CARRoots = CARw;
	}

void CARMA::getCMARoots(complex<double>*& CMARoots) {
	CMARoots = CMAw;
	}

void CARMA::printX() {
	viewMatrix(p,1,X);
	}

const double* CARMA::getX() const {
	return X;
	}

void CARMA::printP() {
	viewMatrix(p,p,P);
	}

const double* CARMA::getP() const {
	return P;
	}

void CARMA::printA() {
	viewMatrix(p,p,A);
	}

const complex<double>* CARMA::getA() const {
	return A;
	}

void CARMA::printvr() {
	viewMatrix(p,p,vr);
	}

const complex<double>* CARMA::getvr() const {
	return vr;
	}

void CARMA::printvrInv() {
	viewMatrix(p,p,vrInv);
	}

const complex<double>* CARMA::getvrInv() const {
	return vrInv;
	}

void CARMA::printw() {
	viewMatrix(p,1,w);
	}

const complex<double>* CARMA::getw() const {
	return w;
	}

void CARMA::printexpw() {
	viewMatrix(p,p,expw);
	}

const complex<double>* CARMA::getexpw() const {
	return expw;
	}

void CARMA::printB() {
	viewMatrix(p,1,B);
	}

const complex<double>* CARMA::getB() const {
	return B;
	}

void CARMA::printC() {
	viewMatrix(p,p,C);
	}

const complex<double>* CARMA::getC() const {
	return C;
	}

void CARMA::printF() {
	viewMatrix(p,p,F);
	}

const double* CARMA::getF() const {
	return F;
	}

/*void CARMA::printD() {
	viewMatrix(p,1,D);
	}

const double* CARMA::getD() const {
	return D;
	}*/

void CARMA::printQ() {
	viewMatrix(p,p,Q);
	}

const double* CARMA::getQ() const {
	return Q;
	}

void CARMA::printT() {
	viewMatrix(p,p,T);
	}

const double* CARMA::getT() const {
	return T;
	}

void CARMA::printSigma() {
	viewMatrix(p,p,Sigma);
	}

const double* CARMA::getSigma() const {
	return Sigma;
	}

/*!
 * Checks the validity of the supplied C-ARMA parameters.
 * @param[in]  Theta  \f$\Theta\f$ contains \f$p\f$ CAR parameters followed by \f$q+1\f$ CMA parameters, i.e. \f$\Theta = [a_{1}, a_{2}, ..., a_{p-1}, a_{p}, b_{0}, b_{1}, ..., b_{q-1}, b_{q}]\f$, where we follow the notation in Brockwell 2001, Handbook of Statistics, Vol 19
 */
int CARMA::checkCARMAParams(double *ThetaIn /**< [in]  */) {
	/*!< \brief Function to check the validity of the CARMA parameters.


	*/

	#ifdef DEBUG_CHECKARMAPARAMS
	int threadNum = omp_get_thread_num();
	printf("checkCARMAParams - threadNum: %d; Address of System: %p\n",threadNum,this);
	#endif

	isStable = 1;
	isInvertible = 1;
	isNotRedundant = 1;
	hasUniqueEigenValues = 1;
	hasPosSigma = 1;

	if (ThetaIn[p] <= 0.0) {
		hasPosSigma = 0;
		}

	for (int rowCtr = 0; rowCtr < p; rowCtr++) {
		#pragma omp simd
		for (int colCtr = 0; colCtr < p; colCtr++) {
			CARMatrix[rowCtr + p*colCtr] = complexZero; // Reset matrix.
			}
		}

	CARMatrix[p*(p-1)] = -1.0*complexOne*ThetaIn[p-1]; // The first row has no 1s so we just set the rightmost entry equal to -alpha_p
	#pragma omp simd
	for (int rowCtr = 1; rowCtr < p; rowCtr++) {
		CARMatrix[rowCtr+(p-1)*p] = -1.0*complexOne*ThetaIn[p - 1 - rowCtr]; // Rightmost column of CARMatrix equals -alpha_k where 1 < k < p.
		CARMatrix[rowCtr+(rowCtr-1)*p] = complexOne; // CARMatrix has Identity matrix in bottom left.
		}
	ilo[0] = 0;
	ihi[0] = 0;
	abnrm[0] = 0.0;
	#pragma omp simd
	for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
		CARw[rowCtr] = complexZero;
		scale[rowCtr] = 0.0;
		rconde[rowCtr] = 0.0;
		rcondv[rowCtr] = 0.0;
		}
	#ifdef DEBUG_CHECKARMAPARAMS
	printf("checkCARMAParams - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%f ",Theta[dimNum]);
		}
	printf("\n");
	printf("checkCARMAParams - threadNum: %d; CARMatrix\n",threadNum);
	viewMatrix(p,p,CARMatrix);
	#endif

	mkl_domain_set_num_threads(1, MKL_DOMAIN_ALL);
	lapack_int YesNo;
	//YesNo = LAPACKE_zgeevx(LAPACK_COL_MAJOR, 'B', 'N', 'N', 'N', p, CARMatrix, p, CARw, vrInv, p, vr, p, ilo, ihi, scale, abnrm, rconde, rcondv); // NOT WORKING!!!
	YesNo = LAPACKE_zgeev(LAPACK_COL_MAJOR, 'N', 'N', p, CARMatrix, p, CARw, vrInv, p, vr, p);
	//YesNo = LAPACKE_zgeev(LAPACK_COL_MAJOR, 'N', 'N', p, CARMatrix, p, CARw, vrInv, p, vr, p);

	for (int i = 0; i < p; i++) {

		#ifdef DEBUG_CHECKARMAPARAMS
		printf("checkCARMAParams - threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
			printf("%f ",Theta[dimNum]);
			}
		printf("\n");
		printf("checkCARMAParams - threadNum: %d; Root: %+f%+fi; Len: %f\n",threadNum, CARw[i].real(), CARw[i].imag(), abs(CARw[i]));
		#endif

		if (CARw[i].real() >= 0.0) {

			#ifdef DEBUG_CHECKARMAPARAMS
			printf("checkCARMAParams - threadNum: %d; walkerPos: ",threadNum);
			for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
				printf("%f ",Theta[dimNum]);
				}
			printf("\n");
			printf("checkCARMAParams - threadNum: %d; badRoot!!!: %+f%+fi; Len: %f\n",threadNum,CARw[i].real(), CARw[i].imag(),abs(CARw[i]));
			#endif

			isStable = 0;
			}

		for (int j = i+1; j < p; j++) { // Only need to check e-values against each other once.
			if (CARw[i] == CARw[j]) {
				hasUniqueEigenValues = 0;
				}
			}
		}

	if (q > 0) {
		for (int rowCtr = 0; rowCtr < q; ++rowCtr) {
			#pragma omp simd
			for (int colCtr = 0; colCtr < q; colCtr++) {
				CMAMatrix[rowCtr + q*colCtr] = complexZero; // Initialize matrix.
				}
			}
		CMAMatrix[(q-1)*q] = -1.0*complexOne*ThetaIn[p]/ThetaIn[p + q]; // MAMatrix has -beta_q/-beta_0 at top right!
		#pragma omp simd
		for (int rowCtr = 1; rowCtr < q; ++rowCtr) {
			CMAMatrix[rowCtr + (q - 1)*q] = -1.0*complexOne*ThetaIn[p + rowCtr]/ThetaIn[p + q]; // Rightmost column of MAMatrix has -MA coeffs.
			CMAMatrix[rowCtr + (rowCtr - 1)*q] = complexOne; // MAMatrix has Identity matrix in bottom left.
			}
		ilo[0] = 0;
		ihi[0] = 0;
		abnrm[0] = 0.0;
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < q; ++rowCtr) {
			CMAw[rowCtr] = complexZero;
			}
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			scale[rowCtr] = 0.0;
			rconde[rowCtr] = 0.0;
			rcondv[rowCtr] = 0.0;
			}
		#ifdef DEBUG_CHECKARMAPARAMS
		printf("checkCARMAParams - threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
			printf("%f ",Theta[dimNum]);
			}
		printf("\n");
		printf("checkCARMAParams - threadNum: %d; CMAMatrix\n",threadNum);
		viewMatrix(q,q,CMAMatrix);
		#endif

		//YesNo = LAPACKE_zgeevx(LAPACK_COL_MAJOR, 'B', 'N', 'V', 'N', q, CMAMatrix, q, CMAw, nullptr, 1, vr, q, ilo, ihi, scale, abnrm, rconde, rcondv); // NOT WORKING!!!!
		YesNo = LAPACKE_zgeev(LAPACK_COL_MAJOR, 'N', 'N', q, CMAMatrix, q, CMAw, vrInv, q, vr, q);
		//YesNo = LAPACKE_zgeev(LAPACK_COL_MAJOR, 'N', 'N', q, CMAMatrix, q, CMAw, vrInv, p, vr, p);

		for (int i = 0; i < q; i++) {

			#ifdef DEBUG_CHECKARMAPARAMS
			printf("checkCARMAParams - threadNum: %d; walkerPos: ",threadNum);
			for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
				printf("%f ",Theta[dimNum]);
				}
			printf("\n");
			printf("checkCARMAParams - threadNum: %d; Root: %+f%+fi; Len: %f\n",threadNum, CMAw[i].real(), CMAw[i].imag(), abs(CMAw[i]));
			#endif

			if (CMAw[i].real() > 0.0) {

				#ifdef DEBUG_CHECKARMAPARAMS
				printf("checkCARMAParams - threadNum: %d; walkerPos: ",threadNum);
				for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
					printf("%f ",Theta[dimNum]);
					}
				printf("\n");
				printf("checkCARMAParams - threadNum: %d; badRoot!!!: %+f%+fi; Len: %f\n",threadNum,CMAw[i].real(), CMAw[i].imag(),abs(CMAw[i]));
				#endif

				isInvertible = 0;
				}
			}

		for (int i = 1; i < p; i++) {
			for (int j = 1; j < q; j++) {
				if (CARw[i] == CMAw[j]) {
					isNotRedundant = 0;
					}
				}
			}
		} else if (q == 0) {
		if (Theta[p] < 0) {
			isInvertible = 0;
			}
		} else {
		printf("FATAL LOGIC ERROR: numP MUST BE > numQ >= 0!\n");
		exit(1);
		}

	#ifdef DEBUG_CHECKARMAPARAMS
	printf("checkCARMAParams - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%f ",Theta[dimNum]);
		}
	printf("\n");
	printf("checkCARMAParams - threadNum: %d; isStable: %d\n",threadNum,isStable);
	printf("checkCARMAParams - threadNum: %d; isInvertible: %d\n",threadNum,isInvertible);
	printf("checkCARMAParams - threadNum: %d; isNotRedundant: %d\n",threadNum,isNotRedundant);
	printf("checkCARMAParams - threadNum: %d; hasUniqueEigenValues: %d\n",threadNum,hasUniqueEigenValues);
	printf("checkCARMAParams - threadNum: %d; hasPosSigma: %d\n",threadNum,hasPosSigma);
	printf("\n");
	printf("\n");
	#endif

	return isStable*isInvertible*isNotRedundant*hasUniqueEigenValues*hasPosSigma;
	}

void CARMA::setCARMA(double *ThetaIn) {

	complex<double> alpha = complexOne, beta = complexZero;

	#if (defined DEBUG_SETCARMA) || (defined DEBUG_SETCARMA_C)
	int threadNum = omp_get_thread_num();
	#endif

	#ifdef DEBUG_SETCARMA
	printf("setCARMA - threadNum: %d; Address of System: %p\n",threadNum,this);
	printf("\n");
	#endif

	#pragma omp simd
	for (int rowCtr = 0; rowCtr < p + q + 1; ++rowCtr) {
		Theta[rowCtr] = ThetaIn[rowCtr];
		}

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			A[rowCtr + colCtr*p].real(0.0);
			A[rowCtr + colCtr*p].imag(0.0);
			}
		}

	A[0].real(-1.0*Theta[0]);
	#pragma omp simd
	for (int i = 1; i < p; ++i) {
		A[i].real(-1.0*Theta[i]);
		A[i*p + (i - 1)].real(1.0);
		}

	cblas_zcopy(pSq, A, 1, ACopy, 1); // Copy A into ACopy so that we can keep a clean working version of it.

	#ifdef DEBUG_SETCARMA
	printf("setCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setCARMA - threadNum: %d; A\n",threadNum);
	viewMatrix(p,p,A);
	printf("\n");
	printf("setCARMA - threadNum: %d; ACopy\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	#endif

	ilo[0] = 0;
	ihi[0] = 0;
	abnrm[0] = 0.0;
	for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
		w[rowCtr] = complexZero;
		scale[rowCtr] = 0.0;
		rconde[rowCtr] = 0.0;
		rcondv[rowCtr] = 0.0;
		#pragma omp simd
		for (int colCtr = 0; colCtr < p; ++colCtr) {
			vr[rowCtr + colCtr*p] = complexZero;
			vrInv[rowCtr + colCtr*p] = complexZero;
			}
		}

	lapack_int YesNo;
	YesNo = LAPACKE_zgeevx(LAPACK_COL_MAJOR, 'B', 'N', 'V', 'N', p, ACopy, p, w, vrInv, 1, vr, p, ilo, ihi, scale, abnrm, rconde, rcondv); // Compute w and vr
	//YesNo = LAPACKE_zgeev(LAPACK_COL_MAJOR, 'N', 'V', p, ACopy, p, w, vrInv, p, vr, p);

	YesNo = LAPACKE_zlacpy(LAPACK_COL_MAJOR, 'B', p, p, vr, p, vrInv, p); // Copy vr into vrInv

	YesNo = LAPACKE_zgetrf(LAPACK_COL_MAJOR, p, p, vrInv, p, ipiv); // Compute LU factorization of vrInv == vr

	YesNo = LAPACKE_zgetri(LAPACK_COL_MAJOR, p, vrInv, p, ipiv); // Compute vrInv = inverse of vr from LU decomposition

	#ifdef DEBUG_SETCARMA
	printf("setCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setCARMA - threadNum: %d; w\n",threadNum);
	viewMatrix(p,1,w);
	printf("\n");
	printf("setCARMA - threadNum: %d; vr\n",threadNum);
	viewMatrix(p,p,vr);
	printf("\n");
	printf("setCARMA - threadNum: %d; vrInv\n",threadNum);
	viewMatrix(p,p,vrInv);
	printf("\n");
	#endif

	//#pragma omp simd
	for (int rowCtr = 0; rowCtr < q + 1; rowCtr++) {
		B[p - 1 - rowCtr] = complexOne*Theta[p + rowCtr];
		}

	#ifdef DEBUG_SETCARMA
	printf("setCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setCARMA - threadNum: %d; B\n",threadNum);
	viewMatrix(p,1,B);
	printf("\n");
	#endif

	// Start computation of C

	#ifdef DEBUG_SETCARMA_C
	printf("setCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setCARMA - threadNum: %d; vrInv (Before)\n",threadNum);
	viewMatrix(p,p,vrInv);
	printf("\n");
	printf("setCARMA - threadNum: %d; B (Before)\n",threadNum);
	viewMatrix(p,1,B);
	printf("\n");
	printf("setCARMA - threadNum: %d; BScratch = vrInv*B (Before)\n",threadNum);
	viewMatrix(p,1,BScratch);
	printf("\n");
	#endif

	cblas_zgemv(CblasColMajor, CblasNoTrans, p, p, &alpha, vrInv, p, B, 1, &beta, BScratch, 1); // BScratch = vrInv*B

	#ifdef DEBUG_SETCARMA_C
	printf("setCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setCARMA - threadNum: %d; vrInv (After)\n",threadNum);
	viewMatrix(p,p,vrInv);
	printf("\n");
	printf("setCARMA - threadNum: %d; B (After)\n",threadNum);
	viewMatrix(p,1,B);
	printf("\n");
	printf("setCARMA - threadNum: %d; BScratch = vrInv*B (After)\n",threadNum);
	viewMatrix(p,1,BScratch);
	printf("\n");
	#endif

	#ifdef DEBUG_SETCARMA_C
	printf("setCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setCARMA - threadNum: %d; B (Before)\n",threadNum);
	viewMatrix(p,1,B);
	printf("\n");
	printf("setCARMA - threadNum: %d; BScratch (Before)\n",threadNum);
	viewMatrix(p,1,BScratch);
	printf("\n");
	printf("setCARMA - threadNum: %d; ACopy = BScratch*B (Before)\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	#endif

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			ACopy[rowCtr + colCtr*p] = BScratch[rowCtr]*B[colCtr]; // ACopy = BScratch*trans(B) = vrInv*b*trans(B)
			}
		}

	#ifdef DEBUG_SETCARMA_C
	printf("setCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setCARMA - threadNum: %d; B (After)\n",threadNum);
	viewMatrix(p,1,B);
	printf("\n");
	printf("setCARMA - threadNum: %d; BScratch (After)\n",threadNum);
	viewMatrix(p,1,BScratch);
	printf("\n");
	printf("setCARMA - threadNum: %d; ACopy = BScratch*B (After)\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	#endif


	#ifdef DEBUG_SETCARMA_C
	printf("setCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setCARMA - threadNum: %d; ACopy (Before)\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	printf("setCARMA - threadNum: %d; vrInv (Before)\n",threadNum);
	viewMatrix(p,p,vrInv);
	printf("\n");
	printf("setCARMA - threadNum: %d; AScratch = ACopy*trans(vrInv) (Before)\n",threadNum);
	viewMatrix(p,p,AScratch);
	printf("\n");
	#endif

	cblas_zgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, &alpha, ACopy, p, vrInv, p, &beta, AScratch, p); // AScratch = ACopy*trans(vrInv) = vrInv*b*trans(B)*trans(vrInv)

	#ifdef DEBUG_SETCARMA_C
	printf("setCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setCARMA - threadNum: %d; ACopy (After)\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	printf("setCARMA - threadNum: %d; vrInv (After)\n",threadNum);
	viewMatrix(p,p,vrInv);
	printf("\n");
	printf("setCARMA - threadNum: %d; AScratch = ACopy*trans(vrInv) (After)\n",threadNum);
	viewMatrix(p,p,AScratch);
	printf("\n");
	#endif

	#ifdef DEBUG_SETCARMA_C
	printf("setCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setCARMA - threadNum: %d; AScratch (Before)\n",threadNum);
	viewMatrix(p,p,AScratch);
	printf("\n");
	printf("setCARMA - threadNum: %d; C = AScratch (Before)\n",threadNum);
	viewMatrix(p,p,C);
	printf("\n");
	#endif

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			C[rowCtr + colCtr*p] = (AScratch[rowCtr + colCtr*p] + AScratch[colCtr + rowCtr*p])/2.0; // C = (AScratch + trans(AScratch))/2 to ensure symmetry!
			}
		}

	#ifdef DEBUG_SETCARMA_C
	printf("setCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setCARMA - threadNum: %d; AScratch (After)\n",threadNum);
	viewMatrix(p,p,AScratch);
	printf("\n");
	printf("setCARMA - threadNum: %d; C = AScratch (After)\n",threadNum);
	viewMatrix(p,p,C);
	printf("\n");
	#endif

	H[0] = 1.0;
	}

void CARMA::solveCARMA() {
	#if (defined DEBUG_SOLVECARMA_F) || (defined DEBUG_SOLVECARMA_Q)
	int threadNum = omp_get_thread_num();
	#endif

	#ifdef DEBUG_SOLVECARMA_F
	printf("solveCARMA - threadNum: %d; Address of System: %p\n",threadNum,this);
	printf("\n");
	#endif

	#ifdef DEBUG_SOLVECARMA_F
	printf("solveCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+8.7e ",Theta[dimNum]);
		}
	printf("\n");
	printf("solveCARMA - threadNum: %d; w (Before)\n",threadNum);
	viewMatrix(p,1,w);
	printf("\n");
	printf("solveCARMA - threadNum: %d; expw = exp(w) (Before)\n",threadNum);
	viewMatrix(p,p,expw);
	printf("\n");
	#endif

	#pragma omp simd
	for (int i = 0; i < p; ++i) {
		expw[i + i*p] = exp(dt*w[i]);
		}

	#ifdef DEBUG_SOLVECARMA_F
	printf("solveCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+8.7e ",Theta[dimNum]);
		}
	printf("\n");
	printf("solveCARMA - threadNum: %d; w (After)\n",threadNum);
	viewMatrix(p,1,w);
	printf("\n");
	printf("solveCARMA - threadNum: %d; expw = exp(w) (After)\n",threadNum);
	viewMatrix(p,p,expw);
	printf("\n");
	#endif

	complex<double> alpha = complexOne, beta = complexZero;

	#ifdef DEBUG_SOLVECARMA_F
	printf("solveCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+8.7e ",Theta[dimNum]);
		}
	printf("\n");
	printf("solveCARMA - threadNum: %d; vr (Before)\n",threadNum);
	viewMatrix(p,p,vr);
	printf("\n");
	printf("solveCARMA - threadNum: %d; expw (Before)\n",threadNum);
	viewMatrix(p,p,expw);
	printf("\n");
	printf("solveCARMA - threadNum: %d; ACopy = vr*expw (Before)\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	#endif

	cblas_zgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, &alpha, vr, p, expw, p, &beta, ACopy, p);

	#ifdef DEBUG_SOLVECARMA_F
	printf("solveCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+8.7e ",Theta[dimNum]);
		}
	printf("\n");
	printf("solveCARMA - threadNum: %d; vr (After)\n",threadNum);
	viewMatrix(p,p,vr);
	printf("\n");
	printf("solveCARMA - threadNum: %d; expw (After)\n",threadNum);
	viewMatrix(p,p,expw);
	printf("\n");
	printf("solveCARMA - threadNum: %d; ACopy = vr*expw (After)\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	#endif

	#ifdef DEBUG_SOLVECARMA_F
	printf("solveCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+8.7e ",Theta[dimNum]);
		}
	printf("\n");
	printf("solveCARMA - threadNum: %d; ACopy (Before)\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	printf("solveCARMA - threadNum: %d; vrInv (Before)\n",threadNum);
	viewMatrix(p,p,vrInv);
	printf("\n");
	printf("solveCARMA - threadNum: %d; AScratch = ACopy*vrInv (Before)\n",threadNum);
	viewMatrix(p,p,AScratch);
	printf("\n");
	#endif

	cblas_zgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, &alpha, ACopy, p, vrInv, p, &beta, AScratch, p);

	#ifdef DEBUG_SOLVECARMA_F
	printf("solveCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+8.7e ",Theta[dimNum]);
		}
	printf("\n");
	printf("solveCARMA - threadNum: %d; ACopy (After)\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	printf("solveCARMA - threadNum: %d; vrInv (After)\n",threadNum);
	viewMatrix(p,p,vrInv);
	printf("\n");
	printf("solveCARMA - threadNum: %d; AScratch = aCopy*vrInv (After)\n",threadNum);
	viewMatrix(p,p,AScratch);
	printf("\n");
	#endif

	#ifdef DEBUG_SOLVECARMA_F
	printf("solveCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+8.7e ",Theta[dimNum]);
		}
	printf("\n");
	printf("solveDLM - threadNum: %d; AScratch (Before)\n",threadNum);
	viewMatrix(p,p,AScratch);
	printf("\n");
	printf("solveDLM - threadNum: %d; F = AScratch.real() (Before)\n",threadNum);
	viewMatrix(p,p,F);
	printf("\n");
	#endif

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			F[rowCtr + colCtr*p] = AScratch[rowCtr + colCtr*p].real();
			}
		}

	#ifdef DEBUG_SOLVECARMA_F
	printf("solveCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < p+q+1; dimNum++) {
		printf("%+8.7e ",Theta[dimNum]);
		}
	printf("\n");
	printf("solveDLM - threadNum: %d; AScratch (After)\n",threadNum);
	viewMatrix(p,p,AScratch);
	printf("\n");
	printf("solveDLM - threadNum: %d; F = AScratch.real() (After)\n",threadNum);
	viewMatrix(p,p,F);
	printf("\n");
	#endif

	/*complex<double> rootSum, rootSumInv, rootSumDT, expRootSumDTM1;
	for (int j = 0; j < p; ++j) {
		BScratch[j] = 0.0+0.0i;
		for (int l = 0; l < p; ++l) {
			#pragma omp simd
			for (int k = 0; k < p; ++k) {
				rootSum = w[k] + w[l];
				rootSumInv = (1.0+0.0i)/rootSum;
				rootSumDT = rootSum*dt;
				expRootSumDTM1 = exp(rootSumDT) - 1.0;
				AScratch[k + l*p] = C[k + l*p]*(exp((dt+0.0i)*rootSum) - (1.0+0.0i))*rootSumInv;
				BScratch[j] += vr[j + k*p]*AScratch[k + l*p]*vr[j + l*p];
				}
			}
		D[j] = sqrt(BScratch[j].real());
		}

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			Q[rowCtr + colCtr*p] = D[rowCtr]*D[colCtr];
			}
		}*/

	for (int colNum = 0; colNum < p; ++colNum) {
		#pragma omp simd
		for (int rowNum = 0; rowNum < p; ++rowNum) {
			AScratch[rowNum + p*colNum] = C[rowNum + p*colNum]*(exp((complexOne*dt)*(w[rowNum] + w[colNum])) - complexOne)*(complexOne/(w[rowNum] + w[colNum])); // AScratch[i,j] = (C[i,j]*(exp((lambda[i] + lambda[j])*dt) - 1))/(lambda[i] + lambda[j])
			ACopy[rowNum + p*colNum] = C[rowNum + p*colNum]*( - complexOne)*(complexOne/(w[rowNum] + w[colNum])); // ACopy[i,j] = = (C[i,j]*( - 1/(lambda[i] + lambda[j]))
		}
	}

	cblas_zgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, &alpha, vr, p, AScratch, p, &beta, AScratch2, p); // AScratch2 = vr*AScratch
	cblas_zgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, &alpha, AScratch2, p, vr, p, &beta, AScratch, p); // AScratch = AScratch2*trans(vr)

	cblas_zgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, &alpha, vr, p, ACopy, p, &beta, AScratch2, p); // AScratch2 = vr*ACopy
	cblas_zgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, &alpha, AScratch2, p, vr, p, &beta, ACopy, p); // Sigma = AScratch2*trans(vr)

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			Q[rowCtr + colCtr*p] = AScratch[rowCtr + colCtr*p].real();
			Sigma[rowCtr + colCtr*p] = ACopy[rowCtr + colCtr*p].real();
			}
		}

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			T[rowCtr + colCtr*p] = Q[rowCtr + colCtr*p];
			}
		}

	char uplo = 'U';
	int n = p, lda = p;
	lapack_int YesNo;
	//YesNo = LAPACKE_dpotrf(LAPACK_COL_MAJOR, 'U', p, T, p);
	dpotrf(&uplo, &n, T, &lda, &YesNo);

	}

/*void CARMA::computeSigma() {

	complex<double> rootSum, rootSumInv, rootSumDT, expRootSumDTM1;
	for (int j = 0; j < p; ++j) {
		BScratch[j] = 0.0+0.0i;
		for (int l = 0; l < p; ++l) {
			#pragma omp simd
			for (int k = 0; k < p; ++k) {
				rootSum = w[k] + w[l];
				rootSumInv = (1.0+0.0i)/rootSum;
				rootSumDT = rootSum*dt;
				expRootSumDTM1 = -1.0;
				AScratch[k + l*p] = C[k + l*p]*expRootSumDTM1*rootSumInv;
				BScratch[j] += vr[j + k*p]*AScratch[k + l*p]*vr[j + l*p];
				}
			}
		VScratch[j] = sqrt(BScratch[j].real());
		}

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			Sigma[rowCtr + colCtr*p] = VScratch[rowCtr]*VScratch[colCtr];
			}
		}

	}*/

void CARMA::resetState(double InitUncertainty) {

	#ifdef DEBUG_RESETSTATE
	int threadNum = omp_get_thread_num();
	printf("resetState - threadNum: %d; Address of System: %p\n",threadNum,this);
	printf("\n");
	#endif

	for (int i = 0; i < p; i++) {
		X[i] = 0.0;
		XMinus[i] = 0.0;
		VScratch[i] = 0.0;
		#pragma omp simd
		for (int j = 0; j < p; j++) {
			P[i*p+j] = 0.0;
			PMinus[i*p+j] = 0.0;
			MScratch[i*p+j] = 0.0;
			}
		P[i*p+i] = InitUncertainty;
		}
	}

void CARMA::resetState() {

	//computeSigma();

	// Copy P and reset the other matrices
	for (int colCtr = 0; colCtr < p; ++colCtr) {
		X[colCtr] = 0.0;
		XMinus[colCtr] = 0.0;
		VScratch[colCtr] = 0.0;
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			PMinus[rowCtr + colCtr*p] = 0.0;
			MScratch[rowCtr + colCtr*p] = 0.0;
			P[rowCtr + colCtr*p] = Sigma[rowCtr + colCtr*p];
			}
		}

	#ifdef DEBUG_RESETSTATE
	printf("resetState - threadNum: %d; P\n",threadNum);
	viewMatrix(p,p,P);
	printf("\n");
	#endif

	}

void CARMA::burnSystem(int numBurn, unsigned int burnSeed, double* burnRand) {

	mkl_domain_set_num_threads(1, MKL_DOMAIN_ALL);
	VSLStreamStatePtr burnStream __attribute__((aligned(64)));
	vslNewStream(&burnStream, VSL_BRNG_SFMT19937, burnSeed);
	//vdRngGaussian(VSL_RNG_METHOD_GAUSSIAN_ICDF, burnStream, numBurn, burnRand, 0.0, 1.0); // Check
	#pragma omp simd
	for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
		VScratch[rowCtr] = 0.0;
		}
	vdRngGaussianMV(VSL_RNG_METHOD_GAUSSIANMV_ICDF, burnStream, numBurn, burnRand, p, VSL_MATRIX_STORAGE_FULL, VScratch, T);
	vslDeleteStream(&burnStream);

	for (int i = 0; i < numBurn; ++i) {
		cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, VScratch, 1); // VScratch = F*X
		cblas_dcopy(p, VScratch, 1, X, 1); // X = VScratch
		cblas_daxpy(p, 1.0, &burnRand[i*p], 1, X, 1); // X = w*D + X
		}
	}

void CARMA::observeSystem(LnLikeData *ptr2Data, unsigned int distSeed, double *distRand) {
	LnLikeData Data = *ptr2Data;

	int numCadences = Data.numCadences;
	bool IR = Data.IR;
	double tolIR = Data.tolIR;
	double t_incr = Data.t_incr;
	double *t = Data.t;
	double *y = Data.y;
	double *mask = Data.mask;

	mkl_domain_set_num_threads(1, MKL_DOMAIN_ALL);
	VSLStreamStatePtr distStream __attribute__((aligned(64)));
	vslNewStream(&distStream, VSL_BRNG_SFMT19937, distSeed);

	if (IR == false) {

		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			VScratch[rowCtr] = 0.0;
			}
		vdRngGaussianMV(VSL_RNG_METHOD_GAUSSIANMV_ICDF, distStream, numCadences, &distRand[0], p, VSL_MATRIX_STORAGE_FULL, VScratch, T);
		for (int i = 0; i < numCadences; ++i) {

			cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, VScratch, 1);
			cblas_dcopy(p, VScratch, 1, X, 1);
			cblas_daxpy(p, 1.0, &distRand[i*p], 1, X, 1);

			y[i] = mask[i]*X[0];
			}

		} else {

		double fracChange = 0.0;

		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			VScratch[rowCtr] = 0.0;
			}
		vdRngGaussianMV(VSL_RNG_METHOD_GAUSSIANMV_ICDF, distStream, 1, &distRand[0], p, VSL_MATRIX_STORAGE_FULL, VScratch, T);

		cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, VScratch, 1); // VScratch = F*x
		cblas_dcopy(p, VScratch, 1, X, 1); // X = VScratch
		cblas_daxpy(p, 1.0, &distRand[0], 1, X, 1);
		y[0] = X[0];

		for (int i = 1; i < numCadences; ++i) {

			t_incr = t[i] - t[i - 1];
			fracChange = abs((t_incr - dt)/((t_incr + dt)/2.0));

			if (fracChange > tolIR) {
				dt = t_incr;
				solveCARMA();
				}

			#pragma omp simd
			for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
				VScratch[rowCtr] = 0.0;
				}
			vdRngGaussianMV(VSL_RNG_METHOD_GAUSSIANMV_ICDF, distStream, 1, &distRand[i*p], p, VSL_MATRIX_STORAGE_FULL, VScratch, T);

			cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, VScratch, 1);
			cblas_dcopy(p, VScratch, 1, X, 1);
			cblas_daxpy(p, 1.0, &distRand[i*p], 1, X, 1);
			y[i] = X[0];
			}

		}

	vslDeleteStream(&distStream);
	}

void CARMA::addNoise(LnLikeData *ptr2Data, unsigned int noiseSeed, double* noiseRand) {
	LnLikeData Data = *ptr2Data;

	int numCadences = Data.numCadences;
	bool IR = Data.IR;
	double t_incr = Data.t_incr;
	double fracIntrinsicVar = Data.fracIntrinsicVar;
	double fracSignalToNoise = Data.fracSignalToNoise;
	double *t = Data.t;
	double *y = Data.y;
	double *yerr = Data.yerr;
	double *mask = Data.mask;

	mkl_domain_set_num_threads(1, MKL_DOMAIN_ALL);
	VSLStreamStatePtr noiseStream __attribute__((aligned(64)));
	vslNewStream(&noiseStream, VSL_BRNG_SFMT19937, noiseSeed);

	double absIntrinsicVar = sqrt(Sigma[0]);
	double absMeanFlux = absIntrinsicVar/fracIntrinsicVar;
	double absFlux = 0.0, noiseLvl = 0.0;
	for (int i = 0; i < numCadences; ++i) {
		absFlux = absMeanFlux + y[i];
		noiseLvl = fracSignalToNoise*absFlux;
		vdRngGaussian(VSL_RNG_METHOD_GAUSSIAN_ICDF, noiseStream, 1, &noiseRand[i], 0.0, noiseLvl);
		y[i] += noiseRand[i];
		yerr[i] = noiseLvl;
		}
	vslDeleteStream(&noiseStream);
	}

double CARMA::computeLnLike(LnLikeData *ptr2Data) {
	LnLikeData Data = *ptr2Data;

	int numCadences = Data.numCadences;
	bool IR = Data.IR;
	double tolIR = Data.tolIR; 
	double t_incr = Data.t_incr;
	double *t = Data.t;
	double *y = Data.y;
	double *yerr = Data.yerr;
	double *mask = Data.mask;
	double maxDouble = numeric_limits<double>::max();

	mkl_domain_set_num_threads(1, MKL_DOMAIN_ALL);
	double LnLike = 0.0, ptCounter = 0.0, v = 0.0, S = 0.0, SInv = 0.0, fracChange = 0.0, Contrib = 0.0;

	if (IR == false) {
		for (int i = 0; i < numCadences; i++) {
			R[0] = yerr[i]*yerr[i]; // Heteroskedastic errors
			H[0] = mask[i]; // Missing data
			cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, XMinus, 1); // Compute XMinus = F*X
			cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, F, p, P, p, 0.0, MScratch, p); // Compute MScratch = F*P
			cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, MScratch, p, F, p, 0.0, PMinus, p); // Compute PMinus = MScratch*F_Transpose
			cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, I, p, Q, p, 1.0, PMinus, p); // Compute PMinus = I*Q + PMinus;
			v = y[i] - XMinus[0]; // Compute v = y - H*X
			cblas_dgemv(CblasColMajor, CblasTrans, p, p, 1.0, PMinus, p, H, 1, 0.0, K, 1); // Compute K = PMinus*H_Transpose
			S = cblas_ddot(p, K, 1, H, 1) + R[0]; // Compute S = H*K + R
			SInv = 1.0/S;
			cblas_dscal(p, SInv, K, 1); // Compute K = SInv*K
			for (int colCounter = 0; colCounter < p; colCounter++) {
				#pragma omp simd
				for (int rowCounter = 0; rowCounter < p; rowCounter++) {
					MScratch[rowCounter*p+colCounter] = I[colCounter*p+rowCounter] - K[colCounter]*H[rowCounter]; // Compute MScratch = I - K*H
					}
				}
			cblas_dcopy(p, K, 1, VScratch, 1); // Compute VScratch = K
			cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, MScratch, p, XMinus, 1, y[i], VScratch, 1); // Compute X = VScratch*y[i] + MScratch*XMinus
			cblas_dcopy(p, VScratch, 1, X, 1); // Compute X = VScratch
			cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, MScratch, p, PMinus, p, 0.0, P, p); // Compute P = IMinusKH*PMinus
			cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, P, p, MScratch, p, 0.0, PMinus, p); // Compute PMinus = P*IMinusKH_Transpose
			for (int colCounter = 0; colCounter < p; colCounter++) {
				#pragma omp simd
				for (int rowCounter = 0; rowCounter < p; rowCounter++) {
					P[colCounter*p+rowCounter] = PMinus[colCounter*p+rowCounter] + R[0]*K[colCounter]*K[rowCounter]; // Compute P = PMinus + K*R*K_Transpose
					}
				}
			Contrib = mask[i]*(-0.5*SInv*pow(v,2.0) -0.5*log2(S)/log2OfE);
			LnLike = LnLike + Contrib; // LnLike += -0.5*v*v*SInv -0.5*log(det(S)) -0.5*log(2.0*pi)
			ptCounter = ptCounter + mask[i];
			}
		//LnLike += -0.5*ptCounter*log2Pi;
		} else {
		H[0] = 1.0;
		R[0] = yerr[0]*yerr[0]; // Heteroskedastic errors
		cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, XMinus, 1); // Compute XMinus = F*X
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, F, p, P, p, 0.0, MScratch, p); // Compute MScratch = F*P
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, MScratch, p, F, p, 0.0, PMinus, p); // Compute PMinus = MScratch*F_Transpose
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, I, p, Q, p, 1.0, PMinus, p); // Compute PMinus = I*Q + PMinus;
		v = y[0] - XMinus[0]; // Compute v = y - H*X
		cblas_dgemv(CblasColMajor, CblasTrans, p, p, 1.0, PMinus, p, H, 1, 0.0, K, 1); // Compute K = PMinus*H_Transpose
		S = cblas_ddot(p, K, 1, H, 1) + R[0]; // Compute S = H*K + R
		SInv = 1.0/S;
		cblas_dscal(p, SInv, K, 1); // Compute K = SInv*K
		for (int colCounter = 0; colCounter < p; colCounter++) {
			#pragma omp simd
			for (int rowCounter = 0; rowCounter < p; rowCounter++) {
				MScratch[rowCounter*p+colCounter] = I[colCounter*p+rowCounter] - K[colCounter]*H[rowCounter]; // Compute MScratch = I - K*H
				}
			}
		cblas_dcopy(p, K, 1, VScratch, 1); // Compute VScratch = K
		cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, MScratch, p, XMinus, 1, y[0], VScratch, 1); // Compute X = VScratch*y[i] + MScratch*XMinus
		cblas_dcopy(p, VScratch, 1, X, 1); // Compute X = VScratch
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, MScratch, p, PMinus, p, 0.0, P, p); // Compute P = IMinusKH*PMinus
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, P, p, MScratch, p, 0.0, PMinus, p); // Compute PMinus = P*IMinusKH_Transpose
		for (int colCounter = 0; colCounter < p; colCounter++) {
			#pragma omp simd
			for (int rowCounter = 0; rowCounter < p; rowCounter++) {
				P[colCounter*p+rowCounter] = PMinus[colCounter*p+rowCounter] + R[0]*K[colCounter]*K[rowCounter]; // Compute P = PMinus + K*R*K_Transpose
				}
			}
		Contrib = (-0.5*SInv*pow(v,2.0) -0.5*log2(S)/log2OfE);
		LnLike = LnLike + Contrib; // LnLike += -0.5*v*v*SInv -0.5*log(det(S)) -0.5*log(2.0*pi)
		ptCounter = ptCounter + 1;
		for (int i = 1; i < numCadences; i++) {
			t_incr = t[i] - t[i - 1];
			fracChange = abs((t_incr - dt)/((t_incr + dt)/2.0));
			if (fracChange > tolIR) {
				dt = t_incr;
				solveCARMA();
				}
			R[0] = yerr[i]*yerr[i]; // Heteroskedastic errors
			cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, XMinus, 1); // Compute XMinus = F*X
			cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, F, p, P, p, 0.0, MScratch, p); // Compute MScratch = F*P
			cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, MScratch, p, F, p, 0.0, PMinus, p); // Compute PMinus = MScratch*F_Transpose
			cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, I, p, Q, p, 1.0, PMinus, p); // Compute PMinus = I*Q + PMinus;
			v = y[i] - XMinus[0]; // Compute v = y - H*X
			cblas_dgemv(CblasColMajor, CblasTrans, p, p, 1.0, PMinus, p, H, 1, 0.0, K, 1); // Compute K = PMinus*H_Transpose
			S = cblas_ddot(p, K, 1, H, 1) + R[0]; // Compute S = H*K + R
			SInv = 1.0/S;
			cblas_dscal(p, SInv, K, 1); // Compute K = SInv*K
			for (int colCounter = 0; colCounter < p; colCounter++) {
				#pragma omp simd
				for (int rowCounter = 0; rowCounter < p; rowCounter++) {
					MScratch[rowCounter*p+colCounter] = I[colCounter*p+rowCounter] - K[colCounter]*H[rowCounter]; // Compute MScratch = I - K*H
					}
				}
			cblas_dcopy(p, K, 1, VScratch, 1); // Compute VScratch = K
			cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, MScratch, p, XMinus, 1, y[i], VScratch, 1); // Compute X = VScratch*y[i] + MScratch*XMinus
			cblas_dcopy(p, VScratch, 1, X, 1); // Compute X = VScratch
			cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, MScratch, p, PMinus, p, 0.0, P, p); // Compute P = IMinusKH*PMinus
			cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, P, p, MScratch, p, 0.0, PMinus, p); // Compute PMinus = P*IMinusKH_Transpose
			for (int colCounter = 0; colCounter < p; colCounter++) {
				#pragma omp simd
				for (int rowCounter = 0; rowCounter < p; rowCounter++) {
					P[colCounter*p+rowCounter] = PMinus[colCounter*p+rowCounter] + R[0]*K[colCounter]*K[rowCounter]; // Compute P = PMinus + K*R*K_Transpose
					}
				}
			Contrib = (-0.5*SInv*pow(v,2.0) -0.5*log2(S)/log2OfE);
			LnLike = LnLike + Contrib; // LnLike += -0.5*v*v*SInv -0.5*log(det(S)) -0.5*log(2.0*pi)
			ptCounter = ptCounter + 1;
			}
		//LnLike += -0.5*ptCounter*log2Pi;
		}

	return LnLike;
	}
