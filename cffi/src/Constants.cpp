#include <cmath>
#include <mathimf.h>
#include <complex>
#include "Constants.hpp"

using namespace std;

extern complex<double> complexZero (0.0,0.0);
extern complex<double> complexOne (1.0,0.0);
extern double pi = 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679;
extern double piSq = pow(pi, 2.0);
extern double e = 2.71828182845904523536028747135266249775724709369995;
extern double infiniteVal = HUGE_VAL;

extern double G = 6.67408e-11; // m^3/kg s^2
extern double c = 299792458.0; // m/s
extern double AU = 1.4960e11; // m
extern double Parsec = 3.0857e16; // m
extern double Day = 86164.090530833; //s
extern double Year = 31557600.0; // s
extern double kms = 1.0e3; // m/s
extern double SolarMass = 1.98855e30; // kg
extern double SolarMassPerCubicParsec = SolarMass/pow(Parsec, 3.0); // kg/m^3

extern double integrationTime = 6.019802903;
extern double readTime = 0.5189485261;
extern int numIntegrationsSC = 9;
extern int numIntegrationsLC = 270;

extern double samplingIntervalSC = (integrationTime+readTime)*numIntegrationsSC;
extern double samplingIntervalLC = (integrationTime+readTime)*numIntegrationsLC;

extern double samplingFrequencySC = 1.0/((integrationTime+readTime)*numIntegrationsSC);
extern double samplingFrequencyLC = 1.0/((integrationTime+readTime)*numIntegrationsLC);

extern double NyquistFrequencySC = samplingFrequencySC/2.0;
extern double NyquistFrequencyLC = samplingFrequencyLC/2.0;

extern double secPerSiderealDay = 86164.090530833;

//extern double scCadence = ((integrationTime+readTime)*numIntegrationsSC)/((double)3600.0*(double)23.9344696);
//extern double lcCadence = ((integrationTime+readTime)*numIntegrationsLC)/((double)3600.0*(double)23.9344696);

extern double scCadence = (integrationTime+readTime)*numIntegrationsSC;
extern double lcCadence = (integrationTime+readTime)*numIntegrationsLC;

extern double log2OfE = log2(e);

extern double log2Pi = log2(2.0*pi)/log2OfE;