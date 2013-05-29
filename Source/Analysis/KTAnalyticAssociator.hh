/**
 @file KTAnalyticAssociator.hh
 @brief Contains KTAnalyticAssociator
 @details Creates an analytic associate of a time series
 @author: N. S. Oblath
 @date: Dec 17, 2012
 */

#ifndef KTANALYTICASSOCIATOR_HH_
#define KTANALYTICASSOCIATOR_HH_

#include "KTProcessor.hh"
#include "KTTimeSeriesData.hh"

#include "KTComplexFFTW.hh"
#include "KTSlot.hh"

#include <boost/shared_ptr.hpp>


namespace Katydid
{
    class KTEggHeader;
    class KTData;
    class KTFrequencySpectrumDataFFTW;
    class KTFrequencySpectrumFFTW;
    class KTNormalizedFSDataFFTW;
    class KTTimeSeriesFFTW;

    /*!
     @class KTAnalyticAssociator
     @author N. S. Oblath

     @brief Creates an analytic associate of a time series

     @details

     Configuration name: "analytic-associator"
 
     Available configuration values:
     - "save-frequency-spectrum": bool -- Option to save the intermediate frequency spectrum that is calculated while creating the analytic associate
     - "complex-fftw": nested config: -- See KTComplexFFTW

     Slots:
     - "header": void (const KTEggHeader*) -- Initializes the FFT
     - "ts": void (shared_ptr< KTData >) -- Calculates an analytic associate of the time series; Requires KTTimeSeriesData; Adds KTAnalyticAssociateData; Optionally adds KTFrequencySpectrumDataFFTW
     - "fs-fftw": void (shared_ptr< KTData >) -- Calculates an analytic associate of the frequency spectrum; Requires KTFrequencySpectrumDataFFTW; Adds KTAnalyticAssociateData
     - "norm-fs-fftw": void (shared_ptr< KTData >) -- Calculates an analytic associate of the frequency spectrum; Requires KTNormalizedFSDataFFTW; Adds KTAnalyticAssociateData

     Signals:
     - "aa": void (shared_ptr< KTData >) -- Emitted upon creation of an analytic associate; Guarantees KTAnalyticAssociateData
    */
    class KTAnalyticAssociator : public KTProcessor
    {
        public:
            KTAnalyticAssociator(const std::string& name = "analytic-associator");
            virtual ~KTAnalyticAssociator();

            Bool_t Configure(const KTPStoreNode* node);

            void InitializeWithHeader(const KTEggHeader* header);

            KTComplexFFTW* GetFullFFT();

            Bool_t GetSaveFrequencySpectrum() const;
            void SetSaveFrequencySpectrum(Bool_t flag);

        private:
            KTComplexFFTW fFullFFT;

            Bool_t fSaveFrequencySpectrum;

        public:
            Bool_t CreateAssociateData(KTTimeSeriesData& tsData);
            Bool_t CreateAssociateData(KTFrequencySpectrumDataFFTW& fsData);
            Bool_t CreateAssociateData(KTNormalizedFSDataFFTW& fsData);

           /// Calculates the AA and returns the new time series; the intermediate FS is assigned to the given output pointer.
            KTTimeSeriesFFTW* CalculateAnalyticAssociate(const KTTimeSeriesFFTW* inputTS, KTFrequencySpectrumFFTW** outputFS=NULL);
            KTTimeSeriesFFTW* CalculateAnalyticAssociate(const KTFrequencySpectrumFFTW* inputFS);

            //***************
            // Signals
            //***************

         private:
             KTSignalData fAASignal;

             //***************
             // Slots
             //***************

         private:
             KTSlotOneArg< void (const KTEggHeader*) > fHeaderSlot;
             KTSlotDataOneType< KTTimeSeriesData > fTimeSeriesSlot;
             KTSlotDataOneType< KTFrequencySpectrumDataFFTW > fFSFFTWSlot;
             KTSlotDataOneType< KTNormalizedFSDataFFTW > fNormFSFFTWSlot;

    };

    inline KTComplexFFTW* KTAnalyticAssociator::GetFullFFT()
    {
        return &fFullFFT;
    }

    inline Bool_t KTAnalyticAssociator::GetSaveFrequencySpectrum() const
    {
        return fSaveFrequencySpectrum;
    }

    inline void KTAnalyticAssociator::SetSaveFrequencySpectrum(Bool_t flag)
    {
        fSaveFrequencySpectrum = flag;
        return;
    }

} /* namespace Katydid */
#endif /* KTANALYTICASSOCIATOR_HH_ */