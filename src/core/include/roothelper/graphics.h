#ifndef ROOTHELPER_GRAPHICS_H_
#define ROOTHELPER_GRAPHICS_H_

#include <TAxis.h>
#include <TCanvas.h>
#include <TCollection.h>
#include <TGaxis.h>
#include <TGraph.h>
#include <TGraph2D.h>
#include <TH1.h>
#include <TH2.h>
#include <TH2D.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TLine.h>
#include <TMultiGraph.h>
#include <TPad.h>
#include <TPaletteAxis.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace roothelper {

class IContainerWrapper;

/**
 * @struct GraphicsSize
 * @brief Layout tokens and configuration for padding, margins, and sizes in the pad/canvas.
 */
struct GraphicsSize {
  static GraphicsSize current;

  unsigned int pad_pixel_w;
  unsigned int pad_pixel_h;
  double text_size;
  double title_offset_x;
  double title_offset_y;
  double title_offset_step_horizontal;
  double top_margin;
  double right_margin;
  double bottom_margin;
  double left_margin;
  double top_margin_with_exponent;
  double right_margin_with_exponent;
  double right_margin_with_palette;
  double margin_step_horizontal;
  double margin_step_vertcical;
};

inline const GraphicsSize kGraphicsSize8pt = {
    700, 500, 0.05195, 1.1, 1.15, 0.225, 0.03, 0.005, 0.14, 0.14, 0.06, 0.07, 0.18, 0.019, 0.01,
};

inline const GraphicsSize kGraphicsSize10pt = {
    700, 500, 0.06494, 1.15, 1.20, 0.2, 0.01, 0.005, 0.155, 0.15, 0.06, 0.07, 0.20, 0.025, 0.01,
};

inline const GraphicsSize g_size_8pt = kGraphicsSize8pt;
inline const GraphicsSize g_size_10pt = kGraphicsSize10pt;

/**
 * @brief Initialize global style, font settings, and layout parameters for ROOT.
 */
void Prepare();

/**
 * @brief Helper to get default pad columns and rows based on total plot count.
 */
std::pair<unsigned int, unsigned int> GetDefaultNPad(unsigned int n_plot);

/**
 * @brief Create a TCanvas partitioned into sub-pads.
 *
 * @param name Canvas name.
 * @param title Canvas title.
 * @param n_pad_x Sub-pad count horizontally.
 * @param n_pad_y Sub-pad count vertically.
 * @param each_size_x Width of a single pad in pixels.
 * @param each_size_y Height of a single pad in pixels.
 * @return TCanvas* Pointer to the created TCanvas.
 */
TCanvas* CreateCanvas(const std::string& name, const std::string& title, unsigned int n_pad_x = 1,
                      unsigned int n_pad_y = 1,
                      unsigned int each_size_x = GraphicsSize::current.pad_pixel_w,
                      unsigned int each_size_y = GraphicsSize::current.pad_pixel_h);

/**
 * @brief Create a TCanvas with an automatically determined pad grid.
 */
TCanvas* CreateCanvasWithDefaultPadMatrix(
    const std::string& name, const std::string& title, unsigned int n_pad = 1,
    unsigned int each_size_x = GraphicsSize::current.pad_pixel_h,
    unsigned int each_size_y = GraphicsSize::current.pad_pixel_h);

/**
 * @brief Retrieve a color from the predefined cyclic color ring.
 */
Color_t GetColorInRing(unsigned int index);

/**
 * @brief Increase the active pad's top margin.
 */
double IncreaseTopMargin(double scale = 1.0);

/**
 * @brief Increase the active pad's right margin.
 */
double IncreaseRightMargin(double scale = 1.0);

/**
 * @brief Increase the active pad's left margin and adjust Y-axis title offset accordingly.
 */
template <class GraphType>
double IncreaseLeftMargin(GraphType* graph_object, double scale = 1.0) {
  const double current = gPad->GetLeftMargin();
  const double next = std::max(0.0, current + scale * GraphicsSize::current.margin_step_horizontal);

  gPad->SetLeftMargin(next);
  graph_object->GetYaxis()->SetTitleOffset(graph_object->GetYaxis()->GetTitleOffset() +
                                           scale *
                                               GraphicsSize::current.title_offset_step_horizontal);

  return next;
}

enum class LegendPosition { TopLeft, TopRight, BottomRight, BottomLeft };

/**
 * @brief Place a TLegend on the active pad.
 *
 * @param leg_pos Predefined corner position.
 * @param option Draw option.
 * @param width Width of the legend in NDC.
 * @param height Height of the legend in NDC.
 * @return TLegend* Pointer to the created TLegend.
 */
TLegend* PutLegend(LegendPosition leg_pos, Option_t* option = "", double width = 0.3,
                   double height = 0.2);

/**
 * @brief Style the X-axis of a ROOT object.
 */
template <class GraphType>
void SetXAxis(GraphType* graph_object, Option_t* draw_option = "") {
  std::string option = draw_option;
  if (option.empty()) {
    if constexpr (std::is_base_of_v<TObject, GraphType>) {
      if (graph_object->GetDrawOption() != nullptr) {
        option = graph_object->GetDrawOption();
      }
    }
  }

  bool has_z = false;
  for (char c : option) {
    if (c == 'z' || c == 'Z') {
      has_z = true;
      break;
    }
  }

  if (has_z) {
    gPad->SetRightMargin(GraphicsSize::current.right_margin_with_palette);
  } else {
    gPad->SetRightMargin(GraphicsSize::current.right_margin);
  }
  gPad->SetBottomMargin(GraphicsSize::current.bottom_margin);

  TAxis* axis = graph_object->GetXaxis();
  axis->SetTitleSize(GraphicsSize::current.text_size);
  axis->SetLabelSize(GraphicsSize::current.text_size);
  axis->SetTitleOffset(GraphicsSize::current.title_offset_x);
  axis->SetNdivisions(510);
  axis->SetDecimals(true);
  axis->CenterTitle();
}

/**
 * @brief Measure the maximum label width of an axis in NDC.
 *
 * Consistent with engineering-format scientific notation rules.
 */
double GetMaxLabelWidthNdc(TAxis* axis, bool is_y_axis = true, bool use_pad_limits = true,
                           double val_min_override = -1e300, double val_max_override = -1e300);

/**
 * @brief Dynamically optimize the left margin and Y-axis title offset for Y labels.
 */
void OptimizeYAxisLayout(TAxis* y_axis);
double GetYaxisLabelWidthNdc(IContainerWrapper* obj);

template <class GraphType>
void SetYAxis(GraphType* graph_object) {
  TAxis* axis = graph_object->GetYaxis();
  gPad->SetTopMargin(GraphicsSize::current.top_margin);

  axis->SetTitleSize(GraphicsSize::current.text_size);
  axis->SetLabelSize(GraphicsSize::current.text_size);
  axis->SetNdivisions(505);
  axis->SetDecimals(true);
  axis->CenterTitle();

  OptimizeYAxisLayout(axis);
}

template <class GraphType>
void SetZAxis(GraphType* graph_object) {
  TObject* obj = dynamic_cast<TObject*>(graph_object);
  if (obj == nullptr) return;

  TAxis* axis_for_attributes = nullptr;
  TAxis* axis_for_limits = nullptr;

  if (obj->InheritsFrom(TH1::Class())) {
    axis_for_attributes = dynamic_cast<TH1*>(obj)->GetZaxis();
    axis_for_limits = axis_for_attributes;
  } else if (obj->InheritsFrom(TGraph2D::Class())) {
    TGraph2D* g2d = dynamic_cast<TGraph2D*>(obj);
    axis_for_attributes = g2d->GetZaxis();
    if (g2d->GetHistogram() != nullptr) {
      axis_for_limits = g2d->GetHistogram()->GetZaxis();
    } else {
      axis_for_limits = g2d->GetZaxis();
    }
  }

  if (axis_for_attributes == nullptr || axis_for_limits == nullptr) return;

  double z_min = 0.0;
  double z_max = 0.0;
  if (obj->InheritsFrom(TH1::Class())) {
    TH1* h = dynamic_cast<TH1*>(obj);
    z_min = h->GetMinimum();
    z_max = h->GetMaximum();
  } else if (obj->InheritsFrom(TGraph2D::Class())) {
    TGraph2D* g2d = dynamic_cast<TGraph2D*>(obj);
    if (g2d->GetHistogram() != nullptr) {
      z_min = g2d->GetHistogram()->GetMinimum();
      z_max = g2d->GetHistogram()->GetMaximum();
    }
  }

  axis_for_attributes->SetTitleSize(GraphicsSize::current.text_size);
  axis_for_attributes->SetLabelSize(GraphicsSize::current.text_size);
  axis_for_attributes->SetTitleOffset(GraphicsSize::current.title_offset_x);
  axis_for_attributes->SetDecimals(true);
  axis_for_attributes->CenterTitle();

  if (axis_for_limits != axis_for_attributes) {
    axis_for_limits->SetTitleSize(GraphicsSize::current.text_size);
    axis_for_limits->SetLabelSize(GraphicsSize::current.text_size);
    axis_for_limits->SetTitleOffset(GraphicsSize::current.title_offset_x);
    axis_for_limits->SetDecimals(true);
    axis_for_limits->CenterTitle();
  }

  TPaletteAxis* palette = nullptr;
  if (obj->InheritsFrom(TH1::Class())) {
    TObject* pal_obj = dynamic_cast<TH1*>(obj)->GetListOfFunctions()->FindObject("palette");
    if (pal_obj != nullptr) {
      palette = static_cast<TPaletteAxis*>(pal_obj);
    }
  } else if (obj->InheritsFrom(TGraph2D::Class())) {
    TGraph2D* g2d = dynamic_cast<TGraph2D*>(obj);
    if (g2d->GetHistogram() != nullptr) {
      TObject* pal_obj = g2d->GetHistogram()->GetListOfFunctions()->FindObject("palette");
      if (pal_obj != nullptr) {
        palette = static_cast<TPaletteAxis*>(pal_obj);
      }
    }
  }

  if (palette != nullptr) {
    TGaxis* p_axis = palette->GetAxis();
    if (p_axis != nullptr) {
      p_axis->SetTitleSize(GraphicsSize::current.text_size);
      p_axis->SetLabelSize(GraphicsSize::current.text_size);

      const double label_width = GetMaxLabelWidthNdc(axis_for_limits, true, false, z_min, z_max);
      const double tick_length = p_axis->GetTickSize();
      const double title_size = p_axis->GetTitleSize();
      const double gap = 0.010;
      const double distance_from_axis = tick_length + label_width + gap;
      double title_offset = (distance_from_axis / title_size) * 0.60 + 0.20;
      if (title_offset < 0.3) {
        title_offset = 0.3;
      }

      axis_for_attributes->SetTitleOffset(title_offset);
      axis_for_limits->SetTitleOffset(title_offset);
      p_axis->SetTitleOffset(title_offset);
      palette->SetTitleOffset(title_offset);
      axis_for_attributes->CenterTitle();
      axis_for_limits->CenterTitle();
      p_axis->CenterTitle();

      // Check if Z-axis requires an exponent multiplier at the top of the axis
      bool has_exponent = false;
      if (gPad->GetLogz() == 0 && !axis_for_limits->GetNoExponent()) {
        const double max_val = std::max(std::abs(z_min), std::abs(z_max));
        const int max_digits = TGaxis::GetMaxDigits();
        if (max_val != 0.0 &&
            (max_val >= std::pow(10.0, max_digits) || max_val < std::pow(10.0, -max_digits))) {
          has_exponent = true;
        }
      }

      const double right_edge_buffer = 0.010;
      double required_margin = 0.045 + distance_from_axis + title_size + right_edge_buffer;

      if (gPad->GetRightMargin() < required_margin) {
        gPad->SetRightMargin(required_margin);
      }

      if (has_exponent) {
        const double required_top_margin = 0.065;
        if (gPad->GetTopMargin() < required_top_margin) {
          gPad->SetTopMargin(required_top_margin);
        }
      }
    }

    palette->SetX1NDC(1.0 - gPad->GetRightMargin() + 0.01);
    palette->SetX2NDC(1.0 - gPad->GetRightMargin() + 0.045);
    palette->SetY1NDC(gPad->GetBottomMargin());
    palette->SetY2NDC(1.0 - gPad->GetTopMargin());
  }
}

template <class GraphType>
void SetAxes(GraphType* graph_object, Option_t* draw_option = "") {
  gPad->Update();
  SetXAxis(graph_object, draw_option);
  SetYAxis(graph_object);
  SetZAxis(graph_object);
  gPad->Modified();
  gPad->Update();
}

TLine* DrawHorizontalLine(double y);
TLine* DrawVerticalLine(double x);

namespace publish {

template <class GraphType>
void SetMaxDigitX(GraphType* graph_object) {
  gPad->SetRightMargin(0.11);
  TAxis* axis = graph_object->GetXaxis();
  axis->SetMaxDigits(3);
}

template <class GraphType>
void SetMaxDigitY(GraphType* graph_object) {
  gPad->SetTopMargin(0.075);
  TAxis* axis = graph_object->GetYaxis();
  axis->SetMaxDigits(3);
}

template <class GraphType>
void SetTimeXAxis(GraphType* graph_object) {
  gPad->SetRightMargin(0.05);
  gPad->SetBottomMargin(0.19);

  TAxis* axis = graph_object->GetXaxis();
  axis->SetTitle("Datetime");
  axis->SetTimeFormat("#splitline{%H:%M}{%m/%d}");
  axis->SetTimeDisplay(1);
  axis->SetLabelSize(0.06);
  axis->SetLabelOffset(0.03);
  axis->SetTitleSize(0.06);
  axis->SetTitleOffset(1.6);
  axis->SetNdivisions(503);
  axis->CenterTitle(true);
}

template <class GraphType>
void SetYAxisFullWidth(GraphType* graph_object) {
  gPad->SetLeftMargin(0.185);
  TAxis* axis = graph_object->GetYaxis();
  axis->SetTitleOffset(1.4);
}

TMultiGraph* SetGraphColorsByRing(TMultiGraph* mg);
TMultiGraph* SetGraphMarkerStylesByRing(TMultiGraph* mg);

}  // namespace publish

TMultiGraph* SetMultigraphAxisFromMember(TMultiGraph* mg);

double FindX(const TGraph* g, double y, double x_start = 0.0, double x_end = 0.0);

template <class ObjectType>
std::vector<TCanvas*> DrawWithAutoRecreatorOfCanvas(const char* canvas_name_title, size_t n_pad_x,
                                                    size_t n_pad_y,
                                                    const std::vector<ObjectType*>& object_list,
                                                    const char* draw_option) {
  std::vector<TCanvas*> c_list;

  const size_t n_pad = n_pad_x * n_pad_y;
  size_t current_pad = n_pad + 1;
  for (size_t i = 0; i < object_list.size(); ++i) {
    TCanvas* c = nullptr;
    if (current_pad > n_pad) {
      c_list.push_back(new TCanvas(Form("%s_%zu", canvas_name_title, c_list.size()),
                                   Form("%s_%zu", canvas_name_title, c_list.size()), 700 * n_pad_x,
                                   500 * n_pad_y));
      c = c_list.back();
      c->Divide(n_pad_x, n_pad_y);
      current_pad = 1;
    } else {
      c = c_list.back();
    }
    c->cd(current_pad);
    object_list[i]->Draw(draw_option);
    ++current_pad;
  }

  return c_list;
}

template <class T, class ObjectType>
std::vector<TCanvas*> DrawWithAutoRecreatorOfCanvas(const char* canvas_name_title, size_t n_pad_x,
                                                    size_t n_pad_y,
                                                    const std::map<T, ObjectType*>& object_map,
                                                    const char* draw_option) {
  std::vector<ObjectType*> object_list;
  for (const auto& pair : object_map) {
    object_list.push_back(pair.second);
  }

  return DrawWithAutoRecreatorOfCanvas(canvas_name_title, n_pad_x, n_pad_y, object_list,
                                       draw_option);
}

template <class ConverterType>
TGraph* ConvertGraphYaxis(TGraph** g, ConverterType conversion_expr, const std::string& y_title) {
  for (size_t i = 0; i < (*g)->GetN(); ++i) {
    (*g)->GetY()[i] = conversion_expr((*g)->GetY()[i]);
  }
  (*g)->GetYaxis()->SetTitle(y_title.c_str());
  (*g)->SetBit(TGraph::kResetHisto);
  return *g;
}

TLatex* DrawLatexNdc(double x0, double y0, const std::string& content);

}  // namespace roothelper

#endif  // ROOTHELPER_GRAPHICS_H_
