#include <TCanvas.h>
#include <TGraph.h>
#include <TROOT.h>
#include <TStyle.h>
#include <roothelper/roothelper.h>

#include <cmath>
#include <iostream>

namespace {

bool CheckOffset(const TAxis* axis, double expected, const char* description) {
  if (std::abs(axis->GetLabelOffset() - expected) < 1.e-6) return true;
  std::cerr << description << ": expected " << expected << ", got "
            << axis->GetLabelOffset() << '\n';
  return false;
}

}  // namespace

int main() {
  gROOT->SetBatch(true);
  roothelper::Prepare();
  // Non-default style values verify that returning to linear axes uses gStyle.
  gStyle->SetLabelOffset(0.012, "X");
  gStyle->SetLabelOffset(0.018, "Y");
  TCanvas canvas("log_axis_labels", "Log axis labels", 800, 600);
  canvas.Divide(2, 1);
  double x[] = {0.001, 0.01, 0.1};
  double y[] = {1., 10., 100.};
  TGraph graph(3, x, y);
  TGraph other(3, x, y);
  canvas.cd(1);
  graph.Draw("ALP");
  // Test linear, log-x, log-log, log-y, then the transition back to linear.
  const int modes[][2] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}, {0, 0}};
  bool passed = true;
  for (const auto& mode : modes) {
    canvas.cd(1);
    gPad->SetLogx(mode[0]);
    gPad->SetLogy(mode[1]);
    gPad->Update();
    roothelper::SetAxes(&graph);
    passed &= CheckOffset(graph.GetXaxis(), mode[0] ? -0.010 : 0.012, "X offset");
    passed &= CheckOffset(graph.GetYaxis(), mode[1] ? -0.010 : 0.018, "Y offset");
    // Styling a different, linear pad must not inherit the first pad's log state.
    canvas.cd(2);
    other.Draw("ALP");
    roothelper::SetXAxis(&other);
    roothelper::SetYAxis(&other);
    passed &= CheckOffset(other.GetXaxis(), 0.012, "Other pad X offset");
    passed &= CheckOffset(other.GetYaxis(), 0.018, "Other pad Y offset");
  }
  if (!passed) return 1;
  std::cout << "Log-axis offsets, linear reset, and pad isolation passed.\n";
  return 0;
}
