// Checks automatic legend placement: corner choice, content fit, "SAME" objects and
// text reduction. Writes PNG files to the working directory for visual checks.

#include <TCanvas.h>
#include <TF1.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TLegend.h>
#include <TROOT.h>
#include <roothelper/graphics.h>

#include <cmath>
#include <cstdio>
#include <string>

namespace {

int g_failures = 0;

void Check(bool condition, const std::string& message) {
  if (!condition) {
    std::printf("FAIL: %s\n", message.c_str());
    ++g_failures;
  }
}

const char* CornerName(roothelper::LegendPosition p) {
  switch (p) {
    case roothelper::LegendPosition::TopLeft:
      return "TopLeft";
    case roothelper::LegendPosition::TopRight:
      return "TopRight";
    case roothelper::LegendPosition::BottomRight:
      return "BottomRight";
    case roothelper::LegendPosition::BottomLeft:
      return "BottomLeft";
  }
  return "";
}

// True if the legend overlaps any graph point or the segments between points.
bool OverlapsGraph(TLegend* leg, TGraph* g) {
  for (int i = 0; i + 1 < g->GetN(); ++i) {
    for (int k = 0; k <= 50; ++k) {
      const double t = k / 50.0;
      const double x = g->GetX()[i] + t * (g->GetX()[i + 1] - g->GetX()[i]);
      const double y = g->GetY()[i] + t * (g->GetY()[i + 1] - g->GetY()[i]);
      const double xn = (gPad->XtoPad(x) - gPad->GetX1()) / (gPad->GetX2() - gPad->GetX1());
      const double yn = (gPad->YtoPad(y) - gPad->GetY1()) / (gPad->GetY2() - gPad->GetY1());
      if (xn > leg->GetX1NDC() && xn < leg->GetX2NDC() && yn > leg->GetY1NDC() &&
          yn < leg->GetY2NDC()) {
        return true;
      }
    }
  }
  return false;
}

TGraph* MakeGraph(const char* name, double (*f)(double)) {
  TGraph* g = new TGraph();
  g->SetName(name);
  g->SetTitle(name);
  for (int i = 0; i <= 100; ++i) {
    const double x = i / 10.0;
    g->SetPoint(i, x, f(x));
  }
  g->SetLineWidth(1);
  return g;
}

void TestRisingCurve() {
  // A line from bottom-left to top-right leaves the other two corners free.
  TCanvas* c = roothelper::CreateCanvas("c_rising", "c_rising");
  TGraph* g = MakeGraph("Rising", [](double x) { return x; });
  g->Draw("AL");
  roothelper::SetAxes(g);
  TLegend* leg = roothelper::PutLegend();
  const roothelper::LegendPosition p =
      roothelper::FindMaxLegendBox(roothelper::kLegendMargin, leg).position;
  Check(p == roothelper::LegendPosition::TopLeft || p == roothelper::LegendPosition::BottomRight,
        std::string("rising: best corner is ") + CornerName(p));
  Check(!OverlapsGraph(leg, g), "rising: legend overlaps graph");
  Check(std::abs(leg->GetTextSize() - roothelper::GraphicsSize::current.text_size) < 1e-9,
        "rising: text size changed although content fits");
  Check(leg->GetX2NDC() - leg->GetX1NDC() < 0.5, "rising: legend was not shrunk to content");
  c->Print("legend_rising.png");
}

void TestSameObjects() {
  // The first graph leaves the top free; a graph drawn with SAME fills the top-left.
  TCanvas* c = roothelper::CreateCanvas("c_same", "c_same");
  TH1D* h = new TH1D("h_same", "Histogram;x;y", 50, 0.0, 10.0);
  for (int i = 1; i <= 50; ++i) h->SetBinContent(i, 1.0 + 0.2 * std::sin(i * 0.3));
  h->SetMaximum(10.0);
  h->SetMinimum(0.0);
  h->Draw("HIST");
  roothelper::SetAxes(h);
  TGraph* g = MakeGraph("Falling", [](double x) { return 9.5 - 0.9 * x; });
  g->Draw("L SAME");
  TLegend* leg = roothelper::PutLegend();
  const bool at_top_right = std::abs(leg->GetX2NDC() - (1.0 - gPad->GetRightMargin())) < 0.05 &&
                            std::abs(leg->GetY2NDC() - (1.0 - gPad->GetTopMargin())) < 0.05;
  Check(at_top_right, "same: legend is not at top right");
  Check(!OverlapsGraph(leg, g), "same: legend overlaps the SAME graph");
  c->Print("legend_same.png");
}

void TestShrinkText() {
  // A long legend in a crowded pad needs a smaller text.
  TCanvas* c = roothelper::CreateCanvas("c_shrink", "c_shrink");
  TGraph* g = MakeGraph("A very long legend label for shrinking text test",
                        [](double x) { return 5.0 + 4.0 * std::sin(x * 1.3); });
  g->Draw("AL");
  roothelper::SetAxes(g);
  TLegend* leg = roothelper::PutLegend();
  Check(leg->GetTextSize() < roothelper::GraphicsSize::current.text_size,
        "shrink: text was not reduced");
  Check(!OverlapsGraph(leg, g), "shrink: legend overlaps graph");
  c->Print("legend_shrink.png");
}

void TestFixedCorner() {
  TCanvas* c = roothelper::CreateCanvas("c_corner", "c_corner");
  TGraph* g = MakeGraph("Rising", [](double x) { return x; });
  g->Draw("AL");
  roothelper::SetAxes(g);
  TF1* f = new TF1("f_corner", "0.5*x+1", 0.0, 10.0);
  f->SetTitle("Function");
  f->Draw("SAME");
  TLegend* leg = roothelper::PutLegend(roothelper::LegendPosition::TopLeft);
  Check(leg->GetX1NDC() < gPad->GetLeftMargin() + 0.05, "corner: legend not at left");
  Check(leg->GetY2NDC() > 1.0 - gPad->GetTopMargin() - 0.05, "corner: legend not at top");
  Check(leg->GetNRows() == 2, "corner: function drawn with SAME is not in legend");
  c->Print("legend_corner.png");
}

}  // namespace

int main() {
  gROOT->SetBatch(true);
  roothelper::Prepare();
  TestRisingCurve();
  TestSameObjects();
  TestShrinkText();
  TestFixedCorner();
  if (g_failures == 0) std::printf("Legend placement checks passed.\n");
  return g_failures == 0 ? 0 : 1;
}
