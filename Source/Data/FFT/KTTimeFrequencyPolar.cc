/*
 * KTTimeFrequencyPolar.cc
 *
 *  Created on: Aug 28, 2012
 *      Author: nsoblath
 */

#include "KTTimeFrequencyPolar.hh"

#include "KTLogger.hh"
#include "KTPowerSpectrum.hh"

#ifdef ROOT_FOUND
#include "TH2.h"
#endif

#include <sstream>

#ifdef USE_OPENMP
#include <omp.h>
#endif

using std::stringstream;

namespace Katydid
{
    KTLOGGER(fslog, "katydid.fft");

    KTTimeFrequencyPolar::KTTimeFrequencyPolar() :
            KTPhysicalArray< 2, complexpolar< double > >(),
            KTTimeFrequency()
    {
    }

    KTTimeFrequencyPolar::KTTimeFrequencyPolar(size_t nTimeBins, double timeRangeMin, double timeRangeMax, size_t nFreqBins, double freqRangeMin, double freqRangeMax) :
            KTPhysicalArray< 2, complexpolar< double > >(nTimeBins, timeRangeMin, timeRangeMax, nFreqBins, freqRangeMin, freqRangeMax),
            KTTimeFrequency()
    {
    }
    KTTimeFrequencyPolar::KTTimeFrequencyPolar(const KTTimeFrequencyPolar& orig) :
            KTPhysicalArray< 2, complexpolar< double > >(orig),
            KTTimeFrequency()
    {
    }

    KTTimeFrequencyPolar::~KTTimeFrequencyPolar()
    {
    }

    KTTimeFrequencyPolar& KTTimeFrequencyPolar::operator=(const KTTimeFrequencyPolar& rhs)
    {
        KTPhysicalArray< 2, complexpolar< double > >::operator=(rhs);
        return *this;
    }

    KTTimeFrequencyPolar& KTTimeFrequencyPolar::CConjugate()
    {
        unsigned nTimeBins = size(1);
        unsigned nFreqBins = size(2);
#pragma omp parallel for
        for (unsigned iTimeBin=0; iTimeBin<nTimeBins; iTimeBin++)
        {
            for (unsigned iFreqBin=0; iFreqBin<nFreqBins; iFreqBin++)
            {
                (*this)(iTimeBin, iFreqBin).conj();
            }
        }
        return *this;
    }

/*
    KTPowerSpectrum* KTTimeFrequencyPolar::CreatePowerSpectrum() const
    {
        unsigned nBins = size();
        KTPowerSpectrum* newPS = new KTPowerSpectrum(GetBinWidth(), GetRangeMin(), GetRangeMax());
        double value;
        double scaling = 1. / KTPowerSpectrum::GetResistance();
#pragma omp parallel for private(value)
        for (unsigned iBin=0; iBin<nBins; iBin++)
        {
           value = (*this)(iBin).abs();
           (*newPS)(iBin) = value * value * scaling;
        }
        return newPS;
    }

    void KTTimeFrequencyPolar::Print(unsigned startPrint, unsigned nToPrint) const
    {
        stringstream printStream;
        for (unsigned iBin = startPrint; iBin < startPrint + nToPrint; iBin++)
        {
            printStream << "Bin " << iBin << ";   x = " << GetBinCenter(iBin) <<
                    ";   y = " << (*this)(iBin) << "\n";
        }
        KTDEBUG(fslog, "\n" << printStream.str());
        return;
    }
*/

#ifdef ROOT_FOUND
    TH2D* KTTimeFrequencyPolar::CreateMagnitudeHistogram(const std::string& name) const
    {
        unsigned nTimeBins = size(1);
        unsigned nFreqBins = size(2);
        TH2D* hist = new TH2D(name.c_str(), "Frequency vs. Time: Magnitude", (Int_t)nTimeBins, GetRangeMin(1), GetRangeMax(1), (Int_t)nFreqBins, GetRangeMin(2), GetRangeMax(2));
        //double value;
        for (unsigned iTimeBin=0; iTimeBin<nTimeBins; iTimeBin++)
        {
            for (unsigned iFreqBin=0; iFreqBin<nFreqBins; iFreqBin++)
            {
                hist->SetBinContent((Int_t)iTimeBin + 1, (Int_t)iFreqBin + 1, (*this)(iTimeBin, iFreqBin).abs());
            }
        }
        hist->SetXTitle("Time (s)");
        hist->SetYTitle("Frequency (Hz)");
        hist->SetZTitle("Voltage (V)");
        return hist;
    }
    TH2D* KTTimeFrequencyPolar::CreatePhaseHistogram(const std::string& name) const
    {
        unsigned nTimeBins = size(1);
        unsigned nFreqBins = size(2);
        TH2D* hist = new TH2D(name.c_str(), "Frequency vs. Time: Phase", (Int_t)nTimeBins, GetRangeMin(1), GetRangeMax(1), (Int_t)nFreqBins, GetRangeMin(2), GetRangeMax(2));
        //double value;
        for (unsigned iTimeBin=0; iTimeBin<nTimeBins; iTimeBin++)
        {
            for (unsigned iFreqBin=0; iFreqBin<nFreqBins; iFreqBin++)
            {
                hist->SetBinContent((Int_t)iTimeBin + 1, (Int_t)iFreqBin + 1, (*this)(iTimeBin, iFreqBin).arg());
            }
        }
        hist->SetXTitle("Time (s)");
        hist->SetYTitle("Frequency (Hz)");
        hist->SetZTitle("Phase");
        return hist;
    }
    TH2D* KTTimeFrequencyPolar::CreatePowerHistogram(const std::string& name) const
    {
        unsigned nTimeBins = size(1);
        unsigned nFreqBins = size(2);
        TH2D* hist = new TH2D(name.c_str(), "Power Spectrum", (Int_t)nTimeBins, GetRangeMin(1), GetRangeMax(1), (Int_t)nFreqBins, GetRangeMin(2), GetRangeMax(2));
        double value;
        double scaling = 1. / KTPowerSpectrum::GetResistance();
        for (unsigned iTimeBin=0; iTimeBin<nTimeBins; iTimeBin++)
        {
            for (unsigned iFreqBin=0; iFreqBin<nFreqBins; iFreqBin++)
            {
                value = (*this)(iTimeBin, iFreqBin).abs();
                hist->SetBinContent((Int_t)iTimeBin + 1, (Int_t)iFreqBin + 1, value * value * scaling);
            }
        }
        hist->SetXTitle("Time (s)");
        hist->SetYTitle("Frequency (Hz)");
        hist->SetZTitle("Power (W)");
        return hist;
    }
    /*
    TH1D* KTTimeFrequencyPolar::CreatePowerDistributionHistogram(const std::string& name) const
    {
        double tMaxMag = -1.;
        double tMinMag = 1.e9;
        unsigned nBins = size();
        double value;
        double scaling = 1. / KTPowerSpectrum::GetResistance();
        for (unsigned iBin=0; iBin<nBins; iBin++)
        {
            value = (*this)(iBin).abs();
            value *= value * scaling;
            if (value < tMinMag) tMinMag = value;
            if (value > tMaxMag) tMaxMag = value;
        }
        if (tMinMag < 1. && tMaxMag > 1.) tMinMag = 0.;
        TH1D* hist = new TH1D(name.c_str(), "Power Distribution", 100, tMinMag*0.95, tMaxMag*1.05);
        for (unsigned iBin=0; iBin<nBins; iBin++)
        {
            value = (*this)(iBin).abs();
            hist->Fill(value * value * scaling);
        }
        hist->SetXTitle("Power (W)");
        return hist;
    }
    */
#endif

} /* namespace Katydid */
