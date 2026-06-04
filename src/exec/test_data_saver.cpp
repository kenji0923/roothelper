#include <TCanvas.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1F.h>
#include <THStack.h>
#include <TMultiGraph.h>
#include <roothelper/data_saver.h>
#include <roothelper/roothelper.h>

#include <cassert>
#include <iostream>

namespace rh = roothelper;

int main(int argc, char** argv) {
  rh::Prepare();

  std::filesystem::path test_dir = "DataSaverTest";
  // Ensure test_dir is clean
  std::filesystem::remove_all(test_dir);

  {
    rh::DataSaver data_saver(test_dir, true);

    TCanvas* c = new TCanvas("c_test_canvas", "Test Canvas", 800, 600);
    c->Divide(2, 1);

    // Sub-pad 1: TMultiGraph
    c->cd(1);
    TMultiGraph* mg = new TMultiGraph("mg_test", "MultiGraph Test");
    TGraph* g1 = new TGraph();
    g1->SetName("g_1");
    g1->SetPoint(0, 0, 0);
    g1->SetPoint(1, 1, 1);
    TGraph* g2 = new TGraph();
    g2->SetName("g_2");
    g2->SetPoint(0, 0, 1);
    g2->SetPoint(1, 1, 0);
    mg->Add(g1);
    mg->Add(g2);
    mg->Draw("A");

    // Sub-pad 2: THStack
    c->cd(2);
    THStack* hs = new THStack("hs_test", "Stack Test");
    TH1F* h1 = new TH1F("h_1", "Hist 1", 10, 0, 10);
    h1->Fill(5);
    TH1F* h2 = new TH1F("h_2", "Hist 2", 10, 0, 10);
    h2->Fill(3);
    hs->Add(h1);
    hs->Add(h2);
    hs->Draw();

    // Write the canvas (this should save it and its primitives recursively)
    data_saver.WriteCanvas(c, "plots");
  }

  // Now verify the root file contents
  TFile* f = TFile::Open((test_dir / "data.root").c_str(), "READ");
  if (!f || f->IsZombie()) {
    std::cerr << "Error: failed to open ROOT file." << std::endl;
    return 1;
  }

  // Assert TCanvas is written
  TCanvas* c_read = nullptr;
  f->GetObject("plots/c_test_canvas", c_read);
  if (!c_read) {
    std::cerr << "Assertion failed: TCanvas 'c_test_canvas' not found in plots/" << std::endl;
    return 1;
  }
  std::cout << "Successfully verified TCanvas in 'plots/'" << std::endl;

  // Assert TMultiGraph is written inside data_c_test_canvas
  TMultiGraph* mg_read = nullptr;
  f->GetObject("plots/data_c_test_canvas/mg_test", mg_read);
  if (!mg_read) {
    std::cerr << "Assertion failed: TMultiGraph 'mg_test' not found in plots/data_c_test_canvas/"
              << std::endl;
    return 1;
  }
  std::cout << "Successfully verified TMultiGraph in 'plots/data_c_test_canvas/'" << std::endl;

  // Assert TGraph children are written inside plots/data_c_test_canvas/data_mg_test
  TGraph* g1_read = nullptr;
  f->GetObject("plots/data_c_test_canvas/data_mg_test/g_1", g1_read);
  if (!g1_read) {
    std::cerr
        << "Assertion failed: TGraph 'g_1' not found in plots/data_c_test_canvas/data_mg_test/"
        << std::endl;
    return 1;
  }
  TGraph* g2_read = nullptr;
  f->GetObject("plots/data_c_test_canvas/data_mg_test/g_2", g2_read);
  if (!g2_read) {
    std::cerr
        << "Assertion failed: TGraph 'g_2' not found in plots/data_c_test_canvas/data_mg_test/"
        << std::endl;
    return 1;
  }
  std::cout << "Successfully verified TGraph children in 'plots/data_c_test_canvas/data_mg_test/'"
            << std::endl;

  // Assert THStack is written inside data_c_test_canvas
  THStack* hs_read = nullptr;
  f->GetObject("plots/data_c_test_canvas/hs_test", hs_read);
  if (!hs_read) {
    std::cerr << "Assertion failed: THStack 'hs_test' not found in plots/data_c_test_canvas/"
              << std::endl;
    return 1;
  }
  std::cout << "Successfully verified THStack in 'plots/data_c_test_canvas/'" << std::endl;

  // Assert TH1F children are written inside plots/data_c_test_canvas/data_hs_test
  TH1* h1_read = nullptr;
  f->GetObject("plots/data_c_test_canvas/data_hs_test/h_1", h1_read);
  if (!h1_read) {
    std::cerr << "Assertion failed: TH1 'h_1' not found in plots/data_c_test_canvas/data_hs_test/"
              << std::endl;
    return 1;
  }
  TH1* h2_read = nullptr;
  f->GetObject("plots/data_c_test_canvas/data_hs_test/h_2", h2_read);
  if (!h2_read) {
    std::cerr << "Assertion failed: TH1 'h_2' not found in plots/data_c_test_canvas/data_hs_test/"
              << std::endl;
    return 1;
  }
  std::cout << "Successfully verified TH1 children in 'plots/data_c_test_canvas/data_hs_test/'"
            << std::endl;

  f->Close();
  std::cout << "All assertions passed successfully!" << std::endl;
  return 0;
}
