#!/usr/bin/env python
"""	Module to perform basic C-ARMA modelling.

	For a demonstration of the module, please run the module as a command line program eg.
	bash-prompt$ python libcarma.py --help
	and
	bash-prompt$ python libcarma.py
"""

import numpy as np
import math as math
import scipy.stats as spstats
from scipy.interpolate import UnivariateSpline
import cmath as cmath
import random
import sys as sys
import abc as abc
import psutil as psutil
import types as types
import os as os
import reprlib as reprlib
import copy as copy
import warnings as warnings
import matplotlib.pyplot as plt
import pdb as pdb

from gatspy.periodic import LombScargleFast, SuperSmoother

try:
	import rand as rand
	import libcarma
	import bSMBHTask as bSMBHTask
	from util.mpl_settings import set_plot_params
except ImportError:
	print 'libbsmbh is not setup. Setup libbsmbh by sourcing bin/setup.sh'
	sys.exit(1)

fhgt = 10
fwid = 16
set_plot_params(useTex = True)

def d2r(degree):
	return degree*(math.pi/180.0)

def r2d(radian):
	return radian*(180.0/math.pi)

class binarySMBHTask(object):
	lenTheta = 8
	G = 6.67408e-11
	c = 299792458.0
	pi = 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679
	twoPi = 2.0*pi
	Parsec = 3.0857e16
	Day = 86164.090530833
	Year = 31557600.0
	SolarMass = 1.98855e30

	def __init__(self, nthreads = psutil.cpu_count(logical = True), nwalkers = 25*psutil.cpu_count(logical = True), nsteps = 250, maxEvals = 10000, xTol = 0.1, mcmcA = 2.0):
		try:
			assert nthreads > 0, r'nthreads must be greater than 0'
			assert type(nthreads) is types.IntType, r'nthreads must be an integer'
			assert nwalkers > 0, r'nwalkers must be greater than 0'
			assert type(nwalkers) is types.IntType, r'nwalkers must be an integer'
			assert nsteps > 0, r'nsteps must be greater than 0'
			assert type(nsteps) is types.IntType, r'nsteps must be an integer'
			assert maxEvals > 0, r'maxEvals must be greater than 0'
			assert type(maxEvals) is types.IntType, r'maxEvals must be an integer'
			assert xTol > 0.0, r'xTol must be greater than 0'
			assert type(xTol) is types.FloatType, r'xTol must be a float'
			self._ndims = self.lenTheta
			self._nthreads = nthreads
			self._nwalkers = nwalkers
			self._nsteps = nsteps
			self._maxEvals = maxEvals
			self._xTol = xTol
			self._mcmcA = mcmcA
			self._Chain = np.require(np.zeros(self._ndims*self._nwalkers*self._nsteps), requirements=['F', 'A', 'W', 'O', 'E'])
			self._LnPosterior = np.require(np.zeros(self._nwalkers*self._nsteps), requirements=['F', 'A', 'W', 'O', 'E'])
			self._taskCython = bSMBHTask.bSMBHTask(self._nthreads)
		except AssertionError as err:
			raise AttributeError(str(err))

	@property
	def nthreads(self):
		return self._nthreads

	@property
	def ndims(self):
		return self._ndims

	@property
	def nwalkers(self):
		return self._nwalkers

	@nwalkers.setter
	def nwalkers(self, value):
		try:
			assert value >= 0, r'nwalkers must be greater than or equal to 0'
			assert type(value) is types.IntType, r'nwalkers must be an integer'
			self._nwalkers = value
			self._Chain = np.zeros(self._ndims*self._nwalkers*self._nsteps)
			self._LnPosterior = np.zeros(self._nwalkers*self._nsteps)
		except AssertionError as err:
			raise AttributeError(str(err))

	@property
	def nsteps(self):
		return self._nsteps

	@nsteps.setter
	def nsteps(self, value):
		try:
			assert value >= 0, r'nsteps must be greater than or equal to 0'
			assert type(value) is types.IntType, r'nsteps must be an integer'
			self._nsteps = value
			self._Chain = np.zeros(self._ndims*self._nwalkers*self._nsteps)
			self._LnPosterior = np.zeros(self._nwalkers*self._nsteps)
		except AssertionError as err:
			raise AttributeError(str(err))

	@property
	def maxEvals(self):
		return self._maxEvals

	@maxEvals.setter
	def maxEvals(self, value):
		try:
			assert value >= 0, r'maxEvals must be greater than or equal to 0'
			assert type(value) is types.IntType, r'maxEvals must be an integer'
			self._maxEvals = value
		except AssertionError as err:
			raise AttributeError(str(err))

	@property
	def xTol(self):
		return self._xTol

	@xTol.setter
	def xTol(self, value):
		try:
			assert value >= 0, r'xTol must be greater than or equal to 0'
			assert type(value) is types.FloatType, r'xTol must be a float'
			self._xTol = value
		except AssertionError as err:
			raise AttributeError(str(err))

	@property
	def mcmcA(self):
		return self._mcmcA

	@mcmcA.setter
	def mcmcA(self, value):
		try:
			assert value >= 0, r'mcmcA must be greater than or equal to 0.0'
			assert type(value) is types.FloatType, r'mcmcA must be a float'
			self._mcmcA = value
		except AssertionError as err:
			raise AttributeError(str(err))

	@property
	def Chain(self):
		return np.reshape(self._Chain, newshape = (self._ndims, self._nwalkers, self._nsteps), order = 'F')

	@property
	def LnPosterior(self):
		return np.reshape(self._LnPosterior, newshape = (self._nwalkers, self._nsteps), order = 'F')

	def __repr__(self):
		return "libsmbh.task(%d, %d, %d, %d, %f)"%(self._nthreads, self._nwalkers, self._nsteps, self._maxEvals, self._xTol)

	def __str__(self):
		line = 'ndims: %d\n'%(self._ndims)
		line += 'nthreads (Number of hardware threads to use): %d\n'%(self._nthreads)
		line += 'nwalkers (Number of MCMC walkers): %d\n'%(self._nwalkers)
		line += 'nsteps (Number of MCMC steps): %d\n'%(self.nsteps)
		line += 'maxEvals (Maximum number of evaluations when attempting to find starting location for MCMC): %d\n'%(self._maxEvals)
		line += 'xTol (Fractional tolerance in optimized parameter value): %f'%(self._xTol)

	def __eq__(self, other):
		if type(other) == task:
			if (self._nthreads == other.nthreads) and (self._nwalkers == other.nwalkers) and (self._nsteps == other.nsteps) and (self._maxEvals == other.maxEvals) and (self.xTol == other.xTol):
				return True
			else:
				return False
		else:
			return False

	def __neq__(self, other):
		if self == other:
			return False
		else:
			return True

	def reset(self, nwalkers = None, nsteps = None):
		if nwalkers is None:
			nwalkers = self._nwalkers
		if nsteps is None:
			nsteps = self._nsteps
		try:
			assert nwalkers > 0, r'nwalkers must be greater than 0'
			assert type(nwalkers) is types.IntType, r'nwalkers must be an integer'
			assert nsteps > 0, r'nsteps must be greater than 0'
			assert type(nsteps) is types.IntType, r'nsteps must be an integer'
			self._nwalkers = nwalkers
			self._nsteps = nsteps
			self._Chain = np.zeros(self._ndims*self._nwalkers*self._nsteps)
			self._LnPosterior = np.zeros(self._nwalkers*self._nsteps)
		except AssertionError as err:
			raise AttributeError(str(err))

	def check(self, Theta, tnum = None):
		if tnum is None:
			tnum = 0
		assert Theta.shape == (self._ndims,), r'Too many coefficients in Theta'
		return bool(self._taskCython.check_Theta(Theta, tnum))

	def set(self, Theta, tnum = None):
		if tnum is None:
			tnum = 0
		assert Theta.shape == (self._ndims,), r'Too many coefficients in Theta'
		return self._taskCython.set_System(Theta, tnum)

	def Theta(self, tnum = None):
		if tnum is None:
			tnum = 0
		Theta = np.zeros(self._ndims)
		self._taskCython.get_Theta(Theta, tnum)
		return Theta

	def list(self):
		setSystems = np.zeros(self._nthreads, dtype = 'int32')
		self._taskCython.get_setSystemsVec(setSystems)
		return setSystems.astype(np.bool_)

	def show(self, tnum = None):
		if tnum is None:
			tnum = 0
		self._taskCython.print_System(tnum)

	def __call__(self, epochVal, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.set_Epoch(epochVal, tnum)

	def epoch(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_Epoch(tnum)

	def period(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_Period(tnum)

	def a1(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_A1(tnum)

	def a2(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_A2(tnum)

	def m1(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_M1(tnum)

	def m2(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_M2(tnum)

	def rPeri1(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_RPeri1(tnum)

	def rPeri2(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_RPeri2(tnum)

	def rApo1(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_RApo1(tnum)

	def rApo2(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_RApo2(tnum)

	def rPeri(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_RPeriTot(tnum)

	def rApo(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_RApoTot(tnum)

	def rS1(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_RS1(tnum)

	def rS2(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_RS2(tnum)

	def eccentricity(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_Eccentricity(tnum)

	def omega1(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_Omega1(tnum)

	def omega2(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_Omega2(tnum)

	def inclination(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_Inclination(tnum)

	def tau(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_Tau(tnum)

	def M(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_MeanAnomoly(tnum)

	def E(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_EccentricAnomoly(tnum)

	def nu(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_TrueAnomoly(tnum)

	def r1(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_R1(tnum)

	def r2(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_R2(tnum)

	def theta1(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_Theta1(tnum)

	def theta2(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_Theta2(tnum)

	def Beta1(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_Beta1(tnum)

	def Beta2(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_Beta2(tnum)

	def radialBeta1(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_RadialBeta1(tnum)

	def radialBeta2(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_RadialBeta2(tnum)

	def dopplerFactor1(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_DopplerFactor1(tnum)

	def dopplerFactor2(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_DopplerFactor2(tnum)

	def beamingFactor1(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_BeamingFactor1(tnum)

	def beamingFactor2(self, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_BeamingFactor2(tnum)

	def aH(self, sigmaStars = 200.0, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_aH(sigmaStars, tnum)

	def aGW(self, sigmaStars = 200.0, rhoStars = 1000.0, H = 16, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_aGW(sigmaStars, rhoStars, H, tnum)

	def durationInHardState(self, sigmaStars = 200.0, rhoStars = 1000.0, H = 16, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_durationInHardState(sigmaStars, rhoStars, H, tnum)

	def ejectedMass(self, sigmaStars = 200.0, rhoStars = 1000.0, H = 16, tnum = None):
		if tnum is None:
			tnum = 0
		return self._taskCython.get_ejectedMass(sigmaStars, rhoStars, H, tnum)

	def simulate(self, duration, dt = None, fracNoiseToSignal = 0.001, tnum = None):
		if tnum is None:
			tnum = 0
		if dt is None:
			dt = self.period()/100.0
		numCadences = int(round(float(duration)/dt))
		intrinsicLC = libcarma.basicLC(numCadences, dt = dt, fracNoiseToSignal = fracNoiseToSignal)
		self._taskCython.make_IntrinsicLC(intrinsicLC.numCadences, intrinsicLC.dt, intrinsicLC.fracNoiseToSignal, intrinsicLC.t, intrinsicLC.x, intrinsicLC.y, intrinsicLC.yerr, intrinsicLC.mask, threadNum = tnum)
		intrinsicLC._simulatedCadenceNum = numCadences - 1
		intrinsicLC._T = intrinsicLC.t[-1] - intrinsicLC.t[0] 
		return intrinsicLC

	def observe(self, intrinsicLC, noiseSeed = None, tnum = None):
		if tnum is None:
			tnum = 0
		randSeed = np.zeros(1, dtype = 'uint32')
		if noiseSeed is None:
			rand.rdrand(randSeed)
			noiseSeed = randSeed[0]
		self._taskCython.add_ObservationNoise(intrinsicLC.numCadences, intrinsicLC.dt, intrinsicLC.fracNoiseToSignal, intrinsicLC.t, intrinsicLC.x, intrinsicLC.y, intrinsicLC.yerr, intrinsicLC.mask, noiseSeed, threadNum = tnum)

		count = int(np.sum(intrinsicLC.mask))
		y_meanSum = 0.0
		yerr_meanSum = 0.0
		for i in xrange(intrinsicLC.numCadences):
			y_meanSum += intrinsicLC.mask[i]*intrinsicLC.y[i]
			yerr_meanSum += intrinsicLC.mask[i]*intrinsicLC.yerr[i]
		if count > 0.0:
			intrinsicLC._mean = y_meanSum/count
			intrinsicLC._meanerr = yerr_meanSum/count
		else:
			intrinsicLC._mean = 0.0
			intrinsicLC._meanerr = 0.0
		y_stdSum = 0.0
		yerr_stdSum = 0.0
		for i in xrange(intrinsicLC.numCadences):
			y_stdSum += math.pow(intrinsicLC.mask[i]*intrinsicLC.y[i] - intrinsicLC._mean, 2.0)
			yerr_stdSum += math.pow(intrinsicLC.mask[i]*intrinsicLC.yerr[i] - intrinsicLC._meanerr, 2.0)
		if count > 0.0:
			intrinsicLC._std = math.sqrt(y_stdSum/count)
			intrinsicLC._stderr = math.sqrt(yerr_stdSum/count)
		else:
			intrinsicLC._std = 0.0
			intrinsicLC._stderr = 0.0

	def logPrior(self, observedLC, forced = True, tnum = None):
		if tnum is None:
			tnum = 0
		lowestFlux = np.min(observedLC.y[np.where(observedLC.mask == 1.0)])
		highestFlux = np.max(observedLC.y[np.where(observedLC.mask == 1.0)])
		observedLC._logPrior =  self._taskCython.compute_LnPrior(observedLC.numCadences, observedLC.dt, lowestFlux, highestFlux, observedLC.t, observedLC.x, observedLC.y, observedLC.yerr, observedLC.mask, tnum)
		return observedLC._logPrior

	def logLikelihood(self, observedLC, forced = True, tnum = None):
		if tnum is None:
			tnum = 0
		observedLC._logPrior = self.logPrior(observedLC, forced = forced, tnum = tnum)
		if forced == True:
			observedLC._computedCadenceNum = -1
		if observedLC._computedCadenceNum == -1:
			observedLC._logLikelihood = self._taskCython.compute_LnLikelihood(observedLC.numCadences, observedLC.dt, observedLC._computedCadenceNum, observedLC.t, observedLC.x, observedLC.y, observedLC.yerr, observedLC.mask, tnum)
			observedLC._logPosterior = observedLC._logPrior + observedLC._logLikelihood
			observedLC._computedCadenceNum = observedLC.numCadences - 1
		else:
			pass
		return observedLC._logLikelihood

	def logPosterior(self, observedLC, forced = True, tnum = None):
		lnLikelihood = self.logLikelihood(observedLC, forced = forced, tnum = tnum)
		return observedLC._logPosterior

		try:
			omega1 = math.acos((1.0/eccentricity)*((maxBetaParallel + minBetaParallel)/(maxBetaParallel - minBetaParallel)))
		except ValueError:
			pdb.set_trace()
		return omega1

	def estimate(self, observedLC):
		"""!
		Estimate intrinsicFlux, period, eccentricity, omega, tau, & a2sini 
		"""
		## intrinsicFluxEst
		maxPeriodFactor = 10.0
		model = LombScargleFast().fit(observedLC.t, observedLC.y, observedLC.yerr)
		periods, power = model.periodogram_auto(nyquist_factor = observedLC.numCadences)
		model.optimizer.period_range = (2.0*np.mean(observedLC.t[1:] - observedLC.t[:-1]), maxPeriodFactor*observedLC.T)
		periodEst = model.best_period
		numIntrinsicFlux = 100
		lowestFlux = np.min(observedLC.y[np.where(observedLC.mask == 1.0)])
		highestFlux = np.max(observedLC.y[np.where(observedLC.mask == 1.0)])
		intrinsicFlux = np.linspace(np.min(observedLC.y[np.where(observedLC.mask == 1.0)]), np.max(observedLC.y[np.where(observedLC.mask == 1.0)]), num = numIntrinsicFlux)
		intrinsicFluxList = list()
		totalIntegralList = list()
		for f in xrange(1, numIntrinsicFlux - 1):
			beamedLC = observedLC.copy()
			beamedLC.x = np.require(np.zeros(beamedLC.numCadences), requirements=['F', 'A', 'W', 'O', 'E'])
			for i in xrange(beamedLC.numCadences):
				beamedLC.y[i] = observedLC.y[i]/intrinsicFlux[f]
				beamedLC.yerr[i] = observedLC.yerr[i]/intrinsicFlux[f]
			dopplerLC = beamedLC.copy()
			dopplerLC.x = np.require(np.zeros(dopplerLC.numCadences), requirements=['F', 'A', 'W', 'O', 'E'])
			for i in xrange(observedLC.numCadences):
				dopplerLC.y[i] = math.pow(beamedLC.y[i], 1.0/3.44)
				dopplerLC.yerr[i] = (1.0/3.44)*math.fabs(dopplerLC.y[i]*(beamedLC.yerr[i]/beamedLC.y[i]))
			dzdtLC = dopplerLC.copy()
			dzdtLC.x = np.require(np.zeros(dopplerLC.numCadences), requirements=['F', 'A', 'W', 'O', 'E'])
			for i in xrange(observedLC.numCadences):
				dzdtLC.y[i] = 1.0 - (1.0/dopplerLC.y[i])
				dzdtLC.yerr[i] = math.fabs((-1.0*dopplerLC.yerr[i])/math.pow(dopplerLC.y[i], 2.0))
			foldedLC = dzdtLC.fold(periodEst)
			foldedLC.x = np.require(np.zeros(foldedLC.numCadences), requirements=['F', 'A', 'W', 'O', 'E'])
			integralSpline = UnivariateSpline(foldedLC.t[np.where(foldedLC.mask == 1.0)], foldedLC.y[np.where(foldedLC.mask == 1.0)], 1.0/foldedLC.yerr[np.where(foldedLC.mask == 1.0)], k = 3, s = None, check_finite = True)
			totalIntegral = math.fabs(integralSpline.integral(foldedLC.t[0], foldedLC.t[-1]))
			intrinsicFluxList.append(intrinsicFlux[f])
			totalIntegralList.append(totalIntegral)
		intrinsicFluxEst = intrinsicFluxList[np.where(np.array(totalIntegralList) == np.min(np.array(totalIntegralList)))[0][0]]

		## periodEst
		for i in xrange(beamedLC.numCadences):
			beamedLC.y[i] = observedLC.y[i]/intrinsicFluxEst
			beamedLC.yerr[i] = observedLC.yerr[i]/intrinsicFluxEst
			dopplerLC.y[i] = math.pow(beamedLC.y[i], 1.0/3.44)
			dopplerLC.yerr[i] = (1.0/3.44)*math.fabs(dopplerLC.y[i]*(beamedLC.yerr[i]/beamedLC.y[i]))
			dzdtLC.y[i] = 1.0 - (1.0/dopplerLC.y[i])
			dzdtLC.yerr[i] = math.fabs((-1.0*dopplerLC.yerr[i])/math.pow(dopplerLC.y[i], 2.0))
		model = LombScargleFast().fit(dzdtLC.t, dzdtLC.y, dzdtLC.yerr)
		periods, power = model.periodogram_auto(nyquist_factor = dzdtLC.numCadences)
		model.optimizer.period_range = (2.0*np.mean(dzdtLC.t[1:] - dzdtLC.t[:-1]), maxPeriodFactor*dzdtLC.T)
		periodEst = model.best_period

		## eccentricityEst & omega2Est
		# First find a full period going from rising to falling. 
		risingSpline = UnivariateSpline(dzdtLC.t[np.where(dzdtLC.mask == 1.0)], dzdtLC.y[np.where(dzdtLC.mask == 1.0)], 1.0/dzdtLC.yerr[np.where(dzdtLC.mask == 1.0)], k = 3, s = None, check_finite = True)
		risingSplineRoots = risingSpline.roots()
		firstRoot = risingSplineRoots[0]
		if risingSpline.derivatives(risingSplineRoots[0])[1] > 0.0:
			tRising = risingSplineRoots[0]
		else:
			tRising = risingSplineRoots[1]
		# Now fold the LC starting at tRising and going for a full period.
		foldedLC = dzdtLC.fold(periodEst, tStart = tRising)
		foldedLC.x = np.require(np.zeros(foldedLC.numCadences), requirements=['F', 'A', 'W', 'O', 'E'])
		# Fit the folded LC with a spline to figure out alpha and beta
		fitLC = foldedLC.copy()
		foldedSpline = UnivariateSpline(foldedLC.t[np.where(foldedLC.mask == 1.0)], foldedLC.y[np.where(foldedLC.mask == 1.0)], 1.0/foldedLC.yerr[np.where(foldedLC.mask == 1.0)], k = 3, s = 2*foldedLC.numCadences, check_finite = True)
		for i in xrange(fitLC.numCadences):
			fitLC.x[i] = foldedSpline(fitLC.t[i])
		# Now get the roots and find the falling root
		tZeros = foldedSpline.roots()
		if tZeros.shape[0] == 1: # We have found just tFalling
			tFalling = tZeros[0]
			tRising = fitLC.t[0]
			startIndex = 0
			tFull = fitLC.t[-1]
			stopIndex = fitLC.numCadences
		elif tZeros.shape[0] == 2: # We have found tFalling and one of tRising or tFull
			if foldedSpline.derivatives(tZeros[0])[1] < 0.0:
				tFalling = tZeros[0]
				tFull = tZeros[1]
				stopIndex = np.where(fitLC.t < tFull)[0][-1]
				tRising = fitLC.t[0]
				startIndex = 0
			elif foldedSpline.derivatives(tZeros[0])[1] > 0.0:
				if foldedSpline.derivatives(tZeros[1])[1] < 0.0:
					tRising = tZeros[0]
					startIndex = np.where(fitLC.t > tRising)[0][0]
					tFalling = tZeros[1]
					tFull = fitLC.t[-1]
					stopIndex = fitLC.numCadences
				else:
					raise RuntimeError('Could not determine alpha & omega correctly because the first root is rising but the second root is not falling!')
		elif tZeros.shape[0] == 3:
			tRising = tZeros[0]
			startIndex = np.where(fitLC.t > tRising)[0][0]
			tFalling = tZeros[1]
			tFull = tZeros[2]
			stopIndex = np.where(fitLC.t < tFull)[0][-1]
		else:
			raise RuntimeError('Could not determine alpha & omega correctly because tZeros has %d roots!'%(tZeros.shape[0]))
		# One full period now goes from tRising to periodEst. The maxima occurs between tRising and tFalling while the minima occurs between tFalling and tRising + periodEst  
		# Find the minima and maxima
		alpha = math.fabs(fitLC.x[np.where(np.max(fitLC.x[startIndex:stopIndex]) == fitLC.x)[0][0]])
		beta = math.fabs(fitLC.x[np.where(np.min(fitLC.x[startIndex:stopIndex]) == fitLC.x)[0][0]])
		peakLoc = fitLC.t[np.where(np.max(fitLC.x[startIndex:stopIndex]) == fitLC.x)[0][0]]
		troughLoc = fitLC.t[np.where(np.min(fitLC.x[startIndex:stopIndex]) == fitLC.x)[0][0]]
		KEst = 0.5*(alpha + beta)
		delta2 = (math.fabs(foldedSpline.integral(tRising, peakLoc)) + math.fabs(foldedSpline.integral(troughLoc, tFull)))/2.0
		delta1 = (math.fabs(foldedSpline.integral(peakLoc, tFalling)) + math.fabs(foldedSpline.integral(tFalling, troughLoc)))/2.0
		eCosOmega2 = (alpha - beta)/(alpha + beta)
		eSinOmega2 = ((2.0*math.sqrt(alpha*beta))/(alpha + beta))*((delta2 - delta1)/(delta2 + delta1))
		eccentricityEst = math.sqrt(math.pow(eCosOmega2, 2.0) + math.pow(eSinOmega2, 2.0))
		tanOmega2 = math.fabs(eSinOmega2/eCosOmega2)
		if (eCosOmega2/math.fabs(eCosOmega2) == 1.0) and (eSinOmega2/math.fabs(eSinOmega2) == 1.0):
			omega2Est = math.atan(tanOmega2)*(180.0/math.pi)
		if (eCosOmega2/math.fabs(eCosOmega2) == -1.0) and (eSinOmega2/math.fabs(eSinOmega2) == 1.0):
			omega2Est = 180.0 - math.atan(tanOmega2)*(180.0/math.pi)
		if (eCosOmega2/math.fabs(eCosOmega2) == -1.0) and (eSinOmega2/math.fabs(eSinOmega2) == -1.0):
			omega2Est = 180.0 + math.atan(tanOmega2)*(180.0/math.pi)
		if (eCosOmega2/math.fabs(eCosOmega2) == 1.0) and (eSinOmega2/math.fabs(eSinOmega2) == -1.0):
			omega2Est = 360.0 - math.atan(tanOmega2)*(180.0/math.pi)
		omega1Est = omega2Est - 180.0

		## tauEst
		zDot = KEst*(1.0 + eccentricityEst)*(eCosOmega2/eccentricityEst)
		zDotLC = dzdtLC.copy()
		for i in xrange(zDotLC.numCadences):
			zDotLC.y[i] = zDotLC.y[i] - zDot
		zDotSpline = UnivariateSpline(zDotLC.t[np.where(zDotLC.mask == 1.0)], zDotLC.y[np.where(zDotLC.mask == 1.0)], 1.0/zDotLC.yerr[np.where(zDotLC.mask == 1.0)], k = 3, s = 2*zDotLC.numCadences, check_finite = True)
		for i in xrange(zDotLC.numCadences):
			zDotLC.x[i] = zDotSpline(zDotLC.t[i])
		zDotZeros = zDotSpline.roots()
		zDotFoldedLC = dzdtLC.fold(periodEst)
		zDotFoldedSpline = UnivariateSpline(zDotFoldedLC.t[np.where(zDotFoldedLC.mask == 1.0)], zDotFoldedLC.y[np.where(zDotFoldedLC.mask == 1.0)], 1.0/zDotFoldedLC.yerr[np.where(zDotFoldedLC.mask == 1.0)], k = 3, s = 2*zDotFoldedLC.numCadences, check_finite = True)
		for i in xrange(zDotFoldedLC.numCadences):
			zDotFoldedLC.x[i] = zDotFoldedSpline(zDotFoldedLC.t[i])
		tC = zDotFoldedLC.t[np.where(np.max(zDotFoldedLC.x) == zDotFoldedLC.x)[0][0]]
		nuC = (360.0 - omega2Est)%360.0
		tE = zDotFoldedLC.t[np.where(np.min(zDotFoldedLC.x) == zDotFoldedLC.x)[0][0]]
		nuE = (180.0 - omega2Est)%360.0
		if math.fabs(360.0 - nuC) < math.fabs(360 - nuE):
			tauEst = zDotZeros[np.where(zDotZeros > tC)[0][0]]
		else:
			tauEst = zDotZeros[np.where(zDotZeros > tE)[0][0]]

		## a2sinInclinationEst
		a2sinInclinationEst = ((KEst*periodEst*self.Day*self.c*math.sqrt(1.0 - math.pow(eccentricityEst, 2.0)))/self.twoPi)/self.Parsec

		return intrinsicFluxEst, periodEst, eccentricityEst, omega1Est, tauEst, a2sinInclinationEst

	def fit(self, observedLC, zSSeed = None, walkerSeed = None, moveSeed = None, xSeed = None):
		randSeed = np.zeros(1, dtype = 'uint32')
		if zSSeed is None:
			rand.rdrand(randSeed)
			zSSeed = randSeed[0]
		if walkerSeed is None:
			rand.rdrand(randSeed)
			walkerSeed = randSeed[0]
		if moveSeed is None:
			rand.rdrand(randSeed)
			moveSeed = randSeed[0]
		if xSeed is None:
			rand.rdrand(randSeed)
			xSeed = randSeed[0]
		xStart = np.require(np.zeros(self.ndims*self.nwalkers), requirements=['F', 'A', 'W', 'O', 'E'])

		intrinsicFluxEst, periodEst, eccentricityEst, omega1Est, tauEst, a2sinInclinationEst = self.estimate(observedLC)

		''''ThetaGuess = np.array([0.001, 75.0, 10.0, 0.0, 0.0, 90.0, 0.0, 100.0, 0.5])
		for dimNum in xrange(self.ndims):
			xStart[dimNum] = ThetaGuess[dimNum]
		for walkerNum in xrange(1, self.nwalkers):'''
		for walkerNum in xrange(self.nwalkers):
			noSuccess = True
			while noSuccess:
				sinInclinationGuess = random.uniform(-1.0, 1.0)
				inclinationGuess = (180.0/math.pi)*math.asin(sinInclinationGuess)
				m1Guess = math.pow(10.0, random.uniform(-1.0, 3.0)) # m1 in 1e6*SolarMass
				m2Guess = math.pow(10.0, random.uniform(-1.0, math.log10(m1Guess))) # m1 in 1e6SolarMass
				rS1Guess = ((2.0*self.G*m1Guess*1.0e6*self.SolarMass)/math.pow(self.c, 2.0)/self.Parsec) # rS1 in Parsec
				rS2Guess = ((2.0*self.G*m2Guess*1.0e6*self.SolarMass)/math.pow(self.c, 2.0)/self.Parsec) # rS2 in Parsec
				rPeriEst = math.pow(((self.G*math.pow(periodEst*self.Day, 2.0)*((m1Guess + m2Guess)*1.0e6*self.SolarMass)*math.pow(1.0 + eccentricityEst, 3.0))/math.pow(self.twoPi, 2.0)),1.0/3.0)/self.Parsec
				a1Est = (m2Guess*rPeriEst)/((m1Guess + m2Guess)*(1.0 - eccentricityEst));
				a2Est = (m1Guess*rPeriEst)/((m1Guess + m2Guess)*(1.0 - eccentricityEst));


				ThetaGuess = np.array([rPeriEst, m1Guess, m2Guess, eccentricityGuess, omega1Est, inclinationGuess, tauGuess, totalFluxGuess])
				res = self.set(ThetaGuess)
				lnPrior = self.logPrior(observedLC)
				if res == 0 and lnPrior == 0.0:
					noSuccess = False
			for dimNum in xrange(self.ndims):
				xStart[dimNum + walkerNum*self.ndims] = ThetaGuess[dimNum]

		res = self._taskCython.fit_BinarySMBHModel(observedLC.numCadences, observedLC.dt, lowestFlux, highestFlux, observedLC.t, observedLC.x, observedLC.y, observedLC.yerr, observedLC.mask, self.nwalkers, self.nsteps, self.maxEvals, self.xTol, self.mcmcA, zSSeed, walkerSeed, moveSeed, xSeed, xStart, self._Chain, self._LnPosterior)
		return res