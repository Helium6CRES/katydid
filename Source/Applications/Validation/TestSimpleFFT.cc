/*
 * TestSimpleFFT.cc
 *
 *  Created on: Dec 22, 2011
 *      Author: nsoblath
 *
 *  Will compare the time series and frequency spectrum using Parseval's theorem.
 *     (see http://en.wikipedia.org/wiki/Discrete_Fourier_transform)
 *  If ROOT is present, will draw the time series and frequency spectrum and save the histograms in TestSimpleFFT.root.
 *
 *  Usage: > TestSimpleFFT
 */


#include "complexpolar.hh"
#include "KTFrequencySpectrum.hh"
#include "KTLogger.hh"
#include "KTSimpleFFT.hh"
#include "KTTimeSeries.hh"

#ifdef ROOT_FOUND
#include "TH1.h"
#include "TFile.h"
#endif

#include <cmath>


using namespace std;
using namespace Katydid;

KTLOGGER(vallog, "katydid.applications.validation");

int main(int argc, char** argv)
{
    const Double_t pi = 3.14159265359;

    const UInt_t nBins = 1024;
    const Double_t startTime = 0.;
    const Double_t endTime = 10.;

    const Double_t mult = 30.;

    KTINFO(vallog, "Testing the 1D real-to-complex FFT\n"
           "\tTime series characteristics:\n"
           "\tSize: " << nBins << " bins\n"
           "\tRange: " << startTime << " to " << endTime << " s\n"
           "\tSine wave frequency: " << mult / (2.*pi) << " Hz\n");

    KTTimeSeries* timeSeries = new KTTimeSeries(nBins, startTime, endTime);

    // Fill the time series with a sinusoid.
    // The units are volts.
    for (UInt_t iBin=0; iBin<nBins; iBin++)
    {
        (*timeSeries)[iBin] = sin(timeSeries->GetBinCenter(iBin) * mult);
        //KTDEBUG(vallog, iBin << "  " << (*timeSeries)[iBin]);
    }

    // Create and prepare the FFT
    KTSimpleFFT fullFFT(timeSeries->GetNBins());
    fullFFT.SetTransformFlag("ESTIMATE");
    fullFFT.InitializeFFT();

    // Perform the FFT and get the results
    KTINFO(vallog, "Performing FFT");
    KTFrequencySpectrum* frequencySpectrum = fullFFT.Transform(timeSeries);

    // Find the peak frequency
    Double_t peakFrequency = -1.;
    Double_t maxValue = -999999.;
    size_t nFreqBins = frequencySpectrum->GetNBins();
    for (UInt_t iBin = 0; iBin < nFreqBins; iBin++)
    {
        if ((*frequencySpectrum)[iBin].abs() > maxValue)
        {
            maxValue = (*frequencySpectrum)[iBin].abs();
            peakFrequency = frequencySpectrum->GetBinCenter(iBin);
        }
    }

    KTINFO(vallog, "FFT complete\n"
           "\tFrequency spectrum characteristics:\n"
           "\tSize: " << nFreqBins << " bins\n"
           "\tRange: " << frequencySpectrum->GetRangeMin() << " to " << frequencySpectrum->GetRangeMax() << " Hz\n"
           "\tBin width: " << frequencySpectrum->GetBinWidth() << " Hz\n"
           "\tPeak frequency: " << peakFrequency << " +/- " << 0.5 * frequencySpectrum->GetBinWidth() << " Hz\n");

#ifdef ROOT_FOUND
    TFile* file = new TFile("TestSimpleFFT.root", "recreate");
    TH1D* tsHist = timeSeries->CreateHistogram("hTimeSeries");
    TH1D* fsHist = frequencySpectrum->CreateMagnitudeHistogram("hFreqSpect");
    tsHist->SetDirectory(file);
    fsHist->SetDirectory(file);
    tsHist->Write();
    fsHist->Write();
    file->Close();
    delete file;
#endif

    // Use Parseval's theorem to check the normalization of the FFT
    KTINFO(vallog, "Using Parceval's theorem to check the normalization\n"
           "\tBoth sums should be approximately (1/2) * nBins = " << 0.5 * (Double_t)nBins);
    // the latter is true because the average of sin^2 is 1/2, and we're effectively calculating avg(sin^2)*nbins.

    // Calculate sum(timeSeries[i]^2)
    Double_t tsSum = 0.; // units: volts^2
    for (UInt_t iBin=0; iBin<nBins; iBin++)
    {
        tsSum += (*timeSeries)[iBin] * (*timeSeries)[iBin];
    }

    KTINFO(vallog, "sum(timeSeries[i]^2) = " << tsSum << " V^2");

    // calculate (1/N) * sum(freqSpectrum[i]^2
    Double_t fsSum = 0.; // units: volts^2
    for (UInt_t iBin=0; iBin<nFreqBins; iBin++)
    {
        fsSum += norm((*frequencySpectrum)[iBin]);
    }

    KTINFO(vallog, "sum(freqSpectrum[i]^2) = " << fsSum << " V^2");

    Double_t fractionalDiff = fabs(tsSum - fsSum) / (0.5 * (tsSum + fsSum));
    Double_t threshold = 1.e-4;
    if (fractionalDiff > threshold)
    {
        KTWARN(vallog, "The two sums appear to be unequal! (|diff|/avg > " << threshold << ")\n"
                "\ttsSum - fsSum = " << tsSum - fsSum << "\n"
                "\ttsSum / fsSum = " << tsSum / fsSum << "\n"
                "\t|diff|/avg    = " << fractionalDiff);
    }
    else
    {
        KTINFO(vallog, "The two sums appear to be equal! (|diff|/avg <= " << threshold << ")\n"
                "\t|diff|/avg = " << fractionalDiff);
    }

    delete timeSeries;
    delete frequencySpectrum;

    return 0;

}



