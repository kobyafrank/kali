import math as math
import numpy as np
import os as os
import sys as sys
import pdb

try:
	import libcarma as libcarma
except ImportError:
	print 'libcarma is not setup. Setup libcarma by sourcing bin/setup.sh'
	sys.exit(1)

class sdssLC(libcarma.basicLC):

	def read(self, name, band, path = None, **kwargs):
		if path is None:
			path = os.environ['PWD']
		filePath = os.path.join(path, name)
		with open(filePath) as sdssFile:
			allLines = sdssFile.readlines()
		self._numCadences = len(allLines) - 1
		words = allLines[1].split(',')
		self._name = name
		self._band = band

		self._computedCadenceNum = -1
		self._tolIR = 1.0e-3
		self._fracIntrinsicVar = 0.0
		self._fracNoiseToSignal = 0.0
		self._maxSigma = 2.0
		self._minTimescale = 2.0
		self._maxTimescale = 0.5
		self._pSim = 0
		self._qSim = 0
		self._pComp = 0
		self._qComp = 0
		self._isSmoothed = False ## Has the LC been smoothed?
		self._dtSmooth = 0.0
		self.XSim = np.require(np.zeros(self.pSim), requirements=['F', 'A', 'W', 'O', 'E']) ## State of light curve at last timestamp
		self.PSim = np.require(np.zeros(self.pSim*self.pSim), requirements=['F', 'A', 'W', 'O', 'E']) ## Uncertainty in state of light curve at last timestamp.
		self.XComp = np.require(np.zeros(self.pComp), requirements=['F', 'A', 'W', 'O', 'E']) ## State of light curve at last timestamp
		self.PComp = np.require(np.zeros(self.pComp*self.pComp), requirements=['F', 'A', 'W', 'O', 'E']) ## Uncertainty in state of light curve at last timestamp.
		self.objId = np.require(np.zeros(self.numCadences, dtype = long), requirements=['F', 'A', 'W', 'O', 'E'])
		self.t = np.require(np.zeros(self.numCadences), requirements=['F', 'A', 'W', 'O', 'E'])
		self.x = np.require(np.zeros(self.numCadences), requirements=['F', 'A', 'W', 'O', 'E'])
		self.y = np.require(np.zeros(self.numCadences), requirements=['F', 'A', 'W', 'O', 'E'])
		self.yerr = np.require(np.zeros(self.numCadences), requirements=['F', 'A', 'W', 'O', 'E'])
		self.mask = np.require(np.zeros(self.numCadences), requirements=['F', 'A', 'W', 'O', 'E'])
		self._xunit = r'$d$ (MJD)' ## Unit in which time is measured (eg. s, sec, seconds etc...).
		self._yunit = r'$F$ (Jy)' ## Unit in which the flux is measured (eg Wm^{-2} etc...).

		if band.lower() in ['g', 'sdss-g', 'sdss_g']:
			self.startT = float(words[12])
			for lineNum, line in enumerate(allLines[1:]):
				words = line.split(',')
				self.objId[lineNum] = long(words[2])
				self.t[lineNum] = float(words[12]) - self.startT
				self.y[lineNum] = float(words[27])
				self.yerr[lineNum] = float(words[28])
				self.mask[lineNum] = 1.0
		elif band.lower() in ['r', 'sdss-r', 'sdss_r']:
			self.startT = float(words[15])
			for lineNum, line in enumerate(allLines[1:]):
				words = line.split(',')
				self.objId[lineNum] = long(words[2])
				self.t[lineNum] = float(words[15]) - self.startT
				self.y[lineNum] = float(words[29])
				self.yerr[lineNum] = float(words[30])
				self.mask[lineNum] = 1.0
		self._isRegular = False
		self._mean = np.mean(self.y)
		self._std = np.std(self.y)
		self._meanerr = np.mean(self.yerr)
		self._stderr = np.std(self.yerr)
		self._T = float(self.t[-1] - self.t[0])
		self._dt = float(np.nanmin(self.t[1:] - self.t[:-1]))

	def write(self):
		pass