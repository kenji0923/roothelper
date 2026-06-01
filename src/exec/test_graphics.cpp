#include <roothelper/roothelper.h>

#include <string>

#include <TCanvas.h>
#include <TF1.h>
#include <TMath.h>


namespace rh = roothelper;


TCanvas* create_test_canvas(std::string canvas_name);


rh::DataSaver data_saver("GraphicsTest");


int main(int argc, char** argv)
{
    rh::prepare();

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

    return 0;
}


TCanvas* create_test_canvas(std::string canvas_name)
{
    canvas_name = std::string("c_") + canvas_name;
    TCanvas* c = rh::create_canvas(canvas_name.c_str(), canvas_name.c_str());

    TF1* f_wave = new TF1("f_wave", [](const double* x, const double* p){ return p[0] * TMath::Sin(TMath::TwoPi() * p[1] * x[0]); }, 0, 1, 2);
    f_wave->SetLineColor(rh::get_color_in_ring(0));
    f_wave->SetParameters(1, 4);
    f_wave->Draw("L");
    rh::set_axes(f_wave);
    f_wave->GetXaxis()->SetTitle("xyzABC (arb. units)");
    f_wave->GetYaxis()->SetTitle("yzxBCA (arb. units)");
    std::cout << rh::increase_right_margin(1) << std::endl;

    data_saver.write_canvas(c);

    return c;
}
