import math as math
import cmath as cmath
import numpy as np
import random as random
import cffi as cffi
import socket
HOST = socket.gethostname()
print 'HOST: %s'%(str(HOST))
import os as os
try: 
	os.environ['DISPLAY']
	usingAGG = False
except KeyError as Err:
	print "No display environment! Using matplotlib backend 'Agg'"
	import matplotlib
	matplotlib.use('Agg')
	usingAGG = True

import matplotlib.pyplot as plt
import matplotlib.lines as mlines
import matplotlib.patches as mpatches
from matplotlib import gridspec, cm
import matplotlib.cm as colormap
import matplotlib.mlab as mlab
import time
import pdb

from bin._libcarma import ffi
import python.util.triangle as triangle
from python.util.mpl_settings import *
ffiObj = cffi.FFI()
try:
	libcarmaPath = str(os.environ['LIBCARMA_CFFI'])
except KeyError as Err:
	print str(Err) + '. Exiting....'
	sys.exit(1)
C = ffi.dlopen(libcarmaPath + '/bin/libcarma.so.1.0.0')

goldenRatio=1.61803398875
fhgt=10.0
fwid=fhgt*goldenRatio
dpi = 300

AnnotateXXLarge = 72
AnnotateXLarge = 48
AnnotateLarge = 32
AnnotateMedium = 28
AnnotateSmall = 24
AnnotateXSmall = 20
AnnotateXXSmall = 16

LegendLarge = 24
LegendMedium = 20
LegendSmall = 16

LabelXLarge = 32
LabelLarge = 28
LabelMedium = 24
LabelSmall = 20
LabelXSmall = 16

AxisXXLarge = 32
AxisXLarge = 28
AxisLarge = 24
AxisMedium = 20
AxisSmall = 16
AxisXSmall = 12
AxisXXSmall = 8

normalFontSize=32
smallFontSize=24
footnoteFontSize=20
scriptFontSize=16
tinyFontSize=12

LabelSize = LabelXLarge
AxisSize = AxisLarge
AnnotateSize = AnnotateXLarge
AnnotateSizeAlt = AnnotateMedium
AnnotateSizeAltAlt = AnnotateLarge
LegendSize = LegendMedium

gs = gridspec.GridSpec(1000, 2250) 

set_plot_params(fontfamily='serif',fontstyle='normal',fontvariant='normal',fontweight='normal',fontstretch='normal',fontsize=AxisMedium,useTex='True')

def MAD(a):
	medianVal=np.median(a)
	b=np.copy(a)
	for i in range(a.shape[0]):
		b[i]=abs(b[i]-medianVal)
	return np.median(b)

new_int = ffiObj.new_allocator(alloc = C._malloc_int, free = C._free_int)
new_double = ffiObj.new_allocator(alloc = C._malloc_double, free = C._free_double)

aListRoots = [-0.73642081+0.0j, -0.01357919-0.0j, -0.52329875-0.0j]#, -0.29848912+1.57831276j, -0.29848912-1.57831276j]
aPoly = np.polynomial.polynomial.polyfromroots(aListRoots)
aPoly = aPoly.tolist()
aPoly.reverse()
aPoly.pop(0)
aPoly = [coeff.real for coeff in aPoly]
p = len(aPoly)

bPoly = [7.0e-9, 1.2e-9]
q = len(bPoly) - 1
maxSigma = 7.0e-9*100.0

ndims = p + q + 1

Theta = aPoly + bPoly
ThetaLine = "Theta: "
for param in Theta:
	ThetaLine += " %8.7e"%(param)
print ThetaLine
Theta_cffi = ffiObj.new("double[%d]"%(len(Theta)))
xStart_cffi = ffiObj.new("double[%d]"%(len(Theta)))
for i in xrange(len(Theta)):
	Theta_cffi[i] = Theta[i]
	xStart_cffi[i] = Theta[i]
numBurn = 1000000
dt = 0.1
numCadences = 100
minTimescale = dt/1000.0
maxTimescale = dt*numCadences*100.0
startCadence = 0
burnSeed = 1311890535
distSeed = 2603023340
noiseSeed = 2410288857
fracInstrinsicVar = 1.0e-1
fracSignalToNoiseNone = 1.0e-15
fracSignalToNoiseKepler = 35.0e-6
fracSignalToNoiseLSST = 1.0e-3
fracSignalToNoiseSDSS = 1.0e-2
fracSignalToNoise = fracSignalToNoiseNone
nthreads = 4
nwalkers = 160
nsteps = 500
maxEvals = 1000
xTol = 0.005
tolIR = 1.0e-3
scatterFactor = 1.0e-1
zSSeed = 2229588325
walkerSeed = 3767076656
moveSeed = 2867335446
xSeed = 1413995162

doPrint = True
doTest = True
doR = True
doFitCARMA = True
doIR = True
makeIRR = True
irr_doFitCARMA = True
BreakAtEnd = False

##############################################################################################################

if doTest:
	yORn = C._testSystem(dt, p, q, Theta)
	if yORn == 1:
		print "System parameters are good!"
	else:
		print "System parameters are bad!"


##############################################################################################################


if doPrint:
	yORn = C._printSystem(dt, p, q, Theta)


##############################################################################################################


if doR:
	IR = 0

	cadence = np.array(numCadences*[0])
	mask = np.array(numCadences*[0.0])
	t = np.array(numCadences*[0.0])
	x = np.array(numCadences*[0.0])
	y = np.array(numCadences*[0.0])
	yerr = np.array(numCadences*[0.0])
	if doFitCARMA:
		Chain = np.zeros((nsteps,nwalkers,ndims))
		LnLike = np.zeros((nsteps,nwalkers))
		Deviances = np.zeros((nsteps,nwalkers))

	cadence_cffi = ffiObj.new("int[%d]"%(numCadences))
	mask_cffi = ffiObj.new("double[%d]"%(numCadences))
	t_cffi = ffiObj.new("double[%d]"%(numCadences))
	x_cffi = ffiObj.new("double[%d]"%(numCadences))
	y_cffi = ffiObj.new("double[%d]"%(numCadences))
	yerr_cffi = ffiObj.new("double[%d]"%(numCadences))
	if doFitCARMA:
		Chain_cffi = ffiObj.new("double[%d]"%(ndims*nwalkers*nsteps))
		LnLike_cffi = ffiObj.new("double[%d]"%(nwalkers*nsteps))

	for i in xrange(numCadences):
		cadence_cffi[i] = i
		mask_cffi[i] = 1.0
		t_cffi[i] = dt*i
		x_cffi[i] = 0.0
		y_cffi[i] = 0.0
		yerr_cffi[i] = 0.0
	if doFitCARMA:
		for stepNum in xrange(nsteps):
			for walkerNum in xrange(nwalkers):
				LnLike_cffi[walkerNum + stepNum*nwalkers] = 0.0
				for dimNum in xrange(ndims):
					Chain_cffi[dimNum + walkerNum*ndims + stepNum*ndims*nwalkers] = 0.0 
	
	intrinStart = time.time()
	yORn = C._makeIntrinsicLC(dt, p, q, Theta_cffi, IR, tolIR, numBurn, numCadences, startCadence, burnSeed, distSeed, cadence_cffi, mask_cffi, t_cffi, x_cffi)
	intrinStop = time.time()
	print "Time taken to compute intrinsic LC: %f (min)"%((intrinStop - intrinStart)/60.0)

	observedStart = time.time()
	yORn = C._makeObservedLC(dt, p, q, Theta_cffi, IR, tolIR, fracInstrinsicVar, fracSignalToNoise, numBurn, numCadences, startCadence, burnSeed, distSeed, noiseSeed, cadence_cffi, mask_cffi, t_cffi, y_cffi, yerr_cffi)
	observedStop = time.time()
	print "Time taken to compute observed LC: %f (min)"%((observedStop - observedStart)/60.0)

	for i in xrange(numCadences):
		cadence[i] = cadence_cffi[i]
		mask[i] = mask_cffi[i]
		t[i] = t_cffi[i]
		x[i] = x_cffi[i]
		y[i] = y_cffi[i]
		yerr[i] = yerr_cffi[i]

	x_mean = np.mean(x)
	y_mean = np.mean(y)
	x -= x_mean
	y -= y_mean
	for i in xrange(numCadences):
		x_cffi[i] = x[i] - x_mean
		y_cffi[i] = y[i] - y_mean

	fig1 = plt.figure(1,figsize=(fwid,fhgt))
	ax1 = fig1.add_subplot(gs[:,:])
	ax1.ticklabel_format(useOffset = False)
	ax1.plot(t, x, color = '#7570b3', zorder = 5)
	ax1.errorbar(t, y, yerr, fmt = '.', capsize = 0, color = '#d95f02', markeredgecolor = 'none', zorder = 10)
	yMax=np.max(x[np.nonzero(x[:])])
	yMin=np.min(x[np.nonzero(x[:])])
	ax1.set_ylabel(r'$F$ (arb units)')
	ax1.set_xlabel(r'$t$ (d)')
	ax1.set_xlim(t[0],t[-1])
	ax1.set_ylim(yMin,yMax)
	if usingAGG:
		fig1.savefig('./examples/fig1.jpg',dpi=70)

	LnLikeStart = time.time()
	LnLikeVal = C._computeLnLikelihood(dt, p, q, Theta_cffi, IR, tolIR, numCadences, cadence_cffi, mask_cffi, t_cffi, y_cffi, yerr_cffi)
	LnLikeStop = time.time()
	print "LnLike: %+17.17e"%(LnLikeVal)
	print "Time taken to compute LnLike of LC: %f (min)"%((LnLikeStop - LnLikeStart)/60.0)
	
	if doFitCARMA:
		#print "minTimescale: %+4.3e"%(minTimescale)
		#print "maxTimescale: %+4.3e"%(maxTimescale)
		fitStart = time.time()
		C._fitCARMA(dt, p, q, IR, tolIR, scatterFactor, numCadences, cadence_cffi, mask_cffi, t_cffi, y_cffi, yerr_cffi, maxSigma, minTimescale, maxTimescale, nthreads, nwalkers, nsteps, maxEvals, xTol, zSSeed, walkerSeed, moveSeed, xSeed, xStart_cffi, Chain_cffi, LnLike_cffi)
		fitStop = time.time()
		print "Time taken to estimate C-ARMA params of LC: %f (min)"%((fitStop - fitStart)/60.0)

		for stepNum in xrange(nsteps):
			for walkerNum in xrange(nwalkers):
				LnLike[stepNum,walkerNum] = LnLike_cffi[walkerNum + stepNum*nwalkers]
				Deviances[stepNum,walkerNum] = -2.0*LnLike_cffi[walkerNum + stepNum*nwalkers]
				for dimNum in xrange(ndims):
					Chain[stepNum,walkerNum,dimNum] = Chain_cffi[dimNum + walkerNum*ndims + stepNum*ndims*nwalkers]

		medianWalker = np.zeros((nsteps,ndims))
		medianDevWalker = np.zeros((nsteps,ndims))
		for i in range(nsteps):
			for k in range(ndims):
				medianWalker[i,k] = np.median(Chain[i,:,k])
				medianDevWalker[i,k] = MAD(Chain[i,:,k])
		stepArr = np.arange(nsteps)

		fig2 = plt.figure(2, figsize=(fwid,fhgt))
		for k in range(ndims):
			plt.subplot(ndims,1,k+1)
			for j in range(nwalkers):
				plt.plot(Chain[:,j,k], c = '#000000', alpha = 0.05, zorder = -5)
			plt.fill_between(stepArr[:], medianWalker[:,k] + medianDevWalker[:,k], medianWalker[:,k] - medianDevWalker[:,k], color = '#ff0000', edgecolor = '#ff0000', alpha = 0.5, zorder = 5)
			plt.plot(stepArr[:], medianWalker[:,k], c = '#dc143c', linewidth = 1, zorder = 10)
			plt.xlabel('stepNum')
			if (0 <= k < p):
				plt.ylabel("$a_{%d}$"%(k + 1))
			elif ((k >= p) and (k < ndims)):
				plt.ylabel("$b_{%d}$"%(k - p))
			if usingAGG:
				fig2.savefig('./examples/fig2.jpg',dpi=70)

		dictDIC = dict()
		samples = Chain[nsteps/2.0:,:,:].reshape((-1,ndims))
		sampleDeviances = Deviances[nsteps/2.0:,:].reshape((-1))
		DIC = 0.5*math.pow(np.std(sampleDeviances),2.0) + np.mean(sampleDeviances)
		dictDIC["%d %d"%(p,q)] = DIC
		lbls = list()
		for i in range(p):
			lbls.append("$a_{%d}$"%(i+1))
		for i in range(q + 1):
			lbls.append("$b_{%d}$"%(i))
		fig3, quantiles, qvalues = triangle.corner(samples, labels = lbls, fig_title = "DIC: %f"%(dictDIC["%d %d"%(p,q)]), show_titles = True, title_args = {"fontsize": 12}, quantiles = [0.16, 0.5, 0.84], verbose = False, plot_contours = True, plot_datapoints = True, plot_contour_lines = False, pcolor_cmap = cm.gist_earth)
		if usingAGG:
			fig3.savefig('./examples/fig3.jpg',dpi=70)


##############################################################################################################

if doIR:
	IR = 1

	irr_cadence = np.array([index for index in xrange(numCadences)])
	numCadences = irr_cadence.shape[0]
	irr_mask = np.array(numCadences*[1.0])
	irr_t = np.array([index*dt for index in xrange(numCadences)])
	if makeIRR:
		random.seed(983440498)
		for i in xrange(numCadences):
			irr_t[i] += random.uniform(-dt/2.0, dt/2.0)
		dt = np.median(irr_t[1:] - irr_t[:-1])
	irr_x = np.array(numCadences*[0.0])
	irr_y = np.array(numCadences*[0.0])
	irr_yerr = np.array(numCadences*[0.0])
	if irr_doFitCARMA:
		irr_Chain = np.zeros((nsteps,nwalkers,ndims))
		irr_LnLike = np.zeros((nsteps,nwalkers))
		irr_Deviances = np.zeros((nsteps,nwalkers))

	irr_cadence_cffi = ffiObj.new("int[%d]"%(numCadences))
	irr_mask_cffi = ffiObj.new("double[%d]"%(numCadences))
	irr_t_cffi = ffiObj.new("double[%d]"%(numCadences))
	irr_x_cffi = ffiObj.new("double[%d]"%(numCadences))
	irr_y_cffi = ffiObj.new("double[%d]"%(numCadences))
	irr_yerr_cffi = ffiObj.new("double[%d]"%(numCadences))
	if irr_doFitCARMA:
		irr_Chain_cffi = ffiObj.new("double[%d]"%(ndims*nwalkers*nsteps))
		irr_LnLike_cffi = ffiObj.new("double[%d]"%(nwalkers*nsteps))

	for i in xrange(numCadences):
		irr_cadence_cffi[i] = irr_cadence[i]
		irr_mask_cffi[i] = irr_mask[i]
		irr_t_cffi[i] = irr_t[i]
		irr_x_cffi[i] = irr_x[i]
		irr_y_cffi[i] = irr_y[i]
		irr_yerr_cffi[i] = irr_yerr[i]
	if irr_doFitCARMA:
		for stepNum in xrange(nsteps):
			for walkerNum in xrange(nwalkers):
				irr_LnLike_cffi[walkerNum + stepNum*nwalkers] = 0.0
				for dimNum in xrange(ndims):
					irr_Chain_cffi[dimNum + walkerNum*ndims + stepNum*ndims*nwalkers] = 0.0

	irr_intrinStart = time.time()
	yORn = C._makeIntrinsicLC(dt, p, q, Theta_cffi, IR, tolIR, numBurn, numCadences, startCadence, burnSeed, distSeed, irr_cadence_cffi, irr_mask_cffi, irr_t_cffi, irr_x_cffi)
	irr_intrinStop = time.time()
	print "Time taken to compute irregular intrinsic LC: %f (min)"%((irr_intrinStop - irr_intrinStart)/60.0)

	irr_observedStart = time.time()
	yORn = C._makeObservedLC(dt, p, q, Theta_cffi, IR, tolIR, fracInstrinsicVar, fracSignalToNoise, numBurn, numCadences, startCadence, burnSeed, distSeed, noiseSeed, irr_cadence_cffi, irr_mask_cffi, irr_t_cffi, irr_y_cffi, irr_yerr_cffi)
	irr_observedStop = time.time()
	print "Time taken to compute irregular observed LC: %f (min)"%((irr_observedStop - irr_observedStart)/60.0)

	for i in xrange(numCadences):
		irr_cadence[i] = irr_cadence_cffi[i]
		irr_mask[i] = irr_mask_cffi[i]
		irr_t[i] = irr_t_cffi[i]
		irr_x[i] = irr_x_cffi[i]
		irr_y[i] = irr_y_cffi[i]
		irr_yerr[i] = irr_yerr_cffi[i]

	irr_x_mean = np.mean(irr_x)
	irr_y_mean = np.mean(irr_y)
	irr_x -= irr_x_mean
	irr_y -= irr_y_mean
	for i in xrange(numCadences):
		irr_x_cffi[i] = irr_x[i] - irr_x_mean
		irr_y_cffi[i] = irr_y[i] - irr_y_mean

	fig4 = plt.figure(4,figsize=(fwid,fhgt))
	ax1 = fig4.add_subplot(gs[:,:])
	ax1.ticklabel_format(useOffset = False)
	ax1.plot(irr_t, irr_x, color = '#7570b3', zorder = 5)
	ax1.errorbar(irr_t, irr_y, irr_yerr, fmt = '.', capsize = 0, color = '#d95f02', markeredgecolor = 'none', zorder = 10)
	irr_yMax=np.max(irr_x[np.nonzero(irr_x[:])])
	irr_yMin=np.min(irr_x[np.nonzero(irr_x[:])])
	ax1.set_ylabel(r'$F$ (arb units)')
	ax1.set_xlabel(r'$t$ (d)')
	ax1.set_xlim(irr_t[0],irr_t[-1])
	ax1.set_ylim(irr_yMin,irr_yMax)
	if usingAGG:
		fig4.savefig('./examples/fig4.jpg',dpi=70)

	irr_LnLikeStart = time.time()
	LnLikeVal = C._computeLnLikelihood(dt, p, q, Theta_cffi, IR, tolIR, numCadences, irr_cadence_cffi, irr_mask_cffi, irr_t_cffi, irr_y_cffi, irr_yerr_cffi)
	irr_LnLikeStop = time.time()
	print "LnLike: %+17.17e"%(LnLikeVal)
	print "Time taken to compute LnLike of irregular LC: %f (min)"%((irr_LnLikeStop - irr_LnLikeStart)/60.0)

	if irr_doFitCARMA:
		irr_fitStart = time.time()
		C._fitCARMA(dt, p, q, IR, tolIR, scatterFactor, numCadences, irr_cadence_cffi, irr_mask_cffi, irr_t_cffi, irr_y_cffi, irr_yerr_cffi, maxSigma, minTimescale, maxTimescale, nthreads, nwalkers, nsteps, maxEvals, xTol, zSSeed, walkerSeed, moveSeed, xSeed, xStart_cffi, irr_Chain_cffi, irr_LnLike_cffi)
		irr_fitStop = time.time()
		print "Time taken to estimate C-ARMA params of irregular LC: %f (min)"%((irr_fitStop - irr_fitStart)/60.0)

		for stepNum in xrange(nsteps):
			for walkerNum in xrange(nwalkers):
				irr_LnLike[stepNum, walkerNum] = irr_LnLike_cffi[walkerNum + stepNum*nwalkers]
				irr_Deviances[stepNum,walkerNum] = -2.0*LnLike_cffi[walkerNum + stepNum*nwalkers]
				for dimNum in xrange(ndims):
					irr_Chain[stepNum, walkerNum, dimNum] = irr_Chain_cffi[dimNum + walkerNum*ndims + stepNum*ndims*nwalkers]

		irr_medianWalker = np.zeros((nsteps,ndims))
		irr_medianDevWalker = np.zeros((nsteps,ndims))
		for i in range(nsteps):
			for k in range(ndims):
				irr_medianWalker[i,k] = np.median(irr_Chain[i,:,k])
				irr_medianDevWalker[i,k] = MAD(irr_Chain[i,:,k])
		stepArr=np.arange(nsteps)

		fig5 = plt.figure(5,figsize=(fwid,fhgt))
		for k in range(ndims):
			plt.subplot(ndims,1,k+1)
			for j in range(nwalkers):
				plt.plot(irr_Chain[:,j,k], c = '#000000', alpha = 0.05, zorder = -5)
			plt.fill_between(stepArr[:], irr_medianWalker[:,k] + irr_medianDevWalker[:,k], irr_medianWalker[:,k] - irr_medianDevWalker[:,k], color = '#ff0000', edgecolor = '#ff0000', alpha = 0.5, zorder = 5)
			plt.plot(stepArr[:], irr_medianWalker[:,k], c = '#dc143c', linewidth = 1, zorder = 10)
			plt.xlabel('stepNum')
			if (0 <= k < p):
				plt.ylabel("$a_{%d}$"%(k + 1))
			elif ((k >= p) and (k < ndims)):
				plt.ylabel("$b_{%d}$"%(k - p))
			if usingAGG:
				fig5.savefig('./examples/fig5.jpg',dpi=70)

		irr_dictDIC = dict()
		irr_samples = irr_Chain[nsteps/2.0:,:,:].reshape((-1,ndims))
		irr_sampleDeviances = irr_Deviances[nsteps/2.0:,:].reshape((-1))
		irr_DIC = 0.5*math.pow(np.std(irr_sampleDeviances),2.0) + np.mean(irr_sampleDeviances)
		irr_dictDIC["%d %d"%(p,q)] = irr_DIC
		irr_lbls = list()
		for i in range(p):
			irr_lbls.append("$a_{%d}$"%(i+1))
		for i in range(q + 1):
			irr_lbls.append("$b_{%d}$"%(i))
		fig6, irr_quantiles, irr_qvalues = triangle.corner(irr_samples, labels = irr_lbls, fig_title = "DIC: %f"%(irr_dictDIC["%d %d"%(p,q)]), show_titles = True, title_args = {"fontsize": 12}, quantiles = [0.16, 0.5, 0.84], verbose = False, plot_contours = True, plot_datapoints = True, plot_contour_lines = False, pcolor_cmap = cm.gist_earth)
		if usingAGG:
			fig6.savefig('./examples/fig6.jpg',dpi=70)

if usingAGG:
	print 'Figures saved in ./examples directory'
else:
	plt.show()

if BreakAtEnd:
	pdb.set_trace()
