/*
 * KTWindower.cc
 *
 *  Created on: Apr 18, 2013
 *      Author: nsoblath
 */

#include "KTWindower.hh"

#include "KTEggHeader.hh"
#include "KTNOFactory.hh"
#include "KTLogger.hh"
#include "KTPStoreNode.hh"
#include "KTTimeSeriesData.hh"
#include "KTTimeSeriesFFTW.hh"
#include "KTTimeSeriesReal.hh"
#include "KTWindowFunction.hh"

using std::string;


namespace Katydid
{
    KTLOGGER(windowlog, "katydid.fft");

    static KTDerivedNORegistrar< KTProcessor, KTWindower > sWindowerRegistrar("windower");

    KTWindower::KTWindower(const std::string& name) :
            KTProcessor(name),
            fWindowFunction(NULL),
            fWindowed("windowed", this),
            fHeaderSlot("header", this, &KTWindower::InitializeWithHeader),
            fTimeSeriesFFTWSlot("ts-fftw", this, &KTWindower::WindowDataFFTW, &fWindowed),
            fTimeSeriesRealSlot("ts-real", this, &KTWindower::WindowDataReal, &fWindowed)
    {
    }

    KTWindower::~KTWindower()
    {
        delete fWindowFunction;
    }

    Bool_t KTWindower::Configure(const KTPStoreNode* node)
    {
        // Config-file settings
        if (node != NULL)
        {
            string windowType = node->GetData< string >("window-function-type", "rectangular");
            if (! SelectWindowFunction(windowType))
            {
                return false;
            }

            KTPStoreNode wfNode = node->GetChild("window-function");
            if (! wfNode.IsValid())
            {
                KTWARN(windowlog, "No PStoreNode found for the window function");
            }
            else
            {
                if (! fWindowFunction->Configure(&wfNode))
                {
                    KTERROR(windowlog, "Problems occurred while configuring the window function");
                    return false;
                }
            }
        }

        return true;
    }

    Bool_t KTWindower::SelectWindowFunction(const string& windowType)
    {
        KTWindowFunction* tempWF = KTNOFactory< KTWindowFunction >::GetInstance()->Create(windowType);
        if (tempWF == NULL)
        {
            KTERROR(windowlog, "Invalid window function type given: <" << windowType << ">.");
            return false;
        }
        SetWindowFunction(tempWF);
        return true;
    }

    Bool_t KTWindower::InitializeWindow(double binWidth, double size)
    {
        fWindowFunction->SetBinWidth(binWidth);
        fWindowFunction->SetSize(size);
        fWindowFunction->RebuildWindowFunction();
        return true;
    }

    void KTWindower::InitializeWithHeader(const KTEggHeader* header)
    {
        if (! InitializeWindow(1. / header->GetAcquisitionRate(), header->GetSliceSize()))
        {
            KTERROR(windowlog, "Something went wrong while initializing the window function!");
            return;
        }
        KTDEBUG(windowlog, "Window function initialized with header");
        return;
    }

    Bool_t KTWindower::WindowDataReal(KTTimeSeriesData& tsData)
    {
        if (tsData.GetTimeSeries(0)->GetNTimeBins() != fWindowFunction->GetSize())
        {
            fWindowFunction->AdaptTo(&tsData); // this call rebuilds the window, so that doesn't need to be done separately
        }

        UInt_t nComponents = tsData.GetNComponents();

        for (UInt_t iComponent = 0; iComponent < nComponents; ++iComponent)
        {
            KTTimeSeriesReal* nextInput = dynamic_cast< KTTimeSeriesReal* >(tsData.GetTimeSeries(iComponent));
            if (nextInput == NULL)
            {
                KTERROR(windowlog, "Incorrect time series type: time series did not cast to KTTimeSeriesFFTW.");
                return false;
            }

            Bool_t result = ApplyWindow(nextInput);

            if (! result)
            {
                KTERROR(windowlog, "Component <" << iComponent << "> did not get windowed correctly.");
                return false;
            }
        }

        KTINFO(windowlog, "Windowing complete");

        return true;
    }

    Bool_t KTWindower::WindowDataFFTW(KTTimeSeriesData& tsData)
    {
        if (tsData.GetTimeSeries(0)->GetNTimeBins() != fWindowFunction->GetSize())
        {
            fWindowFunction->AdaptTo(&tsData); // this call rebuilds the window, so that doesn't need to be done separately
        }

        UInt_t nComponents = tsData.GetNComponents();

        for (UInt_t iComponent = 0; iComponent < nComponents; ++iComponent)
        {
            KTTimeSeriesFFTW* nextInput = dynamic_cast< KTTimeSeriesFFTW* >(tsData.GetTimeSeries(iComponent));
            if (nextInput == NULL)
            {
                KTERROR(windowlog, "Incorrect time series type: time series did not cast to KTTimeSeriesFFTW.");
                return false;
            }

            Bool_t result = ApplyWindow(nextInput);

            if (! result)
            {
                KTERROR(windowlog, "Component <" << iComponent << "> did not get windowed correctly.");
                return false;
            }
        }

        KTINFO(windowlog, "Windowing complete");

        return true;
    }

    Bool_t KTWindower::ApplyWindow(KTTimeSeriesReal* ts) const
    {
        UInt_t nBins = ts->size();
        if (nBins != fWindowFunction->GetSize())
        {
            KTWARN(windowlog, "Number of bins in the data provided does not match the number of bins set for this window\n"
                    << "   Bin expected: " << fWindowFunction->GetSize() << ";   Bins in data: " << nBins);
            return false;
        }

        for (UInt_t iBin=0; iBin < nBins; ++iBin)
        {
            (*ts)(iBin) = (*ts)(iBin) * fWindowFunction->GetWeight(iBin);
        }

        return true;
    }

    Bool_t KTWindower::ApplyWindow(KTTimeSeriesFFTW* ts) const
    {
        UInt_t nBins = ts->size();
        if (nBins != fWindowFunction->GetSize())
        {
            KTWARN(windowlog, "Number of bins in the data provided does not match the number of bins set for this window\n"
                    << "   Bin expected: " << fWindowFunction->GetSize() << ";   Bins in data: " << nBins);
            return false;
        }

        double weight;
        for (UInt_t iBin=0; iBin < nBins; ++iBin)
        {
            weight = fWindowFunction->GetWeight(iBin);
            (*ts)(iBin)[0] = (*ts)(iBin)[0] * weight;
            (*ts)(iBin)[1] = (*ts)(iBin)[1] * weight;
        }

        return true;
    }

    void KTWindower::SetWindowFunction(KTWindowFunction* wf)
    {
        delete fWindowFunction;
        fWindowFunction = wf;
        return;
    }



} /* namespace Katydid */
