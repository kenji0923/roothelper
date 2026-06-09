#ifndef ROOTHELPER_USED_IN_INTERPRETER
#include <roothelper/graphics.h>
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

void Prepare() {
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);
  gStyle->SetOptTitle(0);
  g_latex.SetTextSize(GraphicsSize::current.text_size * 0.8);
}

std::pair<unsigned int, unsigned int> GetDefaultNPad(unsigned int n_plot) {
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

TCanvas* CreateCanvas(const std::string& name, const std::string& title, unsigned int n_pad_x,
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

TCanvas* CreateCanvasWithDefaultPadMatrix(const std::string& name, const std::string& title,
                                               unsigned int n_pad, unsigned int each_size_x,
                                               unsigned int each_size_y) {
  auto [n_pad_x, n_pad_y] = GetDefaultNPad(n_pad);
  return CreateCanvas(name, title, n_pad_x, n_pad_y, each_size_x, each_size_y);
}

Color_t GetColorInRing(unsigned int index) {
  switch (index) {
    case 0:
      return kP10Blue;
    case 1:
      return kP10Yellow;
    case 2:
      return kP10Red;
    case 3:
      return kP10Gray;
    case 4:
      return kP10Violet;
    case 5:
      return kP10Brown;
    case 6:
      return kP10Orange;
    case 7:
      return kP10Green;
    case 8:
      return kP10Ash;
    case 9:
      return kP10Cyan;
    default:
      return 1 + index;
  }
}

double IncreaseTopMargin(double scale) {
  const double current = gPad->GetTopMargin();
  const double next = std::max(0.0, current + scale * GraphicsSize::current.margin_step_vertcical);
  gPad->SetTopMargin(next);
  return next;
}

double IncreaseRightMargin(double scale) {
  const double current = gPad->GetRightMargin();
  const double next = std::max(0.0, current + scale * GraphicsSize::current.margin_step_horizontal);
  gPad->SetRightMargin(next);
  return next;
}

TLegend* PutLegend(LegendPosition leg_pos, Option_t* option, double width, double height) {
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

double GetMaxLabelWidthNdc(TAxis* axis, bool is_y_axis, bool use_pad_limits,
                           double val_min_override, double val_max_override) {
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

  double x_min = (val_min_override > -1e299) ? val_min_override : axis->GetXmin();
  double x_max = (val_max_override > -1e299) ? val_max_override : axis->GetXmax();
  if (!use_pad_limits) {
    if (gPad->GetLogz() != 0) {
      x_min = std::pow(10.0, x_min);
      x_max = std::pow(10.0, x_max);
    }
  } else if (is_y_axis) {
    x_min = gPad->GetUymin();
    x_max = gPad->GetUymax();
    if (gPad->GetLogy() != 0) {
      x_min = std::pow(10.0, x_min);
      x_max = std::pow(10.0, x_max);
    }
  } else {
    x_min = gPad->GetUxmin();
    x_max = gPad->GetUxmax();
    if (gPad->GetLogx() != 0) {
      x_min = std::pow(10.0, x_min);
      x_max = std::pow(10.0, x_max);
    }
  }
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

  // Find the single global exponent N for the entire axis
  int global_exp = 0;
  if (!axis->GetNoExponent()) {
    const double max_val = std::max(std::abs(bin_low), std::abs(bin_high));
    if (max_val != 0.0) {
      if (max_val >= std::pow(10.0, max_digits)) {
        const int int_digits = static_cast<int>(std::floor(std::log10(max_val))) + 1;
        const int diff = int_digits - max_digits;
        if (diff > 0) {
          global_exp = ((diff + 2) / 3) * 3;
        }
      } else if (max_val < std::pow(10.0, -max_digits)) {
        const int neg_digits = static_cast<int>(std::floor(std::log10(max_val)));
        int n = neg_digits;
        if (n % 3 != 0) {
          n = (n / 3) * 3 - 3;
        }
        global_exp = n;
      }
    }
  }

  // Calculate the number of decimals based on the scaled bin width
  double scaled_bin_width = bin_width;
  if (global_exp != 0) {
    scaled_bin_width /= std::pow(10.0, global_exp);
  }

  int decimals = 0;
  if (scaled_bin_width < 1.0) {
    decimals = static_cast<int>(std::ceil(-std::log10(scaled_bin_width) + 1e-9));
  }

  for (int i = 0; i <= n_bins; ++i) {
    double val = bin_low + i * bin_width;
    if (std::abs(val) < 1e-10) {
      val = 0.0;
    }

    TString s;
    double scaled_val = val;
    if (global_exp != 0) {
      scaled_val /= std::pow(10.0, global_exp);
    }
    s.Form("%.*f", decimals, scaled_val);

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

  const double label_width = GetMaxLabelWidthNdc(y_axis);
  const double tick_length = y_axis->GetTickLength();
  const double title_size = y_axis->GetTitleSize();
  const double gap = 0.010;
  const double distance_from_axis = tick_length + label_width + gap;
  double title_offset = (distance_from_axis / title_size) * 0.60 + 0.20;
  if (title_offset < 0.3) {
    title_offset = 0.3;
  }
  y_axis->SetTitleOffset(title_offset);


  const double left_edge_buffer = 0.0;
  const double required_margin = distance_from_axis + title_size + left_edge_buffer;
  if (gPad->GetLeftMargin() < required_margin) {
    gPad->SetLeftMargin(required_margin);
  }

  // Check if Y-axis requires an exponent multiplier at the top of the axis
  bool has_exponent = false;
  if (gPad->GetLogy() == 0 && !y_axis->GetNoExponent()) {
    const double y_min = gPad->GetUymin();
    const double y_max = gPad->GetUymax();
    const double max_val = std::max(std::abs(y_min), std::abs(y_max));
    const int max_digits = TGaxis::GetMaxDigits();
    if (max_val != 0.0 && (max_val >= std::pow(10.0, max_digits) || max_val < std::pow(10.0, -max_digits))) {
      has_exponent = true;
    }
  }

  if (has_exponent) {
    const double required_top_margin = 0.085;
    if (gPad->GetTopMargin() < required_top_margin) {
      gPad->SetTopMargin(required_top_margin);
    }
  }
}

double GetYaxisLabelWidthNdc(IContainerWrapper* /*obj*/) { return 0.0; }

TLine* DrawHorizontalLine(double y) {
  gPad->Update();
  TLine* l = new TLine(gPad->GetUxmin(), y, gPad->GetUxmax(), y);
  l->Draw("SAME");
  return l;
}

TLine* DrawVerticalLine(double x) {
  gPad->Update();
  TLine* l = new TLine(x, gPad->GetUymin(), x, gPad->GetUymax());
  l->Draw("SAME");
  return l;
}

namespace publish {

TMultiGraph* SetGraphColorsByRing(TMultiGraph* mg) {
  set_colors_by_ring<TGraph>(mg->GetListOfGraphs());
  return mg;
}

TMultiGraph* SetGraphMarkerStylesByRing(TMultiGraph* mg) {
  set_marker_styles_by_ring<TGraph>(mg->GetListOfGraphs());
  return mg;
}

}  // namespace publish

TMultiGraph* SetMultigraphAxisFromMember(TMultiGraph* mg) {
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

double FindX(const TGraph* g, double y, double x_start, double x_end) {
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

TLatex* DrawLatexNdc(double x0, double y0, const std::string& content) {
  return g_latex.DrawLatexNDC(x0, y0, content.c_str());
}

}  // namespace roothelper
