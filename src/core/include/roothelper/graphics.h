#ifndef ROOTHELPER_GRAPHICS_H_
#define ROOTHELPER_GRAPHICS_H_

#include <TAxis.h>
#include <TCanvas.h>
#include <TCollection.h>
#include <TGaxis.h>
#include <TGraph.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TLine.h>
#include <TMultiGraph.h>
#include <TPad.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace roothelper {

class IContainerWrapper;

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
  double margin_step_horizontal;
  double margin_step_vertcical;
};

inline const GraphicsSize kGraphicsSize8pt = {
    700, 500, 0.05195, 1.1, 1.15, 0.225, 0.03, 0.005, 0.14, 0.14, 0.06, 0.07, 0.019, 0.01,
};

inline const GraphicsSize kGraphicsSize10pt = {
    700, 500, 0.06494, 1.15, 1.20, 0.2, 0.01, 0.005, 0.155, 0.15, 0.06, 0.07, 0.025, 0.01,
};

inline const GraphicsSize g_size_8pt = kGraphicsSize8pt;
inline const GraphicsSize g_size_10pt = kGraphicsSize10pt;

void prepare();

std::pair<unsigned int, unsigned int> getDefaultNPad(unsigned int n_plot);

TCanvas* createCanvas(const std::string& name, const std::string& title, unsigned int n_pad_x = 1,
                       unsigned int n_pad_y = 1,
                       unsigned int each_size_x = GraphicsSize::current.pad_pixel_w,
                       unsigned int each_size_y = GraphicsSize::current.pad_pixel_h);

TCanvas* createCanvasWithDefaultPadMatrix(
    const std::string& name, const std::string& title, unsigned int n_pad = 1,
    unsigned int each_size_x = GraphicsSize::current.pad_pixel_h,
    unsigned int each_size_y = GraphicsSize::current.pad_pixel_h);

Color_t getColorInRing(unsigned int index);

double increaseTopMargin(double scale = 1.0);

double increaseRightMargin(double scale = 1.0);

template <class GraphType>
double increaseLeftMargin(GraphType* graph_object, double scale = 1.0) {
  const double current = gPad->GetLeftMargin();
  const double next = std::max(0.0, current + scale * GraphicsSize::current.margin_step_horizontal);

  gPad->SetLeftMargin(next);
  graph_object->GetYaxis()->SetTitleOffset(graph_object->GetYaxis()->GetTitleOffset() +
                                           scale *
                                               GraphicsSize::current.title_offset_step_horizontal);

  return next;
}

enum class LegendPosition { TopLeft, TopRight, BottomRight, BottomLeft };

TLegend* putLegend(LegendPosition leg_pos, Option_t* option = "", double width = 0.3,
                    double height = 0.2);

template <class GraphType>
void setXAxis(GraphType* graph_object) {
  gPad->SetRightMargin(GraphicsSize::current.right_margin);
  gPad->SetBottomMargin(GraphicsSize::current.bottom_margin);

  TAxis* axis = graph_object->GetXaxis();
  axis->SetTitleSize(GraphicsSize::current.text_size);
  axis->SetLabelSize(GraphicsSize::current.text_size);
  axis->SetTitleOffset(GraphicsSize::current.title_offset_x);
  axis->SetNdivisions(510);
  axis->SetDecimals(true);
  axis->CenterTitle();
}

double getMaxLabelWidthNdc(TAxis* axis);
void optimizeYAxisLayout(TAxis* y_axis);
double getYaxisLabelWidthNdc(IContainerWrapper* obj);

template <class GraphType>
void setYAxis(GraphType* graph_object) {
  TAxis* axis = graph_object->GetYaxis();
  gPad->SetTopMargin(GraphicsSize::current.top_margin);

  axis->SetTitleSize(GraphicsSize::current.text_size);
  axis->SetLabelSize(GraphicsSize::current.text_size);
  axis->SetNdivisions(505);
  axis->SetDecimals(true);
  axis->CenterTitle();

  optimizeYAxisLayout(axis);
}

template <class GraphType>
void setAxes(GraphType* graph_object) {
  gPad->Update();
  setXAxis(graph_object);
  setYAxis(graph_object);
  gPad->Modified();
  gPad->Update();
}

TLine* drawHorizontalLine(double y);
TLine* drawVerticalLine(double x);

namespace publish {

template <class GraphType>
void setMaxDigitX(GraphType* graph_object) {
  gPad->SetRightMargin(0.11);
  TAxis* axis = graph_object->GetXaxis();
  axis->SetMaxDigits(3);
}

template <class GraphType>
void setMaxDigitY(GraphType* graph_object) {
  gPad->SetTopMargin(0.075);
  TAxis* axis = graph_object->GetYaxis();
  axis->SetMaxDigits(3);
}

template <class GraphType>
void setTimeXAxis(GraphType* graph_object) {
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
void setYAxisFullWidth(GraphType* graph_object) {
  gPad->SetLeftMargin(0.185);
  TAxis* axis = graph_object->GetYaxis();
  axis->SetTitleOffset(1.4);
}

TMultiGraph* setGraphColorsByRing(TMultiGraph* mg);
TMultiGraph* setGraphMarkerStylesByRing(TMultiGraph* mg);

}  // namespace publish

TMultiGraph* setMultigraphAxisFromMember(TMultiGraph* mg);

double findX(const TGraph* g, double y, double x_start = 0.0, double x_end = 0.0);

template <class ObjectType>
std::vector<TCanvas*> drawWithAutoRecreatorOfCanvas(
    const char* canvas_name_title, size_t n_pad_x, size_t n_pad_y,
    const std::vector<ObjectType*>& object_list, const char* draw_option) {
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
std::vector<TCanvas*> drawWithAutoRecreatorOfCanvas(const char* canvas_name_title,
                                                         size_t n_pad_x, size_t n_pad_y,
                                                         const std::map<T, ObjectType*>& object_map,
                                                         const char* draw_option) {
  std::vector<ObjectType*> object_list;
  for (const auto& pair : object_map) {
    object_list.push_back(pair.second);
  }

  return drawWithAutoRecreatorOfCanvas(canvas_name_title, n_pad_x, n_pad_y, object_list,
                                            draw_option);
}

template <class ConverterType>
TGraph* convertGraphYaxis(TGraph** g, ConverterType conversion_expr, const std::string& y_title) {
  for (size_t i = 0; i < (*g)->GetN(); ++i) {
    (*g)->GetY()[i] = conversion_expr((*g)->GetY()[i]);
  }
  (*g)->GetYaxis()->SetTitle(y_title.c_str());
  (*g)->SetBit(TGraph::kResetHisto);
  return *g;
}

TLatex* drawLatexNdc(double x0, double y0, const std::string& content);

}  // namespace roothelper

#endif  // ROOTHELPER_GRAPHICS_H_
