#ifndef ROOTHELPER_ANALYSIS_H_
#define ROOTHELPER_ANALYSIS_H_

#include <TAxis.h>
#include <TGraphErrors.h>

namespace roothelper {

TH1* scale_histo_x(TH1* h, double scale);

TH1* convert_to_density_histo(TH1* h);

TGraphErrors* get_graph_g0xa_plus_g1(double a, const TGraphErrors* g0, const TGraphErrors* g1);

}  // namespace roothelper

#endif  // ROOTHELPER_ANALYSIS_H_
