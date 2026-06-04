#ifndef ROOTHELPER_ANALYSIS_H_
#define ROOTHELPER_ANALYSIS_H_

#include <TAxis.h>
#include <TGraphErrors.h>

/**
 * @namespace roothelper
 * @brief Main namespace for the ROOT helper library.
 */
namespace roothelper {

/**
 * @brief Scale the X-axis of a histogram by a constant factor.
 *
 * Creates a copy of the given histogram with its X-axis bin edges scaled by the given factor.
 * Useful for changing units of the X-axis (e.g., from seconds to milliseconds).
 *
 * @param h Pointer to the input histogram.
 * @param scale The scaling factor to apply to the X-axis.
 * @return TH1* Pointer to the newly scaled histogram (caller takes ownership).
 */
TH1* ScaleHistoX(TH1* h, double scale);

/**
 * @brief Convert a histogram to a density histogram.
 *
 * Normalizes each bin content and error by dividing by the bin width.
 * Useful for comparing histograms with non-uniform binning.
 *
 * @param h Pointer to the input histogram.
 * @return TH1* Pointer to the density histogram (caller takes ownership).
 */
TH1* ConvertToDensityHisto(TH1* h);

/**
 * @brief Compute the linear combination of two TGraphErrors: a * g0 + g1.
 *
 * Evaluates the new graph points as Y_new = a * Y_0 + Y_1, propagating the errors on Y.
 * Assumes g0 and g1 have the same X points.
 *
 * @param a Scaling factor for the first graph.
 * @param g0 Pointer to the first TGraphErrors.
 * @param g1 Pointer to the second TGraphErrors.
 * @return TGraphErrors* Pointer to the resulting TGraphErrors (caller takes ownership).
 */
TGraphErrors* GetGraphG0xAPlusG1(double a, const TGraphErrors* g0, const TGraphErrors* g1);

}  // namespace roothelper

#endif  // ROOTHELPER_ANALYSIS_H_
