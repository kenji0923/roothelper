#ifndef ROOT_HELPER_USED_IN_INTERPRETER
#include <ROOT_helper/analysis.h>
#endif

#include <TAxis.h>
#include <TGraphErrors.h>
#include <TH1.h>

#include <cmath>
#include <stdexcept>

namespace roothelper {

TH1* scale_histo_x(TH1* h, double scale) {
  const int n_bin = h->GetXaxis()->GetNbins();
  const double x_min = h->GetXaxis()->GetBinLowEdge(1);
  const double x_max = h->GetXaxis()->GetBinUpEdge(n_bin);

  h->SetBins(n_bin, x_min * scale, x_max * scale);
  return h;
}

TH1* convert_to_density_histo(TH1* h) {
  const int n_bin = h->GetXaxis()->GetNbins();
  const double integral = h->Integral();

  for (int i_bin = 1; i_bin <= n_bin; ++i_bin) {
    const double bin_width = h->GetXaxis()->GetBinWidth(i_bin);
    h->SetBinContent(i_bin, h->GetBinContent(i_bin) / bin_width / integral);
    h->SetBinError(i_bin, h->GetBinError(i_bin) / bin_width / integral);
  }

  return h;
}

TGraphErrors* get_graph_g0xa_plus_g1(double a, const TGraphErrors* g0, const TGraphErrors* g1) {
  const int n_data = g0->GetN();
  if (n_data != g1->GetN()) {
    throw std::runtime_error(
        "graphs with different numbers of points were selected for the calculation");
  }

  TGraphErrors* g_sum = new TGraphErrors(n_data);
  g_sum->GetXaxis()->SetTitle(g0->GetXaxis()->GetTitle());
  g_sum->GetYaxis()->SetTitle(g0->GetYaxis()->GetTitle());

  for (int i_data = 0; i_data < n_data; ++i_data) {
    const double x_0 = g0->GetX()[i_data];
    const double ex_0 = g0->GetEX()[i_data];
    const double y_0 = g0->GetY()[i_data];
    const double ey_0 = g0->GetEY()[i_data];

    const double x_1 = g1->GetX()[i_data];
    const double ex_1 = g1->GetEX()[i_data];
    const double y_1 = g1->GetY()[i_data];
    const double ey_1 = g1->GetEY()[i_data];

    double x = 0.0;
    double ex = 0.0;
    if (ex_0 == 0.0 && ex_1 == 0.0) {
      x = (x_0 + x_1) / 2.0;
    } else {
      const double w_sum = ex_0 + ex_1;
      const double w_0 = ex_1 / w_sum;
      const double w_1 = ex_0 / w_sum;
      x = x_0 * w_0 + x_1 * w_1;
      ex = std::sqrt(2.0) * ex_0 * ex_1 / w_sum;
    }

    const double y = a * y_0 + y_1;
    const double ey = std::sqrt(std::pow(a * ey_0, 2) + std::pow(ey_1, 2));

    g_sum->SetPoint(i_data, x, y);
    g_sum->SetPointError(i_data, ex, ey);
  }

  return g_sum;
}

}  // namespace roothelper
