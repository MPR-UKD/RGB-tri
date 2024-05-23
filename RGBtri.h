// RGBtri.h

#pragma once

#include <iostream>
// header for Nifti handling
#include "nifti1_io.h"
// standard includes
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>

// helper functions
void displayUsage(const std::string& programName);
void displayVersion();
void equalizeHistogram(int* pdata, int width, int height, int noslice, int max_val, float scalefact, float fCenter);
void WhiteBalanceBand(int* bandArray, int length);
double Percentile(double array[], int length, double percentile);
double LimitToByte(double value);