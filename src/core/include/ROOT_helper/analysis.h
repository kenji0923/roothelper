#ifndef ROOT_HELPER_ANALYSIS_H_
#define ROOT_HELPER_ANALYSIS_H_

#include <TAxis.h>
#include <TGraphErrors.h>

namespace ROOT_helper {

TH1* scale_histo_x(TH1* h, double scale);

TH1* convert_to_density_histo(TH1* h);

TGraphErrors* get_graph_g0xa_plus_g1(double a, const TGraphErrors* g0, const TGraphErrors* g1);

}  // namespace ROOT_helper

#endif  // ROOT_HELPER_ANALYSIS_H_
