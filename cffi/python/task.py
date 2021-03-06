#!/usr/bin/env python
"""	Module that defines base class Task and SuppliedParametersTask.

	For a demonstration of the module, please run the module as a command line program using 
	bash-prompt$ python task.py --help
	and
	bash-prompt$ python task.py $PWD/examples/taskTest taskTest01.ini
"""
import math as math
import cmath as cmath
import numpy as np
import random as random
import copy as copy
import cffi as cffi
import inspect
import socket
HOST = socket.gethostname()
print 'HOST: %s'%(str(HOST))
import os as os
try: 
	os.environ['DISPLAY']
except KeyError as Err:
	print "No display environment! Using matplotlib backend 'Agg'"
	import matplotlib
	matplotlib.use('Agg')
import sys as sys
import time as time
import ConfigParser as CP
import argparse as AP
import hashlib as hashlib
import pdb

from bin._libcarma import ffi
import python.lc as lc

ffiObj = cffi.FFI()
try:
	libcarmaPath = str(os.environ['LIBCARMA_CFFI'])
except KeyError as Err:
	print str(Err) + '. Exiting....'
	sys.exit(1)
C = ffi.dlopen(libcarmaPath + '/bin/libcarma.so.1.0.0')
new_uint = ffiObj.new_allocator(alloc = C._malloc_uint, free = C._free_uint)
new_int = ffiObj.new_allocator(alloc = C._malloc_int, free = C._free_int)
new_double = ffiObj.new_allocator(alloc = C._malloc_double, free = C._free_double)

class Task(object):
	"""	Base Task class. All other tasks inherit from Task.
	"""
	def __init__(self, WorkingDirectory, ConfigFile, TimeStr, *args, **kwargs):
		"""	Initialize Task object.
		"""
		self.WorkingDirectory = WorkingDirectory
		self.ConfigFile = ConfigFile
		self.args = list(args)
		self.kwargs = dict(kwargs)
		self.preprefix = ConfigFile.split(".")[0]
		self.extension = ConfigFile.split(".")[1]
		self.PlotConfigFile = self.preprefix + "Plot." + self.extension
		try:
			self.ConfigFileHash = self.getHash(self.WorkingDirectory + self.ConfigFile)
		except IOError as Err:
			print str(Err) + ". Exiting..."
			sys.exit(1)
		try:
			self.PlotConfigFileHash = self.getHash(self.WorkingDirectory + self.PlotConfigFile)
		except IOError as Err:
			print str(Err) + ". Exiting..."
			sys.exit(1)
		self.SuppliedLCHash = ''
		try:
			TestFile = open(WorkingDirectory + self.preprefix + '_' + TimeStr + '.lc', 'r')
			TestFile.close()
			try:
				LogFile = open(self.WorkingDirectory + self.preprefix + '_' + TimeStr + '.log', 'r')
				LogFile.close()
			except IOError:
				print 'LogFile not found!'
				sys.exit(1)
			self.DateTime = TimeStr
			self.RunTime = None
			self.prefix = self.preprefix + '_' + self.DateTime
		except IOError as Err:
			try:
				LogFile = open(self.WorkingDirectory + self.preprefix + '_' + TimeStr + '.log', 'r')
				LogFile.close()
			except IOError as Err:
				LogFile = open(self.WorkingDirectory + self.preprefix + '_' + TimeStr + '.log', 'w')
				line = 'Starting log on ' + time.strftime("%m-%d-%Y") + ' at ' + time.strftime("%H:%M:%S") + '\n'
				LogFile.write(line)
				LogFile.close()
			self.DateTime = None
			self.RunTime = TimeStr
			self.prefix = self.preprefix + '_' + self.RunTime

		self.parser = CP.SafeConfigParser()
		self.parser.read(WorkingDirectory + self.ConfigFile)
		self.plotParser = CP.SafeConfigParser()
		self.plotParser.read(WorkingDirectory + self.PlotConfigFile)
		self.escChar = '#'
		self.LC = lc.LC()

	def rdrand(self, nrands, rands):
		yORn = C._getRandoms(nrands, rands)
		for i in xrange(nrands):
			if rands[i] == 0:
				rands[i] = np.random.randint(1,4294967296)
		return yORn

	def log(self, val):
		LogFile = open(self.WorkingDirectory + self.prefix + '.log', 'a')
		line = val + ' on ' + time.strftime("%m-%d-%Y") + ' at ' + time.strftime("%H:%M:%S") + '\n'
		LogFile.write(line)
		LogFile.close()

	def echo(self, val):
		line = val + ' on ' + time.strftime("%m-%d-%Y") + ' at ' + time.strftime("%H:%M:%S") + '\n'
		print line

	def strToBool(self, val):
		return val.lower() in ('yes', 'true', 't', '1')

	def formatFloat(self, val, formatStr = r'+3.2'):
		strVal = r'%' + formatStr + r'e'
		strVal = strVal%(val)
		frontVal = strVal[0:int(formatStr[1:2])+2]
		expLoc = strVal.find(r'e')
		expVal = strVal[expLoc+1:len(strVal)]
		try:
			expValInt = int(expVal)
		except ValueError:
			return str(val)
		if int(expVal) == 0:
			retVal = frontVal
		else:
			retVal = frontVal + r'\times 10^{' + expVal + r'}'
		return retVal

	def getHash(self, fullPathToFile):
		"""	Compute the hash value of HashFile
		"""
		hashFile = open(fullPathToFile, 'r')
		hashData = hashFile.read().replace('\n', '').replace(' ', '')
		hashObject = hashlib.sha512(hashData.encode())
		hashFile.close()
		return hashObject.hexdigest()

	def _read_00_escChar(self):
		""" Attempts to set the escape charatcter to be used.
		"""
		try:
			self.escChar = self.parser.get('MISC', 'escChar')
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.escChar = '#'

	def _read_01_basicPlotOptions(self):
		"""	Attempts to read in the plot options to be used.
		"""
		try:
			self.makePlot = self.strToBool(self.plotParser.get('PLOT', 'makePlot'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.makePlot = False
		try:
			self.JPG = self.strToBool(self.plotParser.get('PLOT', 'JPG'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.JPG = False
		try:
			self.PDF = self.strToBool(self.plotParser.get('PLOT', 'PDF'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.PDF = False
		try:
			self.EPS = self.strToBool(self.plotParser.get('PLOT', 'EPS'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.EPS = False
		try:
			self.PNG = self.strToBool(self.plotParser.get('PLOT', 'PNG'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.PNG = False
		try:
			self.showFig = self.strToBool(self.plotParser.get('PLOT', 'showFig'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.showFig = False
		try:
			self.dpi = int(self.plotParser.get('PLOT', 'dpi'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.dpi = 1200

	def parseConfig(self):
		"""	Subclasses define function(s) that extract parameter values from the Config dict. This function 
			sequentially calls them.
		"""
		self.methodList = inspect.getmembers(self, predicate=inspect.ismethod)
		for method in self.methodList:
			if method[0][0:6] == '_read_':
				method[1]()

	def run(self):
		self.parseConfig()
		self.methodList = inspect.getmembers(self, predicate=inspect.ismethod)
		for method in self.methodList:
			if method[0][0:6] == '_make_':
				method[1]()

class SuppliedParametersTask(Task):
	def _read_00_CARMAProps(self):
		"""	Attempts to parse AR roots and MA coefficients.
		"""

		ARRoots = list()
		ARPoly = list()
		MARoots = list()
		MAPoly = list()
		#self.ARCoefs = list()
		#self.ARRoots = list()

		try:
			self.RootPolyTol = float(self.parser.get('C-ARMA', 'RootPolyTol'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.RootPolyTol = 1.0e-6
			print str(Err) + '. Using default RootPolyTol = %+3.4e'%(self.RootPolyTol)

		doneReadingARRoots = False
		pRoot = 0
		while not doneReadingARRoots:
			try:
				ARRoots.append(complex(self.parser.get('C-ARMA', 'r_%d'%(pRoot + 1))))
				pRoot += 1
			except (CP.NoOptionError, CP.NoSectionError) as Err:
				doneReadingARRoots = True

		doneReadingARPoly = False
		pPoly = 0
		while not doneReadingARPoly:
			try:
				ARPoly.append(float(self.parser.get('C-ARMA', 'a_%d'%(pPoly + 1))))
				pPoly += 1
			except ValueError as Err:
				print str(Err) + '. All AR polynomial coefficients must be real!'
				sys.exit(1)
			except (CP.NoOptionError, CP.NoSectionError) as Err:
				doneReadingARPoly = True

		if (pRoot == pPoly):
			aPoly = np.polynomial.polynomial.polyfromroots(ARRoots)
			aPoly = aPoly.tolist()
			aPoly.reverse()
			aPoly.pop(0)
			aPoly = [coeff.real for coeff in aPoly]
			for ARCoef, aPolyCoef in zip(ARPoly, aPoly):
				if abs((ARCoef - aPolyCoef)/((ARCoef + aPolyCoef)/2.0)) > self.RootPolyTol:
					print 'ARRoots and ARPolynomial supplied are not equivalent!'
					sys.exit(1)
			self.p = pRoot
			self.ARRoots = np.array(ARRoots)
			self.ARCoefs = np.array(ARPoly)
		elif (pRoot == 0) and (pPoly > 0):
			self.p = pPoly
			self.ARCoefs = np.array(ARPoly)
			aPoly = copy.deepcopy(ARPoly)
			aPoly.insert(0,1.0)
			self.ARRoots = np.array(np.roots(aPoly))
		elif (pRoot > 0) and (pPoly == 0):
			self.p = pRoot
			self.ARRoots = np.array(ARRoots)
			aPoly = np.polynomial.polynomial.polyfromroots(ARRoots)
			aPoly = aPoly.tolist()
			aPoly.reverse()
			aPoly.pop(0)
			aPoly = [coeff.real for coeff in aPoly]
			self.ARCoefs = np.array(copy.deepcopy(aPoly))
		else:
			print 'ARRoots and ARPolynomial supplied are not equivalent!'
			sys.exit(1)

		doneReadingMARoots = False
		qRoot = 0
		while not doneReadingMARoots:
			try:
				MARoots.append(complex(self.parser.get('C-ARMA', 'm_%d'%(qRoot + 1))))
				qRoot += 1
			except (CP.NoOptionError, CP.NoSectionError) as Err:
				doneReadingMARoots = True
		self.noSigma = False
		try:
			self.Sigma = float(self.parser.get('C-ARMA', 'sigma'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.Sigma = 0.0
			self.noSigma = True
		if self.noSigma == False and self.Sigma <= 0.0:
			print 'sigma must be strictly postive!'
			sys.exit(1)

		doneReadingMAPoly = False
		qPoly = 0
		while not doneReadingMAPoly:
			try:
				MAPoly.append(float(self.parser.get('C-ARMA', 'b_%d'%(qPoly))))
				qPoly += 1
			except (CP.NoOptionError, CP.NoSectionError) as Err:
				doneReadingMAPoly = True

		if qRoot + 1 == qPoly and self.noSigma == False:
			bPoly=(np.polynomial.polynomial.polyfromroots(MARoots)).tolist()
			divisor=bPoly[0].real
			bPoly=[self.Sigma*(coeff.real/divisor) for coeff in bPoly]
			for MACoef, bPolyCoef in zip(MAPoly, bPoly):
				if abs((MACoef - bPolyCoef)/((MACoef + bPolyCoef)/2.0)) > self.RootPolyTol:
					print 'MARoots and MAPolynomial supplied are not equivalent!'
					sys.exit(1)
			self.q = qRoot
			self.MARoots = np.array(MARoots)
			self.MACoefs = np.array(MAPoly)
		elif (qRoot == 0) and (qPoly > 0):
			self.q = qPoly - 1
			self.MACoefs = np.array(MAPoly)
			bPoly = copy.deepcopy(MAPoly)
			self.Sigma = bPoly[0]
			bPoly.reverse()
			bPoly = [polyVal/self.Sigma for polyVal in bPoly]
			self.MARoots = np.array(np.roots(bPoly))
		elif ((qRoot > 0) or (self.Sigma > 0)) and (qPoly == 0):
			self.q = qRoot
			self.MARoots = np.array(MARoots)
			bPoly = (np.polynomial.polynomial.polyfromroots(MARoots)).tolist()
			divisor = bPoly[0].real
			bPoly = [self.Sigma*(coeff.real/divisor) for coeff in bPoly]
			MACoefs = copy.deepcopy(bPoly)
			self.MACoefs = np.array(MACoefs)
		else:
			print 'MARoots and MAPolynomial supplied are not equivalent!'
			sys.exit(1)

		if self.p < 1:
			print 'No C-AR roots supplied!'
			sys.exit(1)

		if self.q < 0:
			print 'No C-MA co-efficients supplied!'
			sys.exit(1)

		if self.p <= self.q:
			print 'Too many C-MA co-efficients! Exiting...'
			sys.exit(1)

		Theta_cffi = ffiObj.new("double[%d]"%(self.p + self.q + 1))
		for i in xrange(self.p):
			Theta_cffi[i] = self.ARCoefs[i]
		for i in xrange(self.q + 1):
			Theta_cffi[self.p + i] = self.MACoefs[i]
		yORn = C._testSystem(self.LC.dt, self.p, self.q, Theta_cffi)
		if yORn == 0:
			print 'Bad C-ARMA Parameters!'
			sys.exit(1)

		self.ndims = self.p + self.q + 1

		try:
			self.numBurn = int(self.parser.get('C-ARMA', 'numBurn'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.numBurn = 1000000
			print str(Err) + '. Using default numBurn = %d'%(self.numBurn)

	def _make_00_EqnString(self):
		"""	Attempts to construct a latex string consisting of the equation of the LC
		"""
		self.eqnStr = r'$'
		if self.p > 1:
			self.eqnStr += r'\mathrm{d}^{%d}F'%(self.p)
			for i in xrange(self.p - 2):
				self.eqnStr += (self.formatFloat(self.ARCoefs[i]) + r'\mathrm{d}^{%d}F'%(self.p - 1 - i))
			self.eqnStr += (self.formatFloat(self.ARCoefs[self.p - 2]) + r'\mathrm{d}F')
			self.eqnStr += (self.formatFloat(self.ARCoefs[self.p - 1]) + r'F=')
		elif self.p == 1:
			self.eqnStr += r'\mathrm{d}F'
			self.eqnStr += (self.formatFloat(self.ARCoefs[0]) + r'F=')
		self.eqnStr += (self.formatFloat(self.MACoefs[0]) + r'(\mathrm{d}W)')
		if self.q >= 1:
			self.eqnStr += (self.formatFloat(self.MACoefs[1]) + r'\mathrm{d}(\mathrm{d}W)')
		if self.q >= 2:
			for i in xrange(self.q - 1):
				self.eqnStr += (self.formatFloat(self.MACoefs[2 + i]) + r'\mathrm{d}^{%d}(\mathrm{d}W)'%(2 + i))
		self.eqnStr += r'$'

class SuppliedLCTask(Task):
	"""	Class for tasks where the LC is supplied externally.
	"""

	def _setIR(self):
		self.LC.IR = False
		for incr in self.LC.t_incr:
			if abs((incr - self.LC.dt)/((incr + self.LC.dt)/2.0)) > self.LC.tolIR:
				self.LC.IR = True

	def _readLC(self, suppliedLC = None):
		if suppliedLC == None:
			logEntry = 'Reading in LC'
		else:
			logEntry = 'Using suppliedLC to make mask'
		self.echo(logEntry)
		self.log(logEntry)
		if suppliedLC == None or suppliedLC == '':
			self.SuppliedLCFile = self.WorkingDirectory + self.prefix + '.lc'
		else:
			self.SuppliedLCFile = self.WorkingDirectory + suppliedLC
			self.SuppliedLCHash = self.getHash(self.SuppliedLCFile)
		inFile = open(self.SuppliedLCFile, 'rb')
		words = inFile.readline().rstrip('\n').split()
		LCHash = words[1]
		if (LCHash == self.ConfigFileHash) or (suppliedLC != None):
			inFile.readline()
			self.LC.numCadences = int(inFile.readline().rstrip('\n').split()[1])
			self.LC.cadence = np.array(self.LC.numCadences*[0])
			self.LC.mask = np.array(self.LC.numCadences*[0.0])
			self.LC.t = np.array(self.LC.numCadences*[0.0])
			self.LC.x = np.array(self.LC.numCadences*[0.0])
			self.LC.y = np.array(self.LC.numCadences*[0.0])
			self.LC.yerr = np.array(self.LC.numCadences*[0.0])
			numObservations = int(inFile.readline().rstrip('\n').split()[1])
			self.LC.meanFlux = float(inFile.readline().rstrip('\n').split()[1])
			self.LC.LnLike = float(inFile.readline().rstrip('\n').split()[1])
			line = inFile.readline()
			for i in xrange(self.LC.numCadences):
				words = inFile.readline().rstrip('\n').split()
				self.LC.cadence[i] = int(words[0])
				self.LC.mask[i] = float(words[1])
				self.LC.t[i] = float(words[2])
				self.LC.x[i] = float(words[3])
				self.LC.y[i] = float(words[4])
				self.LC.yerr[i] = float(words[5])
			self.LC.T = self.LC.t[-1] - self.LC.t[0]
			self.LC.t_incr = self.LC.t[1:] - self.LC.t[0:-1]
			self.LC.dt = np.median(self.LC.t_incr)
			self.LC.numObservations = numObservations
			self._setIR()
			inFile.close()
		else:
			print "Hash mismatch! The ConfigFile %s in WorkingDirectory %s has changed and no longer matches that used to make the light curve. Exiting!"%(self.ConfigFile, self.WorkingDirectory)
			inFile.close()
			sys.exit(1)

	def _read_00_plotOptions(self):
		try:
			self.showDetail = self.strToBool(self.plotParser.get('PLOT', 'showDetail'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.showDetail = True
		try:
			self.detailDuration = float(self.plotParser.get('PLOT', 'detailDuration'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.detailDuration = 1.0
		self.numPtsDetail = int(self.detailDuration/self.LC.dt)
		try:
			self.detailStart = self.plotParser.get('PLOT', 'detailStart')
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.detailStart = 'random'
		if self.detailStart == 'random':
			self.detailStart = random.randint(0, self.LC.numCadences - self.numPtsDetail)
		else:
			self.detailStart = int(self.detailStart)
			if self.detailStart > self.LC.numCadences:
				print "detailStart too large... Try reducing it."
				sys.exit(1)
		try:
			self.LabelLCFontsize = self.strToBool(self.plotParser.get('PLOT', 'LabelLCFontsize'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.LabelLCFontsize = 18
		try:
			self.DetailLabelLCFontsize = self.strToBool(self.plotParser.get('PLOT', 'DetailLabelLCFontsize'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.DetailLabelLCFontsize = 10
		try:
			self.showEqnLC = self.strToBool(self.plotParser.get('PLOT', 'showEqnLC'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.showEqnLC = True
		try:
			self.showLnLike = self.strToBool(self.plotParser.get('PLOT', 'showLnLike'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.showLnLike = True
		try:
			self.EqnLCLocY = float(self.plotParser.get('PLOT', 'EqnLCLocY'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.EqnLCLocY = 0.1
		try:
			self.EqnLCFontsize = int(self.plotParser.get('PLOT', 'EqnLCFontsize'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.EqnLCFontsize = 16
		try:
			self.showLegendLC = self.strToBool(self.plotParser.get('PLOT', 'showLegendLC'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.showLegendLC = True
		try:
			self.LegendLCLoc = int(self.plotParser.get('PLOT', 'LegendLCLoc'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.LegendLCLoc = 2
		try:
			self.LegendLCFontsize = int(self.plotParser.get('PLOT', 'LegendLCFontsize'))
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.LegendLCFontsize = 12
		try:
			self.xLabelLC = self.plotParser.get('PLOT', 'xLabelLC')
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.xLabelLC = r'$t$~($d$)'
		try:
			self.yLabelLC = self.plotParser.get('PLOT', 'yLabelLC')
		except (CP.NoOptionError, CP.NoSectionError) as Err:
			self.xLabelLC = r'$F$~($W m^{-2}$)'