#ifndef ROOTHELPER_USED_IN_INTERPRETER
#include <roothelper/graphics.h>
#endif

#include <TBox.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TFrame.h>
#include <TGaxis.h>
#include <TGraph.h>
#include <TH2.h>
#include <THLimitsFinder.h>
#include <THStack.h>
#include <THashList.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TLegendEntry.h>
#include <TLine.h>
#include <TList.h>
#include <TMarker.h>
#include <TMath.h>
#include <TMultiGraph.h>
#include <TPad.h>
#include <TPave.h>
#include <TSpline.h>
#include <TString.h>
#include <TStyle.h>
#include <TText.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iterator>
#include <regex>
#include <string>
#include <utility>
#include <vector>

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
  g_latex.SetTextFont(42);                               // normal Helvetica (not bold, font 62)
  g_latex.SetTextSize(GraphicsSize::current.text_size);  // match axis-title size
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

namespace {

// Pixel size of the active pad.
double PadWidthPixel() { return gPad->GetWw() * gPad->GetAbsWNDC(); }
double PadHeightPixel() { return gPad->GetWh() * gPad->GetAbsHNDC(); }

// Text height in pixels for a size given as a pad fraction (precision-2 fonts).
double TextHeightPixel(double text_size) {
  return text_size * std::min(PadWidthPixel(), PadHeightPixel());
}

bool UserToNdc(double x, double y, double& x_ndc, double& y_ndc) {
  if ((gPad->GetLogx() != 0 && x <= 0.0) || (gPad->GetLogy() != 0 && y <= 0.0)) {
    return false;
  }
  if (!std::isfinite(x) || !std::isfinite(y)) {
    return false;
  }
  x_ndc = (gPad->XtoPad(x) - gPad->GetX1()) / (gPad->GetX2() - gPad->GetX1());
  y_ndc = (gPad->YtoPad(y) - gPad->GetY1()) / (gPad->GetY2() - gPad->GetY1());
  return true;
}

bool HasAny(const std::string& option, const char* chars) {
  return option.find_first_of(chars) != std::string::npos;
}

std::string ToUpper(const char* option) {
  std::string s = option != nullptr ? option : "";
  for (char& c : s) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }
  return s;
}

// Occupancy of the frame of the active pad, rasterized in cells of about two pixels.
class OccupancyGrid {
 public:
  OccupancyGrid() {
    x1_ = gPad->GetLeftMargin();
    x2_ = 1.0 - gPad->GetRightMargin();
    y1_ = gPad->GetBottomMargin();
    y2_ = 1.0 - gPad->GetTopMargin();
    nx_ = std::clamp(static_cast<int>((x2_ - x1_) * PadWidthPixel() / 2.0), 20, 600);
    ny_ = std::clamp(static_cast<int>((y2_ - y1_) * PadHeightPixel() / 2.0), 20, 600);
    cells_.assign(static_cast<size_t>(nx_) * ny_, 0);
  }

  int nx() const { return nx_; }
  int ny() const { return ny_; }
  double CellWidth() const { return (x2_ - x1_) / nx_; }
  double CellHeight() const { return (y2_ - y1_) / ny_; }
  double x1() const { return x1_; }
  double y1() const { return y1_; }
  double x2() const { return x2_; }
  double y2() const { return y2_; }

  bool At(int ix, int iy) const { return cells_[static_cast<size_t>(iy) * nx_ + ix] != 0; }

  void MarkRect(double xa, double ya, double xb, double yb) {
    if (xa > xb) std::swap(xa, xb);
    if (ya > yb) std::swap(ya, yb);
    const int ixa = std::max(0, static_cast<int>(std::floor((xa - x1_) / CellWidth())));
    const int ixb = std::min(nx_ - 1, static_cast<int>(std::floor((xb - x1_) / CellWidth())));
    const int iya = std::max(0, static_cast<int>(std::floor((ya - y1_) / CellHeight())));
    const int iyb = std::min(ny_ - 1, static_cast<int>(std::floor((yb - y1_) / CellHeight())));
    for (int iy = iya; iy <= iyb; ++iy) {
      for (int ix = ixa; ix <= ixb; ++ix) {
        cells_[static_cast<size_t>(iy) * nx_ + ix] = 1;
      }
    }
  }

  void MarkPoint(double x, double y, double rx = 0.0, double ry = 0.0) {
    MarkRect(x - rx, y - ry, x + rx, y + ry);
  }

  void MarkSegment(double xa, double ya, double xb, double yb) {
    const double steps =
        std::max(std::abs(xb - xa) / CellWidth(), std::abs(yb - ya) / CellHeight());
    if (!std::isfinite(steps)) return;
    // Long segments far outside the frame are still sampled finely enough inside it.
    const int n = std::min(200000, static_cast<int>(std::ceil(2.0 * steps)) + 1);
    for (int i = 0; i <= n; ++i) {
      const double t = static_cast<double>(i) / n;
      MarkPoint(xa + t * (xb - xa), ya + t * (yb - ya));
    }
  }

  void MarkUserRect(double xa, double ya, double xb, double yb) {
    double nxa = 0.0, nya = 0.0, nxb = 0.0, nyb = 0.0;
    if (UserToNdc(xa, ya, nxa, nya) && UserToNdc(xb, yb, nxb, nyb)) {
      MarkRect(nxa, nya, nxb, nyb);
    }
  }

  void MarkUserSegment(double xa, double ya, double xb, double yb) {
    double nxa = 0.0, nya = 0.0, nxb = 0.0, nyb = 0.0;
    if (UserToNdc(xa, ya, nxa, nya) && UserToNdc(xb, yb, nxb, nyb)) {
      MarkSegment(nxa, nya, nxb, nyb);
    }
  }

  // Grow occupied cells by (mx, my) cells in each direction.
  void Dilate(int mx, int my) {
    std::vector<char> tmp(cells_.size(), 0);
    for (int iy = 0; iy < ny_; ++iy) {
      int last = -1000000;
      for (int ix = 0; ix < nx_; ++ix) {
        if (At(ix, iy)) last = ix;
        if (ix - last <= mx) tmp[static_cast<size_t>(iy) * nx_ + ix] = 1;
      }
      last = 1000000;
      for (int ix = nx_ - 1; ix >= 0; --ix) {
        if (At(ix, iy)) last = ix;
        if (last - ix <= mx) tmp[static_cast<size_t>(iy) * nx_ + ix] = 1;
      }
    }
    cells_.assign(cells_.size(), 0);
    for (int ix = 0; ix < nx_; ++ix) {
      int last = -1000000;
      for (int iy = 0; iy < ny_; ++iy) {
        if (tmp[static_cast<size_t>(iy) * nx_ + ix]) last = iy;
        if (iy - last <= my) cells_[static_cast<size_t>(iy) * nx_ + ix] = 1;
      }
      last = 1000000;
      for (int iy = ny_ - 1; iy >= 0; --iy) {
        if (tmp[static_cast<size_t>(iy) * nx_ + ix]) last = iy;
        if (last - iy <= my) cells_[static_cast<size_t>(iy) * nx_ + ix] = 1;
      }
    }
  }

 private:
  double x1_, x2_, y1_, y2_;
  int nx_, ny_;
  std::vector<char> cells_;
};

void MarkObject(OccupancyGrid& grid, TObject* obj, const std::string& option,
                const TObject* ignore);

void MarkFunctions(OccupancyGrid& grid, TList* functions, const TObject* ignore) {
  if (functions == nullptr) return;
  TIter next(functions);
  while (TObject* obj = next()) {
    if (obj->InheritsFrom(TF1::Class()) && obj->TestBit(TF1::kNotDraw)) continue;
    MarkObject(grid, obj, ToUpper(next.GetOption()), ignore);
  }
}

void MarkGraph(OccupancyGrid& grid, TGraph* g, const std::string& option) {
  const int n = g->GetN();
  const double* x = g->GetX();
  const double* y = g->GetY();
  const bool has_marker = HasAny(option, "P*");
  const bool has_bar = HasAny(option, "B");
  const bool has_line = HasAny(option, "LCF") || (!has_marker && !has_bar);
  const bool has_error = option.find('X') == std::string::npos;

  const double marker_px =
      (g->GetMarkerStyle() == 1 || g->GetMarkerSize() <= 0.0) ? 1.0 : 4.0 * g->GetMarkerSize();
  const double rx = marker_px / PadWidthPixel();
  const double ry = marker_px / PadHeightPixel();

  double prev_x = 0.0, prev_y = 0.0;
  bool has_prev = false;
  for (int i = 0; i < n; ++i) {
    double xn = 0.0, yn = 0.0;
    const bool ok = UserToNdc(x[i], y[i], xn, yn);
    if (ok) {
      grid.MarkPoint(xn, yn, has_marker ? rx : 0.0, has_marker ? ry : 0.0);
      if (has_line && has_prev) grid.MarkSegment(prev_x, prev_y, xn, yn);
      prev_x = xn;
      prev_y = yn;
    }
    has_prev = ok;

    if (has_bar) {
      const double base = gPad->GetLogy() != 0 ? std::pow(10.0, gPad->GetUymin()) : 0.0;
      grid.MarkUserSegment(x[i], base, x[i], y[i]);
    }
    if (has_error) {
      const double exl = g->GetErrorXlow(i), exh = g->GetErrorXhigh(i);
      const double eyl = g->GetErrorYlow(i), eyh = g->GetErrorYhigh(i);
      if (eyl > 0.0 || eyh > 0.0) grid.MarkUserSegment(x[i], y[i] - eyl, x[i], y[i] + eyh);
      if (exl > 0.0 || exh > 0.0) grid.MarkUserSegment(x[i] - exl, y[i], x[i] + exh, y[i]);
    }
  }
}

void MarkHist1(OccupancyGrid& grid, TH1* h, const std::string& option) {
  // Skip empty frame histograms such as the one from TPad::DrawFrame.
  if (h->GetEntries() == 0 && h->GetSumOfWeights() == 0.0) return;

  const bool has_hist = option.find("HIST") != std::string::npos;
  const bool has_error = !has_hist && option.find('E') != std::string::npos;
  const bool has_marker = !has_hist && option.find('P') != std::string::npos;
  const bool has_curve = !has_hist && HasAny(option, "LC");
  const bool has_step = has_hist || (!has_error && !has_marker && !has_curve);
  const bool has_fill =
      option.find('B') != std::string::npos ||
      (h->GetFillStyle() != 0 && h->GetFillColor() != 0 && h->GetFillStyle() != 4000);
  const double base = gPad->GetLogy() != 0 ? std::pow(10.0, gPad->GetUymin()) : 0.0;

  const int n = h->GetNbinsX();
  for (int i = 1; i <= n; ++i) {
    const double xl = h->GetBinLowEdge(i);
    const double xh = h->GetBinLowEdge(i + 1);
    const double xc = h->GetBinCenter(i);
    const double y = h->GetBinContent(i);
    if (has_step) {
      grid.MarkUserSegment(xl, y, xh, y);
      if (i > 1) grid.MarkUserSegment(xl, h->GetBinContent(i - 1), xl, y);
    }
    if (has_fill) grid.MarkUserRect(xl, base, xh, y);
    if (has_error) {
      grid.MarkUserSegment(xc, y - h->GetBinErrorLow(i), xc, y + h->GetBinErrorUp(i));
    }
    if (has_marker) grid.MarkUserSegment(xc, y, xc, y);
    if (has_curve && i > 1)
      grid.MarkUserSegment(h->GetBinCenter(i - 1), h->GetBinContent(i - 1), xc, y);
  }
}

void MarkHist2(OccupancyGrid& grid, TH2* h) {
  for (int ix = 1; ix <= h->GetNbinsX(); ++ix) {
    for (int iy = 1; iy <= h->GetNbinsY(); ++iy) {
      if (h->GetBinContent(ix, iy) == 0.0) continue;
      grid.MarkUserRect(h->GetXaxis()->GetBinLowEdge(ix), h->GetYaxis()->GetBinLowEdge(iy),
                        h->GetXaxis()->GetBinUpEdge(ix), h->GetYaxis()->GetBinUpEdge(iy));
    }
  }
}

void MarkFunction(OccupancyGrid& grid, TF1* f) {
  double x_min = gPad->GetUxmin(), x_max = gPad->GetUxmax();
  if (gPad->GetLogx() != 0) {
    x_min = std::pow(10.0, x_min);
    x_max = std::pow(10.0, x_max);
  }
  x_min = std::max(x_min, f->GetXmin());
  x_max = std::min(x_max, f->GetXmax());
  const int n = std::max(f->GetNpx(), 2);
  double prev_x = 0.0, prev_y = 0.0;
  for (int i = 0; i <= n; ++i) {
    const double x = gPad->GetLogx() != 0
                         ? x_min * std::pow(x_max / x_min, static_cast<double>(i) / n)
                         : x_min + (x_max - x_min) * i / n;
    const double y = f->Eval(x);
    if (i > 0) grid.MarkUserSegment(prev_x, prev_y, x, y);
    prev_x = x;
    prev_y = y;
  }
}

void MarkText(OccupancyGrid& grid, TText* t) {
  UInt_t w = 0, h = 0;
  if (t->InheritsFrom(TLatex::Class())) {
    static_cast<TLatex*>(t)->GetBoundingBox(w, h);
  } else {
    t->GetTextExtent(w, h, t->GetTitle());
  }
  double x = t->GetX(), y = t->GetY();
  if (!t->TestBit(TText::kTextNDC) && !UserToNdc(t->GetX(), t->GetY(), x, y)) return;

  const int h_align = t->GetTextAlign() / 10;
  const int v_align = t->GetTextAlign() % 10;
  const double fx = h_align == 2 ? 0.5 : (h_align == 3 ? 1.0 : 0.0);
  const double fy = v_align == 2 ? 0.5 : (v_align == 3 ? 1.0 : 0.0);
  const double angle = t->GetTextAngle() * TMath::DegToRad();
  const double c = std::cos(angle), s = std::sin(angle);

  // Corners in pixels relative to the anchor, rotated, then the bounding box in NDC.
  double xa = 1e300, xb = -1e300, ya = 1e300, yb = -1e300;
  for (double u : {-fx * w, (1.0 - fx) * w}) {
    for (double v : {-fy * h, (1.0 - fy) * h}) {
      const double px = u * c - v * s;
      const double py = u * s + v * c;
      xa = std::min(xa, x + px / PadWidthPixel());
      xb = std::max(xb, x + px / PadWidthPixel());
      ya = std::min(ya, y + py / PadHeightPixel());
      yb = std::max(yb, y + py / PadHeightPixel());
    }
  }
  grid.MarkRect(xa, ya, xb, yb);
}

void MarkObject(OccupancyGrid& grid, TObject* obj, const std::string& option,
                const TObject* ignore) {
  if (obj == nullptr || obj == ignore) return;

  if (obj->InheritsFrom(TFrame::Class()) || obj->InheritsFrom(TVirtualPad::Class())) {
    return;
  } else if (obj->InheritsFrom(TPave::Class())) {
    TPave* pave = static_cast<TPave*>(obj);
    grid.MarkRect(pave->GetX1NDC(), pave->GetY1NDC(), pave->GetX2NDC(), pave->GetY2NDC());
  } else if (obj->InheritsFrom(TBox::Class())) {
    TBox* box = static_cast<TBox*>(obj);
    grid.MarkUserRect(box->GetX1(), box->GetY1(), box->GetX2(), box->GetY2());
  } else if (obj->InheritsFrom(TLine::Class())) {
    TLine* line = static_cast<TLine*>(obj);
    if (line->TestBit(TLine::kLineNDC)) {
      grid.MarkSegment(line->GetX1(), line->GetY1(), line->GetX2(), line->GetY2());
    } else {
      grid.MarkUserSegment(line->GetX1(), line->GetY1(), line->GetX2(), line->GetY2());
    }
  } else if (obj->InheritsFrom(TMarker::Class())) {
    TMarker* m = static_cast<TMarker*>(obj);
    double x = 0.0, y = 0.0;
    if (UserToNdc(m->GetX(), m->GetY(), x, y)) {
      const double r = 4.0 * m->GetMarkerSize();
      grid.MarkPoint(x, y, r / PadWidthPixel(), r / PadHeightPixel());
    }
  } else if (obj->InheritsFrom(TText::Class())) {
    MarkText(grid, static_cast<TText*>(obj));
  } else if (obj->InheritsFrom(TMultiGraph::Class())) {
    TMultiGraph* mg = static_cast<TMultiGraph*>(obj);
    TIter next(mg->GetListOfGraphs());
    while (TObject* g = next()) {
      const std::string g_option = ToUpper(next.GetOption());
      MarkObject(grid, g, g_option.empty() ? option : g_option, ignore);
    }
    MarkFunctions(grid, mg->GetListOfFunctions(), ignore);
  } else if (obj->InheritsFrom(TGraph::Class())) {
    TGraph* g = static_cast<TGraph*>(obj);
    MarkGraph(grid, g, option);
    MarkFunctions(grid, g->GetListOfFunctions(), ignore);
  } else if (obj->InheritsFrom(THStack::Class())) {
    THStack* hs = static_cast<THStack*>(obj);
    TObjArray* stack = option.find("NOSTACK") != std::string::npos ? nullptr : hs->GetStack();
    TIter next(stack != nullptr ? static_cast<TCollection*>(stack)
                                : static_cast<TCollection*>(hs->GetHists()));
    while (TObject* h = next()) MarkObject(grid, h, option, ignore);
  } else if (obj->InheritsFrom(TH2::Class())) {
    MarkHist2(grid, static_cast<TH2*>(obj));
  } else if (obj->InheritsFrom(TH1::Class())) {
    MarkHist1(grid, static_cast<TH1*>(obj), option);
    MarkFunctions(grid, static_cast<TH1*>(obj)->GetListOfFunctions(), ignore);
  } else if (obj->InheritsFrom(TF1::Class())) {
    MarkFunction(grid, static_cast<TF1*>(obj));
  }
}

// Occupancy with drawn objects grown by the margin. The margin band along the frame is
// returned as (inset_x, inset_y) cells and is excluded from boxes.
OccupancyGrid BuildOccupancy(double margin, const TObject* ignore, int& inset_x, int& inset_y) {
  gPad->Update();
  OccupancyGrid grid;
  TIter next(gPad->GetListOfPrimitives());
  while (TObject* obj = next()) MarkObject(grid, obj, ToUpper(next.GetOption()), ignore);

  const double margin_px = margin * PadHeightPixel();
  inset_x = static_cast<int>(std::ceil(margin_px / (grid.CellWidth() * PadWidthPixel())));
  inset_y = static_cast<int>(std::ceil(margin_px / (grid.CellHeight() * PadHeightPixel())));
  grid.Dilate(inset_x, inset_y);
  return grid;
}

bool IsRight(LegendPosition corner) {
  return corner == LegendPosition::TopRight || corner == LegendPosition::BottomRight;
}

bool IsTop(LegendPosition corner) {
  return corner == LegendPosition::TopLeft || corner == LegendPosition::TopRight;
}

// Free height in cells for each box width in cells, for a box anchored at a corner.
// heights[w - 1] is the height available for width w and is non-increasing.
std::vector<int> GetFreeHeights(const OccupancyGrid& grid, LegendPosition corner, int inset_x,
                                int inset_y) {
  std::vector<int> heights;
  int limit = grid.ny() - 2 * inset_y;
  for (int c = inset_x; c < grid.nx() - inset_x; ++c) {
    const int ix = IsRight(corner) ? grid.nx() - 1 - c : c;
    int run = 0;
    while (run < limit) {
      const int r = inset_y + run;
      if (grid.At(ix, IsTop(corner) ? grid.ny() - 1 - r : r)) break;
      ++run;
    }
    limit = run;
    if (limit == 0) break;
    heights.push_back(limit);
  }
  return heights;
}

struct CornerSpace {
  LegendPosition corner;
  const OccupancyGrid* grid;
  int inset_x;
  int inset_y;
  std::vector<int> heights;

  double CellWidthPixel() const { return grid->CellWidth() * PadWidthPixel(); }
  double CellHeightPixel() const { return grid->CellHeight() * PadHeightPixel(); }

  LegendBox Box(double width, double height) const {
    LegendBox box;
    box.position = corner;
    const double off_x = inset_x * grid->CellWidth();
    const double off_y = inset_y * grid->CellHeight();
    box.x1 = IsRight(corner) ? grid->x2() - off_x - width : grid->x1() + off_x;
    box.y1 = IsTop(corner) ? grid->y2() - off_y - height : grid->y1() + off_y;
    box.x2 = box.x1 + width;
    box.y2 = box.y1 + height;
    return box;
  }

  // Largest-area box.
  LegendBox MaxBox() const {
    size_t best_w = 0;
    double best_area = 0.0;
    for (size_t w = 1; w <= heights.size(); ++w) {
      const double area = static_cast<double>(w) * heights[w - 1];
      if (area > best_area) {
        best_area = area;
        best_w = w;
      }
    }
    if (best_w == 0) return Box(0.0, 0.0);
    return Box(best_w * grid->CellWidth(), heights[best_w - 1] * grid->CellHeight());
  }

  double MaxArea() const {
    const LegendBox box = MaxBox();
    return box.Width() * PadWidthPixel() * box.Height() * PadHeightPixel();
  }

  // Largest scale (<= 1) at which a width x height NDC box fits.
  double FitScale(double width, double height) const {
    double best = 0.0;
    for (size_t w = 1; w <= heights.size(); ++w) {
      const double s =
          std::min(w * grid->CellWidth() / width, heights[w - 1] * grid->CellHeight() / height);
      best = std::max(best, s);
      if (best >= 1.0) return 1.0;
    }
    return best;
  }
};

constexpr LegendPosition kLegendCorners[] = {LegendPosition::TopRight, LegendPosition::TopLeft,
                                             LegendPosition::BottomRight,
                                             LegendPosition::BottomLeft};

// Place a legend at the best of the given corners. The occupancy must exclude the legend.
TLegend* PlaceLegendAtCorners(TLegend* leg, const std::vector<LegendPosition>& corners,
                              double margin, double text_size) {
  if (leg == nullptr || gPad == nullptr) return leg;

  int inset_x = 0, inset_y = 0;
  const OccupancyGrid grid = BuildOccupancy(margin, leg, inset_x, inset_y);
  const auto [width, height] = GetLegendContentSize(leg, text_size);

  const CornerSpace* best = nullptr;
  std::vector<CornerSpace> spaces;
  spaces.reserve(corners.size());
  for (LegendPosition corner : corners) {
    spaces.push_back(
        {corner, &grid, inset_x, inset_y, GetFreeHeights(grid, corner, inset_x, inset_y)});
  }

  // Prefer corners where the content fits at full size, then the most free area.
  double best_scale = -1.0;
  double best_area = -1.0;
  for (const CornerSpace& space : spaces) {
    const double scale = space.FitScale(width, height);
    const double area = space.MaxArea();
    const bool better = (scale >= 1.0 && best_scale < 1.0) ||
                        (scale >= 1.0 && best_scale >= 1.0 && area > best_area) ||
                        (scale < 1.0 && best_scale < 1.0 && scale > best_scale);
    if (best == nullptr || better) {
      best = &space;
      best_scale = scale;
      best_area = area;
    }
  }

  // Nothing is free: fall back to the first corner at full size.
  const double scale = best_scale > 0.0 ? best_scale : 1.0;
  const LegendBox box = best->Box(width * scale, height * scale);

  leg->SetX1NDC(box.x1);
  leg->SetY1NDC(box.y1);
  leg->SetX2NDC(box.x2);
  leg->SetY2NDC(box.y2);
  leg->SetTextSize(text_size * scale);
  gPad->Modified();
  return leg;
}

}  // namespace

LegendBox GetMaxLegendBox(LegendPosition corner, double margin, const TObject* ignore) {
  int inset_x = 0, inset_y = 0;
  const OccupancyGrid grid = BuildOccupancy(margin, ignore, inset_x, inset_y);
  const CornerSpace space{corner, &grid, inset_x, inset_y,
                          GetFreeHeights(grid, corner, inset_x, inset_y)};
  return space.MaxBox();
}

LegendBox FindMaxLegendBox(double margin, const TObject* ignore) {
  int inset_x = 0, inset_y = 0;
  const OccupancyGrid grid = BuildOccupancy(margin, ignore, inset_x, inset_y);
  LegendBox best;
  double best_area = -1.0;
  for (LegendPosition corner : kLegendCorners) {
    const CornerSpace space{corner, &grid, inset_x, inset_y,
                            GetFreeHeights(grid, corner, inset_x, inset_y)};
    const double area = space.MaxArea();
    if (area > best_area) {
      best_area = area;
      best = space.MaxBox();
    }
  }
  return best;
}

std::pair<double, double> GetLegendContentSize(TLegend* leg, double text_size) {
  const double w_px = PadWidthPixel();
  const double h_px = PadHeightPixel();
  const double text_px = TextHeightPixel(text_size);

  TLatex latex;
  latex.SetTextFont(leg->GetTextFont());
  latex.SetTextSize(text_size);

  // Symbol column and gaps follow the text height so the layout scales with the text.
  const double symbol_px = 2.5 * text_px;
  const double pad_px = 0.4 * text_px;

  double label_px = 0.0;
  double header_px = 0.0;
  int n_entries = 0;
  int n_headers = 0;
  TIter next(leg->GetListOfPrimitives());
  while (TObject* obj = next()) {
    TLegendEntry* entry = static_cast<TLegendEntry*>(obj);
    UInt_t w = 0, h = 0;
    latex.SetTitle(entry->GetLabel());
    latex.GetBoundingBox(w, h);
    const std::string opt = ToUpper(entry->GetOption());
    if (opt.find('H') != std::string::npos) {
      header_px = std::max(header_px, static_cast<double>(w));
      ++n_headers;
    } else {
      label_px = std::max(label_px, static_cast<double>(w));
      ++n_entries;
    }
  }

  const int n_columns = std::max(1, leg->GetNColumns());
  const int n_rows = n_headers + (n_entries + n_columns - 1) / n_columns;
  const double column_px = symbol_px + label_px + pad_px;
  const double width_px = std::max(n_columns * column_px, header_px + 2.0 * pad_px);
  const double height_px = std::max(1, n_rows) * 1.3 * text_px;

  // TLegend reserves fMargin of each column for the symbol.
  leg->SetMargin(static_cast<float>(symbol_px * n_columns / width_px));
  return {width_px / w_px, height_px / h_px};
}

TLegend* PlaceLegend(TLegend* leg, double margin, double text_size) {
  return PlaceLegendAtCorners(
      leg, std::vector<LegendPosition>(std::begin(kLegendCorners), std::end(kLegendCorners)),
      margin, text_size);
}

TLegend* PlaceLegend(TLegend* leg, LegendPosition corner, double margin, double text_size) {
  return PlaceLegendAtCorners(leg, {corner}, margin, text_size);
}

TLegend* PutLegend(Option_t* option, double margin) {
  gPad->Update();
  TLegend* leg = gPad->BuildLegend(0.0, 0.0, 0.1, 0.1, "", option);
  leg->SetBorderSize(0);
  leg->SetFillStyle(0);
  return PlaceLegend(leg, margin);
}

TLegend* PutLegend(LegendPosition leg_pos, Option_t* option, double width, double height) {
  gPad->Update();

  if (width <= 0.0 || height <= 0.0) {
    TLegend* leg = gPad->BuildLegend(0.0, 0.0, 0.1, 0.1, "", option);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    return PlaceLegend(leg, leg_pos);
  }

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

namespace {

// Tick labels of a linear axis between x_min and x_max as ROOT formats them, with the value
// at which each one is drawn.
std::vector<std::pair<double, std::string>> GetLinearAxisLabels(TAxis* axis, double x_min,
                                                                double x_max) {
  std::vector<std::pair<double, std::string>> labels;
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

    labels.emplace_back(val, s.Data());
  }

  return labels;
}

}  // namespace

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
  TText t;
  t.SetTextFont(axis->GetLabelFont());
  t.SetTextSize(axis->GetLabelSize());

  UInt_t max_w = 0;
  UInt_t w = 0;
  UInt_t h = 0;
  for (const auto& label : GetLinearAxisLabels(axis, x_min, x_max)) {
    t.GetTextExtent(w, h, label.second.c_str());
    if (w > max_w) {
      max_w = w;
    }
  }

  return static_cast<double>(max_w) / (gPad->GetWw() * gPad->GetWNDC());
}

void OptimizeYAxisLayout(TAxis* y_axis, double extra_label_shift) {
  if (y_axis == nullptr || gPad == nullptr) {
    return;
  }

  const double label_width = GetMaxLabelWidthNdc(y_axis);
  const double tick_length = y_axis->GetTickLength();
  const double title_size = y_axis->GetTitleSize();
  const double gap = 0.010;
  const double distance_from_axis = tick_length + label_width + gap + extra_label_shift;
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
}

namespace {

struct TickLabel {
  double position;  // Fraction of the axis length from its lower end.
  std::string text;
  bool has_superscript = false;
};

// Labels drawn on an X or Y axis of the active pad and their positions along the axis.
std::vector<TickLabel> GetDrawnAxisLabels(TAxis* axis, bool is_y_axis) {
  std::vector<TickLabel> labels;
  const bool is_log = is_y_axis ? gPad->GetLogy() != 0 : gPad->GetLogx() != 0;
  const double u_min = is_y_axis ? gPad->GetUymin() : gPad->GetUxmin();
  const double u_max = is_y_axis ? gPad->GetUymax() : gPad->GetUxmax();
  if (!(u_max > u_min)) return labels;
  const double tolerance = 1e-6 * (u_max - u_min);
  auto add = [&](double pad_value, const std::string& text, bool has_superscript = false) {
    if (pad_value < u_min - tolerance || pad_value > u_max + tolerance) return;
    labels.push_back({(pad_value - u_min) / (u_max - u_min), text, has_superscript});
  };

  if (axis->GetLabels() != nullptr) {
    for (int i = axis->GetFirst(); i <= axis->GetLast(); ++i) {
      const char* text = axis->GetBinLabel(i);
      if (text != nullptr && text[0] != '\0') {
        const double c = axis->GetBinCenter(i);
        if (!is_log || c > 0.0) add(is_log ? std::log10(c) : c, text);
      }
    }
  } else if (is_log) {
    // Decade labels: ROOT writes 1 and 10 plainly and 10^{n} otherwise.
    for (int n = static_cast<int>(std::ceil(u_min - 1e-9));
         n <= static_cast<int>(std::floor(u_max + 1e-9)); ++n) {
      if (n == 0 || n == 1) {
        add(n, n == 0 ? "1" : "10");
      } else {
        add(n, "10^{" + std::to_string(n) + "}", true);
      }
    }
  } else {
    for (const auto& label : GetLinearAxisLabels(axis, u_min, u_max)) {
      add(label.first, label.second);
    }
  }
  return labels;
}

// Visible width of a label in pixels.
double LabelWidthPixel(TAxis* axis, const std::string& text) {
  TLatex latex;
  latex.SetTextFont(axis->GetLabelFont());
  latex.SetTextSize(axis->GetLabelSize());
  latex.SetTitle(text.c_str());
  UInt_t w = 0, h = 0;
  latex.GetBoundingBox(w, h);
  return w;
}

// Exponent n of the "x10^{n}" factor that TGaxis draws for a linear axis, or 0 if none.
// Follows TGaxis::PaintAxis: the factor is used when the label step is below
// 10^-maxdigits, or when the labels need more than maxdigits digits.
int GetAxisExponent(TAxis* axis, bool is_y_axis) {
  const bool is_log = is_y_axis ? gPad->GetLogy() != 0 : gPad->GetLogx() != 0;
  if (is_log || axis->GetNoExponent() || axis->GetLabels() != nullptr) return 0;

  const double u_min = is_y_axis ? gPad->GetUymin() : gPad->GetUxmin();
  const double u_max = is_y_axis ? gPad->GetUymax() : gPad->GetUxmax();
  if (!(u_max > u_min)) return 0;
  const double tolerance = 1e-6 * (u_max - u_min);
  std::vector<double> values;
  for (const auto& label : GetLinearAxisLabels(axis, u_min, u_max)) {
    if (label.first >= u_min - tolerance && label.first <= u_max + tolerance) {
      values.push_back(label.first);
    }
  }
  if (values.size() < 2) return 0;

  const int max_digits = axis->GetMaxDigits() > 0 ? axis->GetMaxDigits() : TGaxis::GetMaxDigits();
  const double w_min = values.front();
  const double w_max = values.back();
  const int n_steps = static_cast<int>(values.size()) - 1;
  const double epsilon = 1e-6;
  double ww = std::max(std::abs(w_min), std::abs(w_max));
  if (ww == 0.0) return 0;

  if (std::abs(w_max - w_min) / n_steps < std::pow(10.0, -max_digits)) {
    const double af = std::log10(ww) + epsilon;
    if (af < 0) {
      int iexe = std::abs(static_cast<int>(af));
      if (iexe % 3 == 1) iexe += 2;
      if (iexe % 3 == 2) iexe += 1;
      return -iexe;
    }
  }

  const double af = (ww >= 1 ? std::log10(ww) : std::log10(ww * 0.0001)) + epsilon;
  const int nf = static_cast<int>(af) + 1;
  int nexe = 0;
  if (nf > max_digits) {
    while (true) {
      ++nexe;
      ww /= 10;
      if (nexe % 3 == 0 && ww <= std::pow(10.0, max_digits - 1)) break;
    }
  } else if (nf < -max_digits) {
    const double rne = 1 / std::pow(10.0, max_digits - 2);
    while (true) {
      --nexe;
      ww *= 10;
      if (nexe % 3 == 0 && ww >= rne) break;
    }
  }
  return nexe;
}

// Size in pixels of the exponent factor of an axis.
void ExponentSizePixel(TAxis* axis, int exponent, double& w, double& h) {
  TLatex latex;
  latex.SetTextFont(axis->GetLabelFont());
  latex.SetTextSize(axis->GetLabelSize());
  latex.SetTitle(Form("#times10^{#minus%d}", std::abs(exponent)));
  if (exponent > 0) latex.SetTitle(Form("#times10^{%d}", exponent));
  UInt_t wi = 0, hi = 0;
  latex.GetBoundingBox(wi, hi);
  w = wi;
  h = hi;
}

// Label geometry of TGaxis (ROOT 6.36, font 42), checked against rendered pads.
// Linear X labels hang from the frame, their top 0.1 text height below it plus the offset in
// pad-height units. Log X labels sit on a baseline 1.15 label sizes (pad-height units) plus
// the offset below the frame. Y labels end left of the frame, centered on their ticks:
// linear ones 0.07 text height plus the offset (pad-width units) away, log ones 0.08 label
// sizes (pad-width units) plus the offset plus about 1.5 pixels away.
constexpr double kXLabelTopGapLinear = 0.10;
constexpr double kXLabelBaselineLog = 1.15;
constexpr double kYLabelRightGapLinear = 0.07;
constexpr double kYLabelRightGapLog = 0.08;
constexpr double kYLabelRightGapLogPixel = 1.5;
// Ink heights in text heights.
constexpr double kDigitHeight = 0.67;
constexpr double kSuperscriptLabelHeight = 0.85;
// Minimum clearance between labels and from the pad edge, in text heights.
constexpr double kLabelClearance = 0.15;
// Gap between the axis end and the exponent factor, in label sizes.
constexpr double kExponentGap = 0.1;
// Distance per unit title offset, in title sizes along the axis direction.
constexpr double kTitleOffsetUnit = 1.6;

}  // namespace

void AdjustAxisLabelSpace(TAxis* x_axis, TAxis* y_axis) {
  if (x_axis == nullptr || y_axis == nullptr || gPad == nullptr) return;
  // Axis ranges are needed to locate the labels.
  gPad->Update();

  const double w_px = PadWidthPixel();
  const double h_px = PadHeightPixel();
  const double tx = TextHeightPixel(x_axis->GetLabelSize());
  const double ty = TextHeightPixel(y_axis->GetLabelSize());

  // The first X label and the lowest Y label can collide at the bottom-left corner.
  const std::vector<TickLabel> x_labels = GetDrawnAxisLabels(x_axis, false);
  const std::vector<TickLabel> y_labels = GetDrawnAxisLabels(y_axis, true);
  if (!x_labels.empty() && !y_labels.empty() && x_axis->GetLabelSize() > 0.0 &&
      y_axis->GetLabelSize() > 0.0) {
    const TickLabel& x_first = x_labels.front();
    const TickLabel& y_first = y_labels.front();
    const double frame_left = gPad->GetLeftMargin() * w_px;
    const double frame_bottom = gPad->GetBottomMargin() * h_px;
    const double frame_w = (1.0 - gPad->GetLeftMargin() - gPad->GetRightMargin()) * w_px;
    const double frame_h = (1.0 - gPad->GetBottomMargin() - gPad->GetTopMargin()) * h_px;

    const double x_left =
        frame_left + x_first.position * frame_w - 0.5 * LabelWidthPixel(x_axis, x_first.text);
    const double x_height = x_first.has_superscript ? kSuperscriptLabelHeight : kDigitHeight;
    const double y_height = y_first.has_superscript ? kSuperscriptLabelHeight : kDigitHeight;
    const double x_top =
        gPad->GetLogx() == 0
            ? frame_bottom - kXLabelTopGapLinear * tx - x_axis->GetLabelOffset() * h_px
            : frame_bottom -
                  (kXLabelBaselineLog * x_axis->GetLabelSize() + x_axis->GetLabelOffset()) * h_px +
                  x_height * tx;
    const double y_right =
        gPad->GetLogy() == 0
            ? frame_left - kYLabelRightGapLinear * ty - y_axis->GetLabelOffset() * w_px
            : frame_left -
                  (kYLabelRightGapLog * y_axis->GetLabelSize() + y_axis->GetLabelOffset()) * w_px -
                  kYLabelRightGapLogPixel;
    const double y_bottom = frame_bottom + y_first.position * frame_h - 0.5 * y_height * ty;

    const double clearance = kLabelClearance * std::max(tx, ty);
    const double overlap_v = x_top - y_bottom + clearance;
    const double overlap_h = y_right - x_left + clearance;
    if (overlap_v > 0.0 && overlap_h > 0.0) {
      // Moving X labels down and Y labels left by the same distance separates them once the
      // distance reaches the smaller overlap.
      const double shift = std::min(overlap_v, overlap_h);
      x_axis->SetLabelOffset(x_axis->GetLabelOffset() + shift / h_px);
      x_axis->SetTitleOffset(x_axis->GetTitleOffset() +
                             shift / (kTitleOffsetUnit * x_axis->GetTitleSize() * h_px));
      gPad->SetBottomMargin(gPad->GetBottomMargin() + shift / h_px);
      y_axis->SetLabelOffset(y_axis->GetLabelOffset() + shift / w_px);
      OptimizeYAxisLayout(y_axis, shift / w_px);
    }
  }

  // Widen the right margin when the last X label would extend past the pad edge.
  if (!x_labels.empty() && x_axis->GetLabelSize() > 0.0) {
    const TickLabel& x_last = x_labels.back();
    const double left = gPad->GetLeftMargin();
    const double half_w = 0.5 * LabelWidthPixel(x_axis, x_last.text) / w_px;
    const double limit = 1.0 - kLabelClearance * tx / w_px - half_w;
    if (x_last.position > 0.0) {
      const double frame_right = left + (limit - left) / x_last.position;
      const double required_margin = 1.0 - frame_right;
      if (required_margin > gPad->GetRightMargin()) gPad->SetRightMargin(required_margin);
    }
  }

  // TGaxis draws the X exponent right of the axis end (0.1 label size away, bottom-left
  // aligned on the axis) and the Y exponent above the axis end, plus the style offsets.
  if (x_axis->GetLabelSize() > 0.0) {
    const int exponent = GetAxisExponent(x_axis, false);
    if (exponent != 0) {
      double w = 0.0, h = 0.0;
      ExponentSizePixel(x_axis, exponent, w, h);
      Float_t x_off = 0.0, y_off = 0.0;
      gStyle->GetExponentOffset(x_off, y_off, "x");
      const double required_margin =
          kExponentGap * x_axis->GetLabelSize() + x_off + (w + kLabelClearance * tx) / w_px;
      if (required_margin > gPad->GetRightMargin()) gPad->SetRightMargin(required_margin);
    }
  }
  if (y_axis->GetLabelSize() > 0.0) {
    const int exponent = GetAxisExponent(y_axis, true);
    if (exponent != 0) {
      double w = 0.0, h = 0.0;
      ExponentSizePixel(y_axis, exponent, w, h);
      Float_t x_off = 0.0, y_off = 0.0;
      gStyle->GetExponentOffset(x_off, y_off, "y");
      const double required_margin =
          kExponentGap * y_axis->GetLabelSize() + y_off + (h + kLabelClearance * ty) / h_px;
      if (required_margin > gPad->GetTopMargin()) gPad->SetTopMargin(required_margin);
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
