#ifndef ROOT_HELPER_USED_IN_INTERPRETER
#include <ROOT_helper/graphics.h>
#endif

#include <TCanvas.h>
#include <TF1.h>
#include <TGaxis.h>
#include <TGraph.h>
#include <THLimitsFinder.h>
#include <THashList.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TLine.h>
#include <TList.h>
#include <TMultiGraph.h>
#include <TPad.h>
#include <TSpline.h>
#include <TString.h>
#include <TStyle.h>
#include <TText.h>

#include <algorithm>
#include <cmath>
#include <regex>
#include <utility>

namespace roothelper {
namespace {

TLatex g_latex;

template <class ObjType, class IterableType>
void set_colors_by_ring(IterableType* iterable, Color_t base_color = 1, Int_t inc_color = +1) {
  Color_t color = base_color;

  for (auto* obj : *iterable) {
    ObjType* obj_casted = dynamic_cast<ObjType*>(obj);
    if (obj_casted == nullptr) {
      continue;
    }

    obj_casted->SetLineColor(color);
    obj_casted->SetMarkerColor(color);
    color += inc_color;
  }
}

template <class ObjType, class IterableType>
void set_marker_styles_by_ring(IterableType* iterable, Style_t base_style = 20,
                               Int_t inc_style = +1) {
  Style_t style = base_style;

  for (auto* obj : *iterable) {
    ObjType* obj_casted = dynamic_cast<ObjType*>(obj);
    if (obj_casted == nullptr) {
      continue;
    }

    obj_casted->SetMarkerStyle(style);
    style += inc_style;
  }
}

}  // namespace

GraphicsSize GraphicsSize::current = kGraphicsSize8pt;

void prepare() {
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);
  gStyle->SetOptTitle(0);
  g_latex.SetTextSize(GraphicsSize::current.text_size * 0.8);
}

std::pair<unsigned int, unsigned int> get_default_n_pad(unsigned int n_plot) {
  if (n_plot <= 1) {
    return {1, 1};
  } else if (n_plot <= 2) {
    return {2, 1};
  } else if (n_plot <= 4) {
    return {2, 2};
  } else if (n_plot <= 6) {
    return {3, 2};
  } else if (n_plot <= 9) {
    return {3, 3};
  }

  return {4, 3};
}

TCanvas* create_canvas(const std::string& name, const std::string& title, unsigned int n_pad_x,
                       unsigned int n_pad_y, unsigned int each_size_x, unsigned int each_size_y) {
  const double w = n_pad_x * each_size_x;
  const double h = n_pad_y * each_size_y;

  TCanvas* c = new TCanvas(name.c_str(), title.c_str(), w, h);
  c->SetCanvasSize(w, h);

  if (n_pad_x > 1 || n_pad_y > 1) {
    c->Divide(n_pad_x, n_pad_y, 1e-6, 1e-6);
  }

  return c;
}

TCanvas* create_canvas_with_default_pad_matrix(const std::string& name, const std::string& title,
                                               unsigned int n_pad, unsigned int each_size_x,
                                               unsigned int each_size_y) {
  auto [n_pad_x, n_pad_y] = get_default_n_pad(n_pad);
  return create_canvas(name, title, n_pad_x, n_pad_y, each_size_x, each_size_y);
}

Color_t get_color_in_ring(unsigned int index) {
  switch (index) {
    case 0:
      return kAzure + 2;
    case 1:
      return kOrange + 10;
    case 2:
      return kTeal + 4;
    case 3:
      return kMagenta;
    case 4:
      return kCyan + 1;
    default:
      return kBlack;
  }
}

double increase_top_margin(double scale) {
  const double current = gPad->GetTopMargin();
  const double next = std::max(0.0, current + scale * GraphicsSize::current.margin_step_vertcical);
  gPad->SetTopMargin(next);
  return next;
}

double increase_right_margin(double scale) {
  const double current = gPad->GetRightMargin();
  const double next = std::max(0.0, current + scale * GraphicsSize::current.margin_step_horizontal);
  gPad->SetRightMargin(next);
  return next;
}

TLegend* put_legend(LegendPosition leg_pos, Option_t* option, double width, double height) {
  gPad->Update();

  const Double_t top_edge = 1.0 - gPad->GetTopMargin() - 0.02;
  const Double_t right_edge = 1.0 - gPad->GetRightMargin() - 0.02;
  const Double_t bottom_edge = gPad->GetBottomMargin() + 0.02;
  const Double_t left_edge = gPad->GetLeftMargin() + 0.02;

  TLegend* leg = nullptr;
  switch (leg_pos) {
    case LegendPosition::TopLeft:
      leg =
          gPad->BuildLegend(left_edge, top_edge, left_edge + width, top_edge - height, "", option);
      break;
    case LegendPosition::TopRight:
      leg = gPad->BuildLegend(right_edge - width, top_edge, right_edge, top_edge - height, "",
                              option);
      break;
    case LegendPosition::BottomRight:
      leg = gPad->BuildLegend(right_edge - width, bottom_edge + height, right_edge, bottom_edge, "",
                              option);
      break;
    case LegendPosition::BottomLeft:
      leg = gPad->BuildLegend(left_edge, bottom_edge + height, left_edge + width, bottom_edge, "",
                              option);
      break;
  }

  leg->SetBorderSize(0);
  leg->SetTextSize(GraphicsSize::current.text_size);
  return leg;
}

double GetMaxLabelWidthNDC(TAxis* axis) {
  if (axis == nullptr || gPad == nullptr) {
    return 0.0;
  }

  if (axis->GetLabels() != nullptr) {
    TText t;
    t.SetTextFont(axis->GetLabelFont());
    t.SetTextSize(axis->GetLabelSize());
    UInt_t max_w = 0;
    UInt_t w = 0;
    UInt_t h = 0;
    TIter next(axis->GetLabels()->MakeIterator());
    TObject* obj = nullptr;
    while ((obj = next()) != nullptr) {
      t.GetTextExtent(w, h, obj->GetName());
      if (w > max_w) {
        max_w = w;
      }
    }
    return static_cast<double>(max_w) / (gPad->GetWw() * gPad->GetWNDC());
  }

  const double x_min = axis->GetXmin();
  const double x_max = axis->GetXmax();
  const int n_div_code = axis->GetNdivisions();
  const int n_primary = std::abs(n_div_code) % 100;

  double bin_low = 0.0;
  double bin_high = 0.0;
  double bin_width = 0.0;
  int n_bins = 0;

  if (n_div_code > 0) {
    THLimitsFinder::Optimize(x_min, x_max, n_primary, bin_low, bin_high, n_bins, bin_width, "");
  } else {
    bin_low = x_min;
    bin_high = x_max;
    n_bins = n_primary;
    bin_width = (x_max - x_min) / static_cast<double>(n_bins);
  }

  TText t;
  t.SetTextFont(axis->GetLabelFont());
  t.SetTextSize(axis->GetLabelSize());

  UInt_t max_w = 0;
  UInt_t w = 0;
  UInt_t h = 0;
  const int max_digits = TGaxis::GetMaxDigits();

  for (int i = 0; i <= n_bins; ++i) {
    double val = bin_low + i * bin_width;
    if (std::abs(val) < 1e-10) {
      val = 0.0;
    }

    TString s;
    if (val != 0.0 && (std::abs(val) >= std::pow(10.0, max_digits) ||
                       std::abs(val) < std::pow(10.0, -max_digits))) {
      const int exp = static_cast<int>(std::floor(std::log10(std::abs(val))));
      const double base = val / std::pow(10.0, exp);
      s.Form("%g", base);
    } else {
      s.Form("%g", val);
    }

    t.GetTextExtent(w, h, s.Data());
    if (w > max_w) {
      max_w = w;
    }
  }

  return static_cast<double>(max_w) / (gPad->GetWw() * gPad->GetWNDC());
}

void OptimizeYAxisLayout(TAxis* y_axis) {
  if (y_axis == nullptr || gPad == nullptr) {
    return;
  }

  const double label_width = GetMaxLabelWidthNDC(y_axis);
  const double tick_length = y_axis->GetTickLength();
  const double title_size = y_axis->GetTitleSize();
  const double gap = 0.010;
  const double distance_from_axis = tick_length + label_width + gap;
  const double title_offset = (distance_from_axis / title_size) * 0.70;
  y_axis->SetTitleOffset(title_offset);

  const double left_edge_buffer = 0.0;
  const double required_margin = distance_from_axis + title_size + left_edge_buffer;
  if (gPad->GetLeftMargin() < required_margin) {
    gPad->SetLeftMargin(required_margin);
  }
}

double GetYaxisLabelWidthNDC(IContainerWrapper* /*obj*/) { return 0.0; }

TLine* draw_horizontal_line(double y) {
  gPad->Update();
  TLine* l = new TLine(gPad->GetUxmin(), y, gPad->GetUxmax(), y);
  l->Draw("SAME");
  return l;
}

TLine* draw_vertical_line(double x) {
  gPad->Update();
  TLine* l = new TLine(x, gPad->GetUymin(), x, gPad->GetUymax());
  l->Draw("SAME");
  return l;
}

namespace publish {

TMultiGraph* set_graph_colors_by_ring(TMultiGraph* mg) {
  set_colors_by_ring<TGraph>(mg->GetListOfGraphs());
  return mg;
}

TMultiGraph* set_graph_marker_styles_by_ring(TMultiGraph* mg) {
  set_marker_styles_by_ring<TGraph>(mg->GetListOfGraphs());
  return mg;
}

}  // namespace publish

TMultiGraph* set_multigraph_axis_from_member(TMultiGraph* mg) {
  TList* list = mg->GetListOfGraphs();
  for (auto* obj : *list) {
    TGraph* g = static_cast<TGraph*>(obj);

    std::string new_title;
    std::smatch m_title;
    const std::string orig_title = mg->GetTitle();
    if (std::regex_search(orig_title, m_title, std::regex("^(.+);$"))) {
      new_title = m_title.str(1);
    } else {
      new_title = orig_title;
    }
    const std::string new_title_x = g->GetXaxis()->GetTitle();
    const std::string new_title_y = g->GetYaxis()->GetTitle();

    mg->SetTitle(Form("%s;%s;%s", new_title.c_str(), new_title_x.c_str(), new_title_y.c_str()));
    break;
  }
  return mg;
}

double find_x(const TGraph* g, double y, double x_start, double x_end) {
  if (x_start >= x_end) {
    x_start = *std::min_element(g->GetX(), g->GetX() + g->GetN());
    x_end = *std::max_element(g->GetX(), g->GetX() + g->GetN());
  }

  TSpline* spl = new TSpline3("spl_g", g);
  TF1* f = new TF1(
      "f_spl_g", [spl](const double* x, const double* /*p*/) { return spl->Eval(x[0]); }, x_start,
      x_end, 0);

  double x = 0.0;
  if (y > f->GetMaximum(x_start, x_end)) {
    x = f->GetMaximumX(x_start, x_end);
  } else if (y < f->GetMinimum(x_start, x_end)) {
    x = f->GetMinimumX(x_start, x_end);
  } else {
    x = f->GetX(y, x_start, x_end);
  }

  delete f;
  delete spl;
  return x;
}

TLatex* draw_latex_ndc(double x0, double y0, const std::string& content) {
  return g_latex.DrawLatexNDC(x0, y0, content.c_str());
}

}  // namespace roothelper
