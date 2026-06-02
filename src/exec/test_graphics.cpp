#include <roothelper/roothelper.h>

#include <string>
#include <cmath>

#include <TCanvas.h>
#include <TF1.h>
#include <TMath.h>
#include <TGraph2D.h>
#include <TRandom.h>


namespace rh = roothelper;


TCanvas* create_test_canvas(std::string canvas_name);
TCanvas* create_test_canvas_2d(std::string canvas_name);


rh::DataSaver data_saver("test_macro/GraphicsTest");


int main(int argc, char** argv)
{
    rh::Prepare();

    /**
     * Single pad.
     * 8pt text size with 86mm width.
     * Increasing right margin of the pad for a label at the end of x-axis.
     */
    create_test_canvas("SinglePad_8pt_LabelAtXend");

    /**
     * Single pad.
     * 10pt text size with 86mm width.
     * Increasing right margin of the pad for a label at the end of x-axis.
     */
    rh::GraphicsSize::current = rh::g_size_10pt;
    create_test_canvas("SinglePad_10pt_LabelAtXend");


    /**
     * 2D graph with COLZ.
     * 8pt text size.
     */
    rh::GraphicsSize::current = rh::g_size_8pt;
    create_test_canvas_2d("Graph2DTest");

    return 0;
}


TCanvas* create_test_canvas(std::string canvas_name)
{
    canvas_name = std::string("c_") + canvas_name;
    TCanvas* c = rh::CreateCanvas(canvas_name.c_str(), canvas_name.c_str());

    TF1* f_wave = new TF1("f_wave", [](const double* x, const double* p){ return p[0] * TMath::Sin(TMath::TwoPi() * p[1] * x[0]); }, 0, 1, 2);
    f_wave->SetLineColor(rh::GetColorInRing(0));
    f_wave->SetParameters(1, 4);
    f_wave->Draw("L");
    rh::SetAxes(f_wave);
    f_wave->GetXaxis()->SetTitle("xyzABC (arb. units)");
    f_wave->GetYaxis()->SetTitle("yzxBCA (arb. units)");
    std::cout << rh::IncreaseRightMargin(1) << std::endl;

    data_saver.WriteCanvas(c);

    return c;
}


TCanvas* create_test_canvas_2d(std::string canvas_name)
{
    canvas_name = std::string("c_") + canvas_name;
    TCanvas* c = rh::CreateCanvas(canvas_name.c_str(), "TGraph2D Test with COLZ");

    TGraph2D* g = new TGraph2D();
    g->SetName("g_2d");
    g->SetTitle("TGraph2D COLZ Test;X Title;Y Title;Z Title (Color Palette)");

    TRandom r;
    int n = 200;
    for (int i = 0; i < n; ++i) {
        double x = r.Uniform(-2, 2);
        double y = r.Uniform(-2, 2);
        double z = x*x * std::exp(-x*x - y*y);
        g->SetPoint(i, x, y, z);
    }

    g->Draw("COLZ");
    rh::SetAxes(g, "COLZ");

    data_saver.WriteCanvas(c);

    return c;
}
