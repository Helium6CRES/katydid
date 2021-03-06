/*
 * KTSlidingWindowFFT.cc
 *
 *  Created on: Sep 12, 2011
 *      Author: nsoblath
 */

#include "KTSlidingWindowFFT.hh"

#include "KTCacheDirectory.hh"
#include "KTEggHeader.hh"
#include "KTBundle.hh"
#include "KTTimeSeriesChannelData.hh"
#include "KTTimeSeriesPairedData.hh"
#include "KTTimeSeriesReal.hh"
#include "KTFactory.hh"
#include "KTPhysicalArray.hh"
#include "KTPStoreNode.hh"
#include "KTSlidingWindowFSData.hh"
#include "KTWindowFunction.hh"

using std::string;
using std::vector;

namespace Katydid
{
    static KTDerivedRegistrar< KTProcessor, KTSlidingWindowFFT > sSWFFTRegistrar("sliding-window-fft");

    KTSlidingWindowFFT::KTSlidingWindowFFT() :
            KTFFT(),
            KTProcessor(),
            fFTPlan(NULL),
            fInputArray(NULL),
            fOutputArray(NULL),
            fTransformFlag("MEASURE"),
            fIsInitialized(kFALSE),
            fUseWisdom(true),
            fWisdomFilename("wisdom_slidingwindowfft.fftw3"),
            fOverlap(0),
            fOverlapFrac(0.),
            fUseOverlapFrac(kFALSE),
            fWindowFunction(NULL),
            fInputDataName("time-series"),
            fOutputDataName("sw-frequency-spectrum"),
            fSingleFFTSignal(),
            fFullFFTSignal()
    {
        fConfigName = "sliding-window-fft";

        RegisterSignal("single_fft", &fSingleFFTSignal, "void (UInt_t, KTFrequencySpectrumPolar*)");
        RegisterSignal("full_fft", &fFullFFTSignal, "void (const KTWriteableData*)");

        RegisterSlot("header", this, &KTSlidingWindowFFT::ProcessHeader, "void (const KTEggHeader*)");
        RegisterSlot("ts-data", this, &KTSlidingWindowFFT::ProcessTimeSeriesData, "void (const KTTimeSeriesData*)");
        RegisterSlot("bundle", this, &KTSlidingWindowFFT::ProcessBundle, "void (KTBundle*)");

        SetupTransformFlagMap();
    }

    KTSlidingWindowFFT::~KTSlidingWindowFFT()
    {
        fftw_destroy_plan(fFTPlan);
        if (fInputArray != NULL) fftw_free(fInputArray);
        if (fOutputArray != NULL) fftw_free(fOutputArray);
        delete fWindowFunction;
    }

    Bool_t KTSlidingWindowFFT::Configure(const KTPStoreNode* node)
    {
        // Config-file options
        SetTransformFlag(node->GetData< string >("transform-flag", fTransformFlag));

        SetUseWisdom(node->GetData<Bool_t>("use-wisdom", fUseWisdom));
        SetWisdomFilename(node->GetData<string>("wisdom-filename", fWisdomFilename));

        if (node->HasData("overlap-time")) SetOverlap(node->GetData< Double_t >("overlap-time", 0));
        if (node->HasData("overlap-size")) SetOverlap(node->GetData< UInt_t >("overlap-size", 0));
        if (node->HasData("overlap-frac")) SetOverlapFrac(node->GetData< Double_t >("overlap-frac", 0.));

        string windowType = node->GetData< string >("window-function-type", "rectangular");
        KTBundleWindowFunction* tempWF = KTFactory< KTBundleWindowFunction >::GetInstance()->Create(windowType);
        if (tempWF == NULL)
        {
            KTERROR(fftlog_sw, "Invalid window function type given: <" << windowType << ">.");
            return false;
        }
        SetWindowFunction(tempWF);

        const KTPStoreNode* childNode = node->GetChild("window-function");
        if (childNode != NULL)
        {
            fWindowFunction->Configure(childNode);
        }

        SetInputDataName(node->GetData< string >("input-data-name", fInputDataName));
        SetOutputDataName(node->GetData< string >("output-data-name", fOutputDataName));

        if (fUseWisdom)
        {
            if (! KTCacheDirectory::GetInstance()->PrepareForUse())
            {
                KTWARN(fftlog_sw, "Unable to use wisdom because cache directory is not ready.");
                fUseWisdom = false;
            }
        }

        // No command-line options

        return true;
    }

    void KTSlidingWindowFFT::ProcessHeader(const KTEggHeader* header)
    {
        fWindowFunction->SetBinWidth(1. / header->GetAcquisitionRate());
        RecreateFFT();
        InitializeFFT();
        return;
    }

    void KTSlidingWindowFFT::ProcessTimeSeriesData(const KTTimeSeriesData* tsData)
    {
        KTSlidingWindowFSData* newData = TransformData(tsData);
        if (tsData->GetBundle() != NULL)
            tsData->GetBundle()->AddData(newData);
        return;
    }


    void KTSlidingWindowFFT::ProcessBundle(KTBundle* bundle)
    {
        const KTTimeSeriesData* tsData = bundle->GetData< KTTimeSeriesData >(fInputDataName);
        if (tsData == NULL)
        {
            KTWARN(fftlog_sw, "No time series data named <" << fInputDataName << "> was available in the bundle");
            return;
        }

        KTSlidingWindowFSData* newData = TransformData(tsData);
        bundle->AddData(newData);
        return;
    }

    void KTSlidingWindowFFT::InitializeFFT()
    {
        if (fWindowFunction == NULL)
        {
            KTERROR(fftlog_sw, "No window function has been set. The FFT has not been initialized.");
            return;
        }

        // fTransformFlag is guaranteed to be valid in the Set method.
        KTDEBUG(fftlog_sw, "Transform flag: " << fTransformFlag);
        TransformFlagMap::const_iterator iter = fTransformFlagMap.find(fTransformFlag);
        Int_t transformFlag = iter->second;

        if (fUseWisdom)
        {
            KTDEBUG(fftlog_sw, "Reading wisdom from file <" << fWisdomFilename << ">");
            if (fftw_import_wisdom_from_filename(fWisdomFilename.c_str()) == 0)
            {
                KTWARN(fftlog_sw, "Unable to read FFTW wisdom from file <" << fWisdomFilename << ">");
            }
        }

        KTDEBUG(fftlog_sw, "Creating plan: " << fWindowFunction->GetSize() << " bins; forward FFT");
        if (fFTPlan != NULL)
            fftw_destroy_plan(fFTPlan);
        fFTPlan = fftw_plan_dft_r2c_1d(fWindowFunction->GetSize(), fInputArray, fOutputArray, transformFlag);
        if (fFTPlan != NULL)
        {
            if (fUseWisdom)
            {
                if (fftw_export_wisdom_to_filename(fWisdomFilename.c_str()) == 0)
                {
                    KTWARN(fftlog_sw, "Unable to write FFTW wisdom to file <" << fWisdomFilename << ">");
                }
            }
            KTDEBUG(fftlog_sw, "FFTW plan created");
            fIsInitialized = true;
        }
        else
        {
            KTWARN(fftlog_sw, "Unable to create FFTW plan!");
            fIsInitialized = false;
        }
        return;
    }

    KTSlidingWindowFSData* KTSlidingWindowFFT::TransformData(const KTTimeSeriesData* tsData)
    {
        if (! fIsInitialized)
        {
            KTWARN(fftlog_sw, "FFT must be initialized before the transform is performed.\n" <<
                    "Please first call InitializeFFT(), and then perform the transform.");
            return NULL;
        }

        KTSlidingWindowFSData* newData = new KTSlidingWindowFSData(tsData->GetNTimeSeries());

        for (UInt_t iChannel = 0; iChannel < tsData->GetNTimeSeries(); iChannel++)
        {
            const KTTimeSeriesReal* nextInput = dynamic_cast< const KTTimeSeriesReal* >(tsData->GetTimeSeries(iChannel));
            if (nextInput == NULL)
            {
                KTERROR(fftlog_sw, "Incorrect time series type: time series did not cast to KTTimeSeriesReal.");
                return NULL;
            }
            KTPhysicalArray< 1, KTFrequencySpectrumPolar* >* newResults = NULL;
            try
            {
                newResults = Transform(nextInput);
            }
            catch (std::exception& e)
            {
                KTERROR(fftlog_sw, "Channel " << iChannel << " did not transform correctly:\n" << e.what());
                return NULL;
            }
            newData->SetSpectra(newResults, iChannel);
        }

        newData->SetBundle(tsData->GetBundle());
        newData->SetName(fOutputDataName);

        fFullFFTSignal(newData);

        return newData;
    }

    KTPhysicalArray< 1, KTFrequencySpectrumPolar* >* KTSlidingWindowFFT::Transform(const KTTimeSeriesReal* data) const
    {
        // # of time bins in each FFT, and the number of bins in the data
        UInt_t windowSize = fWindowFunction->GetSize();
        UInt_t dataSize = data->size();
        if (windowSize < dataSize)
        {
            // Characteristics of the whole windowed FFT
            UInt_t windowShift = windowSize - GetEffectiveOverlap();
            UInt_t nWindows = (dataSize - windowSize) / windowShift + 1; // integer arithmetic gets truncated to the nearest integer
            UInt_t nTimeBinsUsed = windowSize + (nWindows - 1) * windowShift;
            UInt_t nTimeBinsNotUsed = dataSize - nTimeBinsUsed;

            // Characteristics of the frequency axis
            Double_t timeBinWidth = data->GetTimeBinWidth();
            Double_t freqMin = GetMinFrequency(timeBinWidth);
            Double_t freqMax = GetMaxFrequency(timeBinWidth);

            // Characteristics of the time axis
            Double_t timeMin = 0.;
            Double_t timeMax = nTimeBinsUsed * timeBinWidth;
            KTPhysicalArray< 1, KTFrequencySpectrumPolar* >* newSpectra = new KTPhysicalArray< 1, KTFrequencySpectrumPolar* >(nWindows, timeMin, timeMax);

            KTDEBUG(fftlog_sw, "Performing windowed FFT\n"
                    << "\tWindow size: " << windowSize << '\n'
                    << "\tWindow shift: " << windowShift << '\n'
                    << "\t# of windows: " << nWindows << '\n'
                    << "\t# of unused bins: " << nTimeBinsNotUsed)

            UInt_t windowStart = 0;
            for (UInt_t iWindow = 0; iWindow < nWindows; iWindow++)
            {
                KTDEBUG(fftlog_sw, "Window: " << iWindow << "; first bin: " << windowStart);
                copy(data->begin() + windowStart, data->begin() + windowStart + fWindowFunction->GetSize(), fInputArray);
                fftw_execute(fFTPlan);
                (*newSpectra)(iWindow) = ExtractTransformResult(freqMin, freqMax);
                // emit a signal that the FFT was performed, for any connected slots
                fSingleFFTSignal(iWindow, (*newSpectra)(iWindow));
                windowStart += windowShift;
            }
            KTINFO(fftlog_sw, "FFTs complete; windows used: " << nWindows << "; time bins not used: " << nTimeBinsNotUsed);
            return newSpectra;
       }

       KTERROR(fftlog_sw, "Window size is larger than time data: " << windowSize << " > " << dataSize << "\n" <<
              "No transform was performed!");
       throw(std::length_error("Window size is larger than time data"));
       return NULL;
    }

    KTFrequencySpectrumPolar* KTSlidingWindowFFT::ExtractTransformResult(Double_t freqMin, Double_t freqMax) const
    {
        UInt_t freqSize = GetFrequencySize();
        Double_t normalization = sqrt(2. / (Double_t)GetTimeSize());

        Double_t tempReal, tempImag;
        KTFrequencySpectrumPolar* newSpect = new KTFrequencySpectrumPolar(freqSize, freqMin, freqMax);
        for (Int_t iPoint = 0; iPoint<freqSize; iPoint++)
        {
            (*newSpect)(iPoint).set_rect(fOutputArray[iPoint][0], fOutputArray[iPoint][1]);
            (*newSpect)(iPoint) *= normalization;
        }

        return newSpect;
    }

    void KTSlidingWindowFFT::SetWindowSize(UInt_t nBins)
    {
        if (fWindowFunction == NULL)
        {
            KTERROR(fftlog_sw, "Window function has not been set.");
            return;
        }
        fWindowFunction->SetSize(nBins);
        RecreateFFT();
        return;
    }

    void KTSlidingWindowFFT::SetWindowLength(Double_t wlTime)
    {
        if (fWindowFunction == NULL)
        {
            KTERROR(fftlog_sw, "Window function has not been set.");
            return;
        }
        fWindowFunction->SetLength(wlTime);
        RecreateFFT();
        return;
    }

    void KTSlidingWindowFFT::SetWindowFunction(KTBundleWindowFunction* wf)
    {
        delete fWindowFunction;
        fWindowFunction = wf;
        RecreateFFT();
        return;
    }

    void KTSlidingWindowFFT::RecreateFFT()
    {
        if (fWindowFunction == NULL)
        {
            KTERROR(fftlog_sw, "No window function has been set. The FFT has not been recreated.");
            return;
        }

        fftw_destroy_plan(fFTPlan);
        fFTPlan = NULL;
        fftw_free(fInputArray);
        fftw_free(fOutputArray);
        fInputArray = (double*) fftw_malloc(sizeof(double) * fWindowFunction->GetSize());
        fOutputArray = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * CalculateNFrequencyBins(fWindowFunction->GetSize()));
        fIsInitialized = false;
    }

     void KTSlidingWindowFFT::SetupTransformFlagMap()
     {
         fTransformFlagMap.clear();
         fTransformFlagMap["ESTIMATE"] = FFTW_ESTIMATE;
         fTransformFlagMap["MEASURE"] = FFTW_MEASURE;
         fTransformFlagMap["PATIENT"] = FFTW_PATIENT;
         fTransformFlagMap["EXHAUSTIVE"] = FFTW_EXHAUSTIVE;
         return;
     }


} /* namespace Katydid */
