#ifndef ROOTHELPER_ANALYSIS_H_
#define ROOTHELPER_ANALYSIS_H_

#include <TAxis.h>
#include <TGraphErrors.h>

namespace roothelper {

TH1* scaleHistoX(TH1* h, double scale);

TH1* convertToDensityHisto(TH1* h);

TGraphErrors* getGraphG0xAPlusG1(double a, const TGraphErrors* g0, const TGraphErrors* g1);

}  // namespace roothelper

#endif  // ROOTHELPER_ANALYSIS_H_
