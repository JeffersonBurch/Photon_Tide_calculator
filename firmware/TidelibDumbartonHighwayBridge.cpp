/*  TidelibDumbartonHighwayBridge.cpp 
 This source file contains a tide calculation function for the site listed
 below. This file and the associated header file should be placed in the
 Ardiuno/libraries/ directory inside a single folder.
 Luke Miller, 2015-12-22 
 http://github.com/millerlp/Tide_calculator
 Released under the GPL version 3 license.
 Compiled for Arduino v1.6.4 circa 2015

 The harmonic constituents used here were originally derived from 
 the Center for Operational Oceanic Products and Services (CO-OPS),
 National Ocean Service (NOS), National Oceanic and Atmospheric 
 Administration, U.S.A.
 The data were originally processed by David Flater for use with XTide,
 available at http://www.flaterco.com/xtide/files.html
 The predictions from this program should not be used for navigation
 and no accuracy or warranty is given or implied for these tide predictions.
 */
#include <time.h>
#include <math.h>
#include "TidelibDumbartonHighwayBridge.h"

unsigned int YearIndx = 0; // Used to index rows in the Equilarg/Nodefactor arrays
float currHours = 0;          // Elapsed hours since start of year
const int adjustGMT = 8;     // Time zone adjustment to get time in GMT.
//Make sure this is correct for the local standard time of the tide station.
// 8 = Pacific Standard Time (America/Los_Angeles)
/* Initialize harmonic constituent arrays. These each hold 37 values for
the tide site that was extracted using the R scripts:
tide_harmonics_parse.R and tide_harmonics_library_generator.R

The values are available from NOAA's http://tidesandcurrent.noaa.gov site.
Kappa here is referred to as 'Phase' on NOAA's site. The order of the
constituents is shown below in the names. Unfortunately this does not match
NOAA's order, so you will have to rearrange NOAA's values if you want to
put new site values in here by hand.
The Speed, Equilarg and Nodefactor arrays can all stay the same for any site.
*/

// Selected station: Dumbarton Highway Bridge, San Francisco Bay, California
const char stationID[] = "Dumbarton Highway Bridge, San Francisco Bay, California";
// Selection station ID number: 
const long stationIDnumber = 9414509;
// The 'datum' printed here is the difference between mean sea level and 
// mean lower low water for the NOAA station. These two values can be 
// found for NOAA tide reference stations on the tidesandcurrents.noaa.gov
//  site under the datum page for each station.
const float Datum = 4.6818 ; // units in feet
// Harmonic constant names: J1, K1, K2, L2, M1, M2, M3, M4, M6, M8, N2, 2N2, O1, OO1, P1, Q1, 2Q1, R2, S1, S2, S4, S6, T2, LDA2, MU2, NU2, RHO1, MK3, 2MK3, MN4, MS4, 2SM2, MF, MSF, MM, SA, SSA
// These names match the NOAA names, except LDA2 here is LAM2 on NOAA's site
typedef float prog_float_t; // Need to define this type before use
// Amp is the amplitude of each of the harmonic constituents for this site
const prog_float_t Amp[] = {0.056,1.378,0.236,0.19,0.046,3.038,0.023,0.059,0.056,0.01,0.591,0.069,0.81,0.039,0.449,0.148,0.03,0.007,0.072,0.65,0,0,0.033,0.039,0.069,0.138,0.039,0.105,0.151,0.03,0.043,0.026,0,0,0,0.125,0.128};
// Kappa is the 'modified' or 'adapted' phase lag (Epoch) of each of the 
// harmonic constituents for this site.
const prog_float_t Kappa[] = {281.9,244,244.6,237.1,272.4,246.7,354.4,10.2,49.9,105.8,225.6,199.6,228.9,283.2,240.8,228.1,215.9,264.9,218.3,264.2,0,0,249.3,212.2,25.7,226.8,215.5,95.6,80.6,356.7,23.3,75,0,0,0,221.4,286.9};
// Speed is the frequency of the constituent, denoted as little 'a' by Hicks 2006
const prog_float_t Speed[] = {15.58544,15.04107,30.08214,29.52848,14.49669,28.9841,43.47616,57.96821,86.95231,115.9364,28.43973,27.89535,13.94304,16.1391,14.95893,13.39866,12.85429,30.04107,15,30,60,90,29.95893,29.45563,27.96821,28.51258,13.47151,44.02517,42.92714,57.42383,58.9841,31.0159,1.098033,1.015896,0.5443747,0.0410686,0.0821373};
const prog_float_t Equilarg[24][37] = { 
{83.38,11.3,202.4,3.66,322.99,110.42,165.63,220.84,331.26,81.68,38.87,327.32,98.58,105.09,349.67,27.03,315.49,177.14,180,0,0,0,2.86,108.65,220.64,292.19,280.36,121.72,209.54,149.29,110.42,249.58,93.25,249.58,71.55,280.33,200.67},
{166.42,7.56,195.61,182.09,211.44,210.46,135.69,60.92,271.39,121.85,50.19,249.92,204.31,348,349.9,44.04,243.77,176.88,180,0,0,0,3.12,19.21,61.44,221.71,215.56,218.02,53.36,260.65,210.46,149.54,341.84,149.54,160.27,280.1,200.19},
{264.33,5.26,191.35,1.96,102.4,286.18,69.27,212.36,138.54,64.72,24.12,122.06,283.91,260.62,349.16,21.85,119.79,177.61,180,0,0,0,2.39,276.77,213.53,115.59,113.32,291.44,207.11,310.3,286.18,73.82,258.36,73.82,262.06,280.84,201.69},
{349.75,2.87,186.44,202.17,42.75,26.41,39.61,52.82,79.23,105.64,35.63,44.85,27.45,150.48,349.4,36.66,45.88,177.35,180,0,0,0,2.65,187.53,54.51,45.29,46.33,29.28,49.95,62.04,26.41,333.59,151.52,333.59,350.78,280.6,201.21},
{76.92,1.58,183.41,39.83,314.62,126.83,10.24,253.65,20.48,147.31,47.32,327.82,129.41,45.42,349.63,49.91,330.4,177.1,180,0,0,0,2.9,98.47,255.68,335.19,337.77,128.41,252.07,174.15,126.83,233.17,48,233.17,79.5,280.37,200.73},
{165.67,1.37,182.52,211.35,206.58,227.47,341.21,94.94,322.41,189.88,59.24,251.02,230.02,304.87,349.87,61.8,253.57,176.84,180,0,0,0,3.16,9.64,97.08,265.3,267.86,228.84,93.57,286.71,227.47,132.53,307.42,132.53,168.23,280.13,200.25},
{269.75,3.07,185.62,17.26,90.25,303.97,275.96,247.95,191.92,135.9,33.96,123.94,304.24,235.22,349.13,34.23,124.21,177.57,180,0,0,0,2.43,267.98,249.95,159.97,160.24,307.04,244.88,337.93,303.97,56.03,235.49,56.03,270.01,280.87,201.75},
{0.7,4.49,188.41,224.35,24.63,45.09,247.63,90.18,135.26,180.35,46.35,47.61,43.13,140.79,349.36,44.39,45.65,177.32,180,0,0,0,2.68,179.62,91.82,90.56,88.6,49.58,85.68,91.44,45.09,314.91,138.83,314.91,358.74,280.64,201.27},
{92.29,6.43,192.41,74.18,317.02,146.39,219.58,292.78,79.17,225.56,58.93,331.47,141.56,48.09,349.6,54.1,326.64,177.06,180,0,0,0,2.94,91.44,293.87,21.33,16.51,152.82,286.35,205.32,146.39,213.61,43.26,213.61,87.46,280.4,200.79},
{184.25,8.66,197.15,246.07,213.12,247.82,191.73,135.64,23.46,271.27,71.64,255.45,239.75,316.39,349.84,63.57,247.38,176.8,180,0,0,0,3.2,3.4,136.06,312.24,304.17,256.48,126.97,319.45,247.82,112.18,308.32,112.18,176.18,280.16,200.32},
{290.4,12,204.13,41.02,96.21,324.92,127.37,289.83,254.75,219.66,46.94,128.97,312.48,252.38,349.1,34.51,116.54,177.53,180,0,0,0,2.47,262.34,289.52,207.49,195.06,336.91,277.83,11.86,324.92,35.08,239.95,35.08,277.97,280.9,201.81},
{22.4,14.26,208.93,250.96,25.28,66.36,99.53,132.71,199.07,265.43,59.66,52.97,50.65,160.77,349.33,43.96,37.26,177.28,180,0,0,0,2.72,174.3,131.72,138.41,122.7,80.62,118.45,126.02,66.36,293.64,145.06,293.64,6.69,280.67,201.33},
{114.06,16.25,213.08,101.09,323.67,167.68,71.52,335.37,143.05,310.73,72.27,336.85,149.03,68.26,349.57,53.62,318.2,177.02,180,0,0,0,2.98,86.15,333.8,69.21,50.56,183.94,319.11,239.95,167.68,192.32,49.61,192.32,95.42,280.43,200.85},
{205.14,17.78,216.09,279.46,220.94,268.83,43.24,177.66,86.49,355.32,84.69,260.55,247.83,334.17,349.81,63.69,239.55,176.77,180,0,0,0,3.23,357.82,175.7,359.84,338.83,286.61,159.88,353.52,268.83,91.17,313.17,91.17,184.14,280.19,200.38},
{309.38,19.6,219.48,75.08,101.48,345.37,338.06,330.74,316.11,301.48,59.44,133.52,321.92,264.98,349.06,35.99,110.07,177.5,180,0,0,0,2.5,256.21,328.61,254.54,231.08,4.97,311.14,44.82,345.37,14.63,241.53,14.63,285.93,280.94,201.87},
{38.35,19.55,218.92,275.51,16.35,86.05,309.08,172.1,258.16,344.21,71.4,56.75,62.35,165.06,349.3,47.7,33.05,177.24,180,0,0,0,2.76,167.41,170.04,184.69,160.99,105.6,152.56,157.45,86.05,273.95,141.35,273.95,14.65,280.7,201.39},
{125.79,18.44,216.22,118.09,318.69,186.5,279.75,13.01,199.51,26.01,83.13,339.76,164.08,60.76,349.54,60.71,317.34,176.98,180,0,0,0,3.02,78.39,11.25,114.62,92.2,204.94,354.57,269.63,186.5,173.5,38.34,173.5,103.37,280.46,200.92},
{211.48,16.21,211.57,307.74,220.29,286.76,250.14,213.51,140.27,67.03,94.66,262.57,267.37,311.4,349.78,75.28,243.18,176.73,180,0,0,0,3.27,349.17,212.25,44.35,24.96,302.97,197.3,21.42,286.76,73.24,292.02,73.24,192.09,280.22,200.44},
{309.59,14.02,207.47,112.35,96.57,2.49,183.74,4.98,7.47,9.96,68.61,134.73,346.78,224.64,349.03,52.9,119.02,177.46,180,0,0,0,2.54,246.74,4.36,298.24,282.53,16.51,350.96,71.1,2.49,357.51,208.93,357.51,293.88,280.97,201.93},
{32.7,10.32,200.72,296.37,348.28,102.54,153.81,205.08,307.61,50.15,79.93,57.33,92.45,107.74,349.27,69.84,47.24,177.2,180,0,0,0,2.8,157.31,205.16,227.76,217.67,112.86,194.76,182.47,102.54,257.46,97.64,257.46,22.6,280.73,201.46},
{115.93,6.69,194.07,131.78,265.69,202.59,123.89,45.19,247.78,90.38,91.27,339.94,197.99,351.23,349.51,86.66,335.33,176.95,180,0,0,0,3.05,67.89,45.97,157.3,152.69,209.29,38.5,293.86,202.59,157.41,346.62,157.41,111.33,280.49,200.98},
{200.33,3.71,188.26,330.91,200.59,302.74,94.1,245.47,188.21,130.95,102.69,262.64,302.45,238.11,349.75,102.4,262.35,176.69,180,0,0,0,3.31,338.56,246.86,86.91,86.63,306.44,241.76,45.42,302.74,57.26,237.83,57.26,200.05,280.25,200.5},
{300.48,2.72,185.99,150.46,88.18,18.65,27.98,37.3,55.95,74.6,76.81,134.97,20.01,157.28,349,78.17,136.33,177.42,180,0,0,0,2.58,236.31,39.15,340.99,342.34,21.37,34.58,95.46,18.65,341.35,158.64,341.35,301.84,281,201.99},
{28.3,1.86,183.78,319.33,338.46,119.15,358.73,238.3,357.45,116.61,88.59,58.03,121.41,54.07,349.24,90.85,60.29,177.16,180,0,0,0,2.84,147.34,240.4,270.96,273.22,121.01,236.44,207.74,119.15,240.85,56.33,240.85,30.56,280.76,201.52} 
 };

const prog_float_t Nodefactor[24][37] = { 
{0.8278,0.8824,0.7472,0.878,1.5575,1.0377,1.0571,1.0768,1.1173,1.1594,1.0377,1.0377,0.8068,0.4868,1,0.8068,0.8068,1,1,1,1,1,1,1.0377,1.0377,1.0377,0.8068,0.9156,0.9501,1.0768,1.0377,1.0377,0.6271,1.0377,1.1307,1,1},
{0.8343,0.8864,0.7533,0.9704,1.4048,1.0367,1.0556,1.0747,1.1141,1.155,1.0367,1.0367,0.8135,0.5,1,0.8135,0.8135,1,1,1,1,1,1,1.0367,1.0367,1.0367,0.8135,0.9189,0.9526,1.0747,1.0367,1.0367,0.6381,1.0367,1.1272,1,1},
{0.8669,0.9068,0.7865,1.1656,0.9653,1.0315,1.0477,1.0641,1.0977,1.1323,1.0315,1.0315,0.8475,0.5711,1,0.8475,0.8475,1,1,1,1,1,1,1.0315,1.0315,1.0315,0.8475,0.9354,0.965,1.0641,1.0315,1.0315,0.6961,1.0315,1.109,1,1},
{0.9176,0.9394,0.8458,1.204,0.9343,1.0229,1.0345,1.0463,1.0702,1.0947,1.0229,1.0229,0.9011,0.6981,1,0.9011,0.9011,1,1,1,1,1,1,1.0229,1.0229,1.0229,0.9011,0.9609,0.9829,1.0463,1.0229,1.0229,0.7936,1.0229,1.0783,1,1},
{0.9761,0.9782,0.9272,0.9582,1.6115,1.0117,1.0176,1.0235,1.0354,1.0475,1.0117,1.0117,0.9643,0.8745,1,0.9643,0.9643,1,1,1,1,1,1,1.0117,1.0117,1.0117,0.9643,0.9897,1.0012,1.0235,1.0117,1.0117,0.9188,1.0117,1.039,1,1},
{1.0336,1.0176,1.0225,0.7337,1.9813,0.9992,0.9989,0.9985,0.9977,0.9969,0.9992,0.9992,1.0279,1.0859,1,1.0279,1.0279,1,1,1,1,1,1,0.9992,0.9992,0.9992,1.0279,1.0168,1.016,0.9985,0.9992,0.9992,1.0571,0.9992,0.9955,1,1},
{1.0836,1.0529,1.1201,1.0649,1.5936,0.987,0.9805,0.9741,0.9614,0.9489,0.987,0.987,1.085,1.309,1,1.085,1.085,1,1,1,1,1,1,0.987,0.987,0.987,1.085,1.0392,1.0257,0.9741,0.987,0.987,1.1924,0.987,0.953,1,1},
{1.1226,1.0812,1.2075,1.3148,1.0585,0.9763,0.9646,0.9531,0.9305,0.9084,0.9763,0.9763,1.131,1.5151,1,1.131,1.131,1,1,1,1,1,1,0.9763,0.9763,0.9763,1.131,1.0555,1.0305,0.9531,0.9763,0.9763,1.3097,0.9763,0.9161,1,1},
{1.1492,1.1009,1.2736,1.0393,1.7582,0.9683,0.9528,0.9376,0.9079,0.8791,0.9683,0.9683,1.1631,1.6753,1,1.1631,1.1631,1,1,1,1,1,1,0.9683,0.9683,0.9683,1.1631,1.066,1.0322,0.9376,0.9683,0.9683,1.3967,0.9683,0.8888,1,1},
{1.163,1.1112,1.3104,0.5914,2.2831,0.9639,0.9464,0.9291,0.8956,0.8633,0.9639,0.9639,1.1801,1.7658,1,1.1801,1.1801,1,1,1,1,1,1,0.9639,0.9639,0.9639,1.1801,1.0711,1.0324,0.9291,0.9639,0.9639,1.4444,0.9639,0.8738,1,1},
{1.164,1.112,1.3131,1.0008,1.8466,0.9636,0.9459,0.9285,0.8947,0.8621,0.9636,0.9636,1.1814,1.7726,1,1.1814,1.1814,1,1,1,1,1,1,0.9636,0.9636,0.9636,1.1814,1.0715,1.0324,0.9285,0.9636,0.9636,1.4479,0.9636,0.8727,1,1},
{1.1522,1.1031,1.2815,1.3288,1.0994,0.9674,0.9515,0.9358,0.9052,0.8757,0.9674,0.9674,1.1668,1.6946,1,1.1668,1.1668,1,1,1,1,1,1,0.9674,0.9674,0.9674,1.1668,1.0671,1.0323,0.9358,0.9674,0.9674,1.4069,0.9674,0.8856,1,1},
{1.1276,1.0849,1.2196,1.091,1.6268,0.9748,0.9625,0.9502,0.9263,0.903,0.9748,0.9748,1.137,1.544,1,1.137,1.137,1,1,1,1,1,1,0.9748,0.9748,0.9748,1.137,1.0576,1.0309,0.9502,0.9748,0.9748,1.3257,0.9748,0.9111,1,1},
{1.0904,1.0578,1.1347,0.6859,2.1026,0.9852,0.9779,0.9705,0.9561,0.942,0.9852,0.9852,1.093,1.3431,1,1.093,1.093,1,1,1,1,1,1,0.9852,0.9852,0.9852,1.093,1.0422,1.0267,0.9705,0.9852,0.9852,1.2123,0.9852,0.9468,1,1},
{1.042,1.0235,1.0379,0.9408,1.7337,0.9973,0.9959,0.9945,0.9918,0.9891,0.9973,0.9973,1.0374,1.1206,1,1.0374,1.0374,1,1,1,1,1,1,0.9973,0.9973,0.9973,1.0374,1.0207,1.0179,0.9945,0.9973,0.9973,1.0788,0.9973,0.9887,1,1},
{0.9854,0.9845,0.9416,1.2309,0.9912,1.0098,1.0147,1.0196,1.0296,1.0396,1.0098,1.0098,0.9745,0.9059,1,0.9745,0.9745,1,1,1,1,1,1,1.0098,1.0098,1.0098,0.9745,0.9942,1.0039,1.0196,1.0098,1.0098,0.9401,1.0098,1.0323,1,1},
{0.9266,0.9453,0.8573,1.1617,1.0896,1.0212,1.0321,1.0429,1.0651,1.0877,1.0212,1.0212,0.9107,0.7229,1,0.9107,0.9107,1,1,1,1,1,1,1.0212,1.0212,1.0212,0.9107,0.9654,0.9859,1.0429,1.0212,1.0212,0.8118,1.0212,1.0726,1,1},
{0.8739,0.9113,0.7941,0.9272,1.5305,1.0304,1.046,1.0617,1.094,1.1273,1.0304,1.0304,0.8549,0.5873,1,0.8549,0.8549,1,1,1,1,1,1,1.0304,1.0304,1.0304,0.8549,0.939,0.9675,1.0617,1.0304,1.0304,0.709,1.0304,1.1049,1,1},
{0.8378,0.8886,0.7567,0.8837,1.5595,1.0361,1.0547,1.0736,1.1124,1.1526,1.0361,1.0361,0.8171,0.5073,1,0.8171,0.8171,1,1,1,1,1,1,1.0361,1.0361,1.0361,0.8171,0.9207,0.954,1.0736,1.0361,1.0361,0.6442,1.0361,1.1253,1,1},
{0.8269,0.8818,0.7464,1.0589,1.2041,1.0378,1.0573,1.077,1.1177,1.16,1.0378,1.0378,0.8059,0.4851,1,0.8059,0.8059,1,1,1,1,1,1,1.0378,1.0378,1.0378,0.8059,0.9152,0.9498,1.077,1.0378,1.0378,0.6256,1.0378,1.1312,1,1},
{0.8441,0.8925,0.763,1.1994,0.7928,1.0352,1.0532,1.0716,1.1093,1.1483,1.0352,1.0352,0.8237,0.5207,1,0.8237,0.8237,1,1,1,1,1,1,1.0352,1.0352,1.0352,0.8237,0.9239,0.9564,1.0716,1.0352,1.0352,0.6552,1.0352,1.1218,1,1},
{0.8849,0.9183,0.8065,1.1386,1.0823,1.0286,1.0432,1.0579,1.0882,1.1193,1.0286,1.0286,0.8665,0.6138,1,0.8665,0.8665,1,1,1,1,1,1,1.0286,1.0286,1.0286,0.8665,0.9445,0.9715,1.0579,1.0286,1.0286,0.7297,1.0286,1.0984,1,1},
{0.9399,0.9541,0.8752,0.8768,1.6895,1.0188,1.0283,1.0379,1.0573,1.0772,1.0188,1.0188,0.925,0.7613,1,0.925,0.925,1,1,1,1,1,1,1.0188,1.0188,1.0188,0.925,0.972,0.9902,1.0379,1.0188,1.0188,0.8397,1.0188,1.0639,1,1},
{0.9989,0.9937,0.9631,0.8167,1.8437,1.0069,1.0104,1.0139,1.0209,1.028,1.0069,1.0069,0.9893,0.9534,1,0.9893,0.9893,1,1,1,1,1,1,1.0069,1.0069,1.0069,0.9893,1.0006,1.0075,1.0139,1.0069,1.0069,0.9717,1.0069,1.0224,1,1} 
 };

// Define unix time values for the start of each year.
//                                      2015       2016       2017       2018       2019       2020       2021       2022       2023       2024       2025       2026       2027       2028       2029       2030       2031       2032       2033       2034       2035       2036       2037       2038
const unsigned long startSecs[] = {1420070400,1451606400,1483228800,1514764800,1546300800,1577836800,1609459200,1640995200,1672531200,1704067200,1735689600,1767225600,1798761600,1830297600,1861920000,1893456000,1924992000,1956528000,1988150400,2019686400,2051222400,2082758400,2114380800,2145916800};

// 1st year of data in the Equilarg/Nodefactor/startSecs arrays.
const unsigned int startYear = 2015;
//------------------------------------------------------------------
// Define some variables that will hold extract values from the arrays above
float currAmp, currSpeed, currNodefactor, currEquilarg, currKappa, tideHeight;

// Constructor function, doesn't do anything special
TideCalc::TideCalc(void){}

// Return tide station name
char* TideCalc::returnStationID(void){
    return stationID;
}

// Return NOAA tide station ID number
long TideCalc::returnStationIDnumber(void){
    return stationIDnumber;
}

// currentTide calculation function, takes a DateTime object from real time clock
float TideCalc::currentTide(time_t now) {
	// Calculate difference between current year and starting year.
	YearIndx = Time.year(now) - startYear;
 	// Calculate hours since start of current year. Hours = seconds / 3600
	currHours = (now - startSecs[YearIndx]) / float(3600);
   // Shift currHours to Greenwich Mean Time
   currHours = currHours + adjustGMT;
   // *****************Calculate current tide height*************
   tideHeight = Datum; // initialize results variable, units of feet.
   for (int harms = 0; harms < 37; harms++) {
       // Step through each harmonic constituent, extract the relevant
       // values of Nodefactor, Amplitude, Equilibrium argument, Kappa
       // and Speed.
       currNodefactor = Nodefactor[YearIndx][harms];
 		currAmp = Amp[harms];
       currEquilarg = Equilarg[YearIndx][harms];
       currKappa = Kappa[harms];
       currSpeed = Speed[harms];
    // Calculate each component of the overall tide equation
    // The currHours value is assumed to be in hours from the start of the
    // year, in the Greenwich Mean Time zone, not the local time zone.
       tideHeight = tideHeight + (currNodefactor * currAmp *
           cos( (currSpeed * currHours + currEquilarg - currKappa) * DEG_TO_RAD));
    }
    //******************End of Tide Height calculation*************
    return tideHeight;  // Output of tideCalc is the tide height, units of feet
}
