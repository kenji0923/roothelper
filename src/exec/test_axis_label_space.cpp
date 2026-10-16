// Checks that the last X label is kept inside the pad and that the first X label and the
// lowest Y label are separated at the bottom-left corner.

#include <TCanvas.h>
#include <TGraph.h>
#include <TROOT.h>
#include <TStyle.h>
#include <roothelper/graphics.h>

#include <cmath>
#include <cstdio>

int main() {
  gROOT->SetBatch(true);
  roothelper::Prepare();
  bool passed = true;

  // Range 0..10 puts labels on both ends of the X axis and "0" at the corner of both axes.
  TCanvas* c = roothelper::CreateCanvas("c_label_space", "c_label_space");
  TGraph* g = new TGraph();
  g->SetPoint(0, 0.0, 0.0);
  g->SetPoint(1, 10.0, 10.0);
  g->Draw("AL");
  g->GetXaxis()->SetLimits(0.0, 10.0);
  g->SetMinimum(0.0);
  g->SetMaximum(10.0);
  roothelper::SetAxes(g);

  if (!(gPad->GetRightMargin() > roothelper::GraphicsSize::current.right_margin)) {
    std::printf("FAIL: right margin was not widened (%g)\n", gPad->GetRightMargin());
    passed = false;
  }

  const double w_px = gPad->GetWw() * gPad->GetAbsWNDC();
  const double h_px = gPad->GetWh() * gPad->GetAbsHNDC();
  const double x_shift_px = (g->GetXaxis()->GetLabelOffset() - gStyle->GetLabelOffset("X")) * h_px;
  const double y_shift_px = (g->GetYaxis()->GetLabelOffset() - gStyle->GetLabelOffset("Y")) * w_px;
  if (!(x_shift_px > 0.0) || std::abs(x_shift_px - y_shift_px) > 1e-6) {
    std::printf("FAIL: label shifts differ or are zero (x %g px, y %g px)\n", x_shift_px,
                y_shift_px);
    passed = false;
  }
  if (!(gPad->GetBottomMargin() > roothelper::GraphicsSize::current.bottom_margin)) {
    std::printf("FAIL: bottom margin did not follow the X label shift\n");
    passed = false;
  }
  c->Print("axis_label_space.png");

  // Applying the styling again must give the same layout.
  const double right = gPad->GetRightMargin();
  const double x_offset = g->GetXaxis()->GetLabelOffset();
  roothelper::SetAxes(g);
  if (std::abs(gPad->GetRightMargin() - right) > 1e-9 ||
      std::abs(g->GetXaxis()->GetLabelOffset() - x_offset) > 1e-9) {
    std::printf("FAIL: styling twice changed the layout\n");
    passed = false;
  }

  // Short labels keep the divisions; five-character labels every 0.005 over 0.05 are reduced.
  if (g->GetXaxis()->GetNdivisions() % 100 != 10) {
    std::printf("FAIL: divisions of uncrowded labels changed (%d)\n",
                g->GetXaxis()->GetNdivisions());
    passed = false;
  }
  TCanvas* cd = roothelper::CreateCanvas("c_label_crowded", "c_label_crowded");
  TGraph* gd = new TGraph();
  gd->SetPoint(0, 0.709, 0.0);
  gd->SetPoint(1, 0.759, 5.0);
  gd->Draw("AL");
  gd->GetXaxis()->SetLimits(0.709, 0.759);
  roothelper::SetAxes(gd);
  const int crowded_div = gd->GetXaxis()->GetNdivisions();
  if (!(crowded_div % 100 < 10) || crowded_div / 100 != 5) {
    std::printf("FAIL: crowded X divisions %d (want fewer primary, secondary kept)\n",
                crowded_div);
    passed = false;
  }
  cd->Print("axis_label_crowded.png");

  // Exponent factors: X needs room right of the frame, Y above it.
  struct ExponentCase {
    const char* name;
    double x_max;
    double y_max;
    bool x_exponent;
    bool y_exponent;
  };
  const ExponentCase cases[] = {{"none", 10.0, 10.0, false, false},
                                {"x_large", 2e6, 10.0, true, false},
                                {"x_small", 3e-6, 10.0, true, false},
                                {"y_large", 10.0, 2e6, false, true},
                                {"y_small", 10.0, 3e-6, false, true}};
  for (const ExponentCase& e : cases) {
    TCanvas* ce = roothelper::CreateCanvas(e.name, e.name);
    TGraph* ge = new TGraph();
    ge->SetPoint(0, 0.0, 0.0);
    ge->SetPoint(1, e.x_max, e.y_max);
    ge->Draw("AL");
    ge->GetXaxis()->SetLimits(0.0, e.x_max);
    ge->SetMinimum(0.0);
    ge->SetMaximum(e.y_max);
    roothelper::SetAxes(ge);
    // "x10^{n}" needs more than the last-label clipping alone (about 0.024 here).
    const bool wide_right = gPad->GetRightMargin() > 0.06;
    const bool tall_top = gPad->GetTopMargin() > roothelper::GraphicsSize::current.top_margin;
    if (wide_right != e.x_exponent || tall_top != e.y_exponent) {
      std::printf("FAIL: %s: right margin %g, top margin %g\n", e.name, gPad->GetRightMargin(),
                  gPad->GetTopMargin());
      passed = false;
    }
    ce->Print(Form("axis_exponent_%s.png", e.name));
  }

  if (!passed) return 1;
  std::printf("Axis label space checks passed.\n");
  return 0;
}
