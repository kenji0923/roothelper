#ifndef ROOTHELPER_ANALYSIS_H_
#define ROOTHELPER_ANALYSIS_H_

#include <TAxis.h>
#include <TGraphErrors.h>

namespace roothelper {

TH1* ScaleHistoX(TH1* h, double scale);

TH1* ConvertToDensityHisto(TH1* h);

TGraphErrors* GetGraphG0xAPlusG1(double a, const TGraphErrors* g0, const TGraphErrors* g1);

}  // namespace roothelper

#endif  // ROOTHELPER_ANALYSIS_H_
