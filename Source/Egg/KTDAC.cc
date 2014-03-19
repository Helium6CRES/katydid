/*
 * KTDAC.cc
 *
 *  Created on: Dec 24, 2013
 *      Author: N.S. Oblath
 */

#include "KTDAC.hh"

#include "KTParam.hh"
#include "KTRawTimeSeries.hh"
#include "KTRawTimeSeriesData.hh"
#include "KTTimeSeriesData.hh"
#include "KTTimeSeriesFFTW.hh"
#include "KTTimeSeriesReal.hh"

#include "thorax.hh"

using std::string;

namespace Katydid
{
    KTLOGGER(egglog, "KTDAC");

    KT_REGISTER_PROCESSOR(KTDAC, "dac");

    KTDAC::KTDAC(const std::string& name) :
            KTProcessor(name),
            fNBits(8),
            fMinVoltage(-0.25),
            fVoltageRange(0.5),
            fTimeSeriesType(kRealTimeSeries),
            fBitDepthMode(kNoChange),
            fEmulatedNBits(fNBits),
            fShouldRunInitialize(true),
            fVoltages(),
            fConvertTSFunc(&KTDAC::ConvertToReal),
            fOversamplingBins(1),
            fOversamplingScaleFactor(1.),
            fTimeSeriesSignal("ts", this),
            fRawTSSlot("raw-ts", this, &KTDAC::ConvertData, &fTimeSeriesSignal)
    {
    }

    KTDAC::~KTDAC()
    {
    }

    bool KTDAC::Configure(const KTParamNode* node)
    {
        if (node == NULL) return false;

        SetNBits(node->GetValue< unsigned >("n-bits", fNBits));
        SetMinVoltage(node->GetValue< double >("min-voltage", fMinVoltage));
        SetVoltageRange(node->GetValue< double >("voltage-range", fVoltageRange));

        string timeSeriesTypeString = node->GetValue("time-series-type", "real");
        if (timeSeriesTypeString == "real") SetTimeSeriesType(kRealTimeSeries);
        else if (timeSeriesTypeString == "fftw") SetTimeSeriesType(kFFTWTimeSeries);
        else
        {
            KTERROR(egglog, "Illegal string for time series type: <" << timeSeriesTypeString << ">");
            return false;
        }

        if (node->Has("n-bits-emulated"))
        {
            SetEmulatedNBits(node->GetValue< unsigned >("n-bits-emulated", fEmulatedNBits));
        }

        Initialize();

        return true;
    }

    void KTDAC::Initialize()
    {
        if (! fVoltages.empty())
        {
            fVoltages.clear();
        }

        unsigned levelDivisor = 1;

        dig_calib_params params;
        if (fBitDepthMode == kReducing)
        {
            get_calib_params(fEmulatedNBits, sizeof(uint64_t), fMinVoltage, fVoltageRange, &params);
            levelDivisor = 1 << (fNBits - fEmulatedNBits);
        }
        else
        {
            get_calib_params(fNBits, sizeof(uint64_t), fMinVoltage, fVoltageRange, &params);
        }

        if (fBitDepthMode == kIncreasing)
        {
            if (fNBits >= fEmulatedNBits)
            {
                KTERROR(egglog, "Increasing-bit-depth mode was indicated, but emulated bits (" << fEmulatedNBits << ") <= actual bits (" << fNBits << ")");
                return;
            }
            unsigned additionalBits = fEmulatedNBits - fNBits;
            fOversamplingBins = pow(2, 2 * additionalBits);
            fOversamplingScaleFactor = 1. / pow(2, additionalBits);
        }

        KTDEBUG(egglog, "Assigning voltages with:\n" <<
                "\tDigitizer bits: " << fNBits << '\n' <<
                "\tVoltage levels: " << 1 << fNBits << '\n' <<
                "\tEmulated bits: " << fEmulatedNBits << '\n' <<
                "\tLevel divisor: " << levelDivisor << '\n' <<
                "\tReduced levels: " << params.levels << '\n' <<
                "\tVoltage range: " << params.v_range << '\n' <<
                "\tMinimum voltage: " << params.v_min << " V\n" <<
                "\tOversampling bins: " << fOversamplingBins << '\n' <<
                "\tOversampling scale factor: " << fOversamplingScaleFactor << '\n');


        fVoltages.resize(params.levels);
        for (uint64_t level = 0; level < params.levels; ++level)
        {
            fVoltages[level] = dd2a(level / levelDivisor, &params);
            KTWARN(egglog, "level " << level << ", voltage " << fVoltages[level]);
        }

        fShouldRunInitialize = false;
        return;
    }

    bool KTDAC::ConvertData(KTRawTimeSeriesData& rawData)
    {
        unsigned nComponents = rawData.GetNComponents();
        KTTimeSeriesData& newData = rawData.Of< KTTimeSeriesData >().SetNComponents(nComponents);
        for (unsigned component = 0; component < nComponents; ++component)
        {
            KTTimeSeries* newTS = (this->*fConvertTSFunc)(rawData.GetTimeSeries(component));
            newData.SetTimeSeries(newTS);
        }
        return true;
    }

    KTTimeSeries* KTDAC::ConvertToFFTW(KTRawTimeSeries* ts)
    {
        if (fShouldRunInitialize)
            Initialize();

        unsigned nBins = ts->size();
        KTTimeSeriesFFTW* newTS = new KTTimeSeriesFFTW(nBins, ts->GetRangeMin(), ts->GetRangeMax());
        for (unsigned bin = 0; bin < nBins; ++bin)
        {
            (*newTS)(bin)[0] = Convert((*ts)(bin));
        }
        return newTS;
    }

    KTTimeSeries* KTDAC::ConvertToReal(KTRawTimeSeries* ts)
    {
        if (fShouldRunInitialize)
            Initialize();

        unsigned nBins = ts->size();
        KTTimeSeriesReal* newTS = new KTTimeSeriesReal(nBins, ts->GetRangeMin(), ts->GetRangeMax());
        for (unsigned bin = 0; bin < nBins; ++bin)
        {
            (*newTS)(bin) = Convert((*ts)(bin));
        }
        return newTS;
    }

    KTTimeSeries* KTDAC::ConvertToFFTWOversampled(KTRawTimeSeries* ts)
    {
        if (fShouldRunInitialize)
            Initialize();

        unsigned nBins = ts->size() / fOversamplingBins;
        KTTimeSeriesFFTW* newTS = new KTTimeSeriesFFTW(nBins, ts->GetRangeMin(), ts->GetRangeMax());
        double avgValue;
        unsigned bin = 0;
        for (unsigned oversampledBin = 0; oversampledBin < nBins; ++oversampledBin)
        {
            avgValue = 0.;
            for (unsigned iOSBin = 0; iOSBin < fOversamplingBins; ++iOSBin)
            {
                avgValue += Convert((*ts)(bin));
                ++bin;
            }
            (*newTS)(oversampledBin)[0] = avgValue * fOversamplingScaleFactor;
        }
#ifndef NDEBUG
        if (bin != ts->size())
        {
            KTWARN(egglog, "Data lost upon oversampling: " << ts->size() - bin << " samples");
        }
#endif
        return newTS;
    }

    KTTimeSeries* KTDAC::ConvertToRealOversampled(KTRawTimeSeries* ts)
    {
        if (fShouldRunInitialize)
            Initialize();

        unsigned nBins = ts->size() / fOversamplingBins;
        KTTimeSeriesReal* newTS = new KTTimeSeriesReal(nBins, ts->GetRangeMin(), ts->GetRangeMax());
        double avgValue;
        unsigned bin = 0;
        for (unsigned oversampledBin = 0; oversampledBin < nBins; ++oversampledBin)
        {
            avgValue = 0.;
            for (unsigned iOSBin = 0; iOSBin < fOversamplingBins; ++iOSBin)
            {
                avgValue += Convert((*ts)(bin));
                ++bin;
            }
            (*newTS)(oversampledBin) = avgValue * fOversamplingScaleFactor;
        }
#ifndef NDEBUG
        if (bin != ts->size())
        {
            KTWARN(egglog, "Data lost upon oversampling: " << ts->size() - bin << " samples");
        }
#endif
        return newTS;
    }

    void KTDAC::SetTimeSeriesType(KTDAC::TimeSeriesType type)
    {
        fTimeSeriesType = type;
        if (type == kFFTWTimeSeries)
        {
            if (fBitDepthMode != kIncreasing)
            {
                fConvertTSFunc = &KTDAC::ConvertToFFTW;
                KTDEBUG(egglog, "Convert function set to FFTW");
            }
            else
            {
                KTDEBUG(egglog, "Convert function set to FFTW oversampled");
                fConvertTSFunc = &KTDAC::ConvertToFFTWOversampled;
            }
        }
        else
        {
            if (fBitDepthMode != kIncreasing)
            {
                KTDEBUG(egglog, "Convert function set to real");
                fConvertTSFunc = &KTDAC::ConvertToReal;
            }
            else
            {
                KTDEBUG(egglog, "Convert function set to real oversampled");
                fConvertTSFunc = &KTDAC::ConvertToRealOversampled;
            }
        }

        fShouldRunInitialize = true;
        return;
    }

    bool KTDAC::SetEmulatedNBits(unsigned nBits)
    {
        if (nBits == fNBits)
        {
            fBitDepthMode = kNoChange;
            fEmulatedNBits = fNBits;
        }
        else if (nBits > fNBits)
        {
            fBitDepthMode = kIncreasing;
            fEmulatedNBits = nBits;
        }
        else
        {
            // otherwise nBits < fNBits
            fBitDepthMode = kReducing;
            fEmulatedNBits = nBits;
        }

        // now update the conversion function pointer
        if (fConvertTSFunc == &KTDAC::ConvertToReal || fConvertTSFunc == &KTDAC::ConvertToRealOversampled)
        {
            if (fBitDepthMode != kIncreasing)
            {
                KTDEBUG(egglog, "Convert function set to real");
                fConvertTSFunc = &KTDAC::ConvertToReal;
            }
            else
            {
                KTDEBUG(egglog, "Convert function set to real oversampled");
                fConvertTSFunc = &KTDAC::ConvertToRealOversampled;
            }
        }
        else
        {
            if (fBitDepthMode != kIncreasing)
            {
                KTDEBUG(egglog, "Convert function set to FFTW");
                fConvertTSFunc = &KTDAC::ConvertToFFTW;
            }
            else
            {
                KTDEBUG(egglog, "Convert function set to FFTW oversampled");
                fConvertTSFunc = &KTDAC::ConvertToFFTWOversampled;
            }
        }

        fShouldRunInitialize = true;
        return true;
    }

} /* namespace Katydid */
