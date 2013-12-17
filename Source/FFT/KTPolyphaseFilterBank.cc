/*
 * KTPolyphaseFilterBank.cc
 *
 *  Created on: Apr 18, 2013
 *      Author: nsoblath
 */

#include "KTPolyphaseFilterBank.hh"

#include "KTEggHeader.hh"
#include "KTNOFactory.hh"
#include "KTLogger.hh"
#include "KTPStoreNode.hh"
#include "KTSliceHeader.hh"
#include "KTTimeSeriesData.hh"
#include "KTTimeSeriesFFTW.hh"
#include "KTTimeSeriesReal.hh"
#include "KTWindowFunction.hh"

using std::string;


namespace Katydid
{
    KTLOGGER(pfblog, "katydid.fft");

    static KTDerivedNORegistrar< KTProcessor, KTPolyphaseFilterBank > sPFBRegistrar("polyphase-filter-bank");

    KTPolyphaseFilterBank::KTPolyphaseFilterBank(const std::string& name) :
            KTProcessor(name),
            fNSubsets(2),
            fSubsetSize(1),
            fWindowed("windowed", this),
            fHeaderSlot("header", this, &KTPolyphaseFilterBank::InitializeWithHeader),
            fTimeSeriesRealSlot("ts", this, &KTPolyphaseFilterBank::WindowDataReal, &fWindowed),
            fTimeSeriesFFTWSlot("ts", this, &KTPolyphaseFilterBank::WindowDataFFTW, &fWindowed)
    {
    }

    KTPolyphaseFilterBank::~KTPolyphaseFilterBank()
    {
    }

    Bool_t KTPolyphaseFilterBank::Configure(const KTPStoreNode* node)
    {
        // Config-file settings
        if (node == NULL)
        {
            return false;
        }

        if (node->HasData("n-subsets"))
        {
            SetNSubsets(node->GetData("n-subsets", fNSubsets));
        }
        if (node->HasData("subset-size"))
        {
            SetSubsetSize(node->GetData("subset-size", fSubsetSize));
        }

        return true;
    }

    Bool_t KTPolyphaseFilterBank::ProcessDataReal(const KTTimeSeriesData& tsData)
    {
        if (dynamic_cast< KTTimeSeriesReal* >(tsData.GetTimeSeries(0)) == NULL)
        {
            KTERROR(pfblog, "Data does not contain real-type time series.");
            return false;
        }

        KTTimeSeriesData* newData = CreateFilteredDataReal(tsData);
        if (newData != NULL)
        {
            return true;
        }
        KTERROR(pfblog, "New data object was not created correctly (real)");
        return false;
    }

    Bool_t KTPolyphaseFilterBank::ProcessDataFFTW(const KTTimeSeriesData& tsData)
    {
        if (dynamic_cast< KTTimeSeriesFFTW* >(tsData.GetTimeSeries(0)) == NULL)
        {
            KTERROR(pfblog, "Data does not contain fftw-type time series.");
            return false;
        }

        KTTimeSeriesData* newData = CreateFilteredDataFFTW(tsData);
        if (newData != NULL)
        {
            return true;
        }
        KTERROR(pfblog, "New data object was not created correctly (fftw)");
        return false;
    }

    KTDataPtr KTPolyphaseFilterBank::CreateFilteredDataReal(const KTTimeSeriesData& tsData)
    {
        KTSliceHeader& oldSliceHeader = tsData.Of< KTSliceHeader >();

        KTDataPtr newData(new KTData());

        // Fill out slice header information
        KTSliceHeader& sliceHeader = newData->Of< KTSliceHeader >();
        if (! TransferHeaderInformation(oldSliceHeader, sliceHeader))
        {
            KTERROR(pfblog, "Header information was not transferred");
            return KTDataPtr();
        }
        KTDEBUG(pfblog, "Filled out slice header:\n"
                << "\tSample rate: " << sliceHeader.GetSampleRate() << " Hz\n"
                << "\tSlice size: " << sliceHeader.GetSliceSize() << '\n'
                << "\tBin width: " << sliceHeader.GetBinWidth() << " s\n"
                << "\tSlice length: " << sliceHeader.GetSliceLength() << " s\n"
                << "\tTime in run: " << sliceHeader.GetTimeInRun() << " s\n"
                << "\tSlice number: " << sliceHeader.GetSliceNumber());


        unsigned nComponents = tsData.GetNComponents();
        KTTimeSeriesData& tsData = newData->Of< KTTimeSeriesData >().SetNComponents(nComponents);

        for (unsigned iComponent = 0; iComponent < nComponents; iComponent++)
        {
            KTTimeSeries* newTS = ApplyPFB(static_cast< const KTTimeSeriesReal* >(tsData.GetTimeSeries(iComponent)));
            if (newTS == NULL)
            {
                KTERROR(pfblog, "Time series for component " << iComponent << " was not created!");
                return KTDataPtr();
            }
            tsData.SetTimeSeries(newTS, iComponent);
        }

        return newData;
    }

    KTDataPtr KTPolyphaseFilterBank::CreateFilteredDataFFTW(const KTTimeSeriesData& tsData)
    {
        unsigned nComponents = tsData.GetNComponents();

        for (unsigned iComponent = 0; iComponent < nComponents; iComponent++)
        {
            KTTimeSeriesFFTW* newTS = ApplyPFB(static_cast< const KTTimeSeriesFFTW* >(tsData.GetTimeSeries(iComponent)));
        }
    }

    KTTimeSeriesReal* KTPolyphaseFilterBank::ApplyPFB(const KTTimeSeriesReal* data) const
    {

    }

    KTTimeSeriesFFTW* KTPolyphaseFilterBank::ApplyPFB(const KTTimeSeriesFFTW* data) const
    {

    }

    Bool_t KTPolyphaseFilterBank::TransferHeaderInformation(const KTSliceHeader& oldHeader, KTSliceHeader& newHeader)
    {
        newHeader.SetNComponents(oldHeader.GetNComponents());
        newHeader.SetSampleRate(oldHeader.GetSampleRate());
        newHeader.SetSliceSize(fSubsetSize);
        newHeader.CalculateBinWidthAndSliceLength();
        newHeader.SetTimeInRun(GetNewTimeInRun(oldHeader));
        newHeader.SetSliceNumber(GetNewSliceNumber(oldHeader));

        unsigned nComponents = newHeader.GetNComponents();
        for (unsigned iComponent = 0; iComponent < nComponents; iComponent++)
        {
            newHeader.SetAcquisitionID(oldHeader.GetAcquisitionID(iComponent), iComponent);
            newHeader.SetRecordID(oldHeader.GetRecordID(iComponent), iComponent);
            newHeader.SetTimeStamp(oldHeader.GetTimeStamp(iComponent), iComponent);
        }

        return true;
    }


} /* namespace Katydid */
