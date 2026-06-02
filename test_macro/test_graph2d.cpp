#include "../roothelper.h"

#include <string>
#include <TRandom.h>
#include <TCanvas.h>
#include <TGraph2D.h>

namespace rh = roothelper;

int test_graph2d() {
    rh::Prepare();

    TCanvas* c = rh::CreateCanvas("c_Graph2DTest", "TGraph2D Test with COLZ");

    // Create a 2D graph with some sample points (e.g. a simple 2D function)
    TGraph2D* g = new TGraph2D();
    g->SetName("g_2d");
    g->SetTitle("TGraph2D COLZ Test;X Title;Y Title;Z Title (Color Palette)");

    TRandom r;
    int n = 200;
    for (int i = 0; i < n; ++i) {
        double x = r.Uniform(-2, 2);
        double y = r.Uniform(-2, 2);
        double z = x*x * exp(-x*x - y*y); // 2D function shape
        g->SetPoint(i, x, y, z);
    }

    // Draw the 2D graph with COLZ
    g->Draw("COLZ");

    // Apply roothelper style and set axes (specifically passing "COLZ" so Z-axis increases right margin)
    rh::SetAxes(g, "COLZ");

    // Save the canvas using DataSaver
    rh::DataSaver data_saver("GraphicsTest");
    data_saver.WriteCanvas(c);

    return 0;
}
