/**
 @file KTGainVariationProcessor.hh
 @brief Contains KTGainVariationProcessor
 @details Processes the gain variation.
 @author: N. S. Oblath
 @date: Dec 10, 2012
 */

#ifndef KTGAINVARIATIONPROCESSOR_HH_
#define KTGAINVARIATIONPROCESSOR_HH_

#include "KTProcessor.hh"

#include "KTPhysicalArray.hh"
#include "KTSlot.hh"

#include <boost/shared_ptr.hpp>


namespace Katydid
{
    class KTData;
    class KTFrequencySpectrumDataPolar;
    class KTFrequencySpectrumDataFFTW;
    class KTGainVariationData;
    class KTPStoreNode;
    //class KTSpline;

    /*!
     @class KTGainVariationProcessor
     @author N. S. Oblath

     @brief Fit the gain variation

     @details
     Fit the gain variation to a parabola using linear regression.
     The fit is performed between fMinBin and fMaxBin, inclusive.  If the [min,max] range has been set by frequency, those frequencies are turned into bins the first time they're used.

     Configuration name: "gain-variation"

     Available configuration values:
     - "min-frequency": double -- minimum frequency for the fit
     - "max-frequency": double -- maximum frequency for the fit
     - "min-bin": unsigned -- minimum bin for the fit
     - "max-bin": unsigned -- maximum bin for the fit

     Slots:
     - "fs-polar": void (shared_ptr< KTData >) -- Calculates gain variation on a polar fs data object; Requires KTFrequencySpectrumDataPolar; Adds KTGainVariationData
     - "fs-fftw": void (shared_ptr< KTData >) -- Calculates gain variation on a fftw fs data object; Requires KTFrequencySpectrumDataFFTW; Adds KTGainVariationData

     Signals:
     - "gain-var": void (shared_ptr< KTData >) emitted upon performance of a fit; Guarantees KTGainVariationData
    */

    class KTGainVariationProcessor : public KTProcessor
    {
        public:
            KTGainVariationProcessor(const std::string& name = "gain-variation");
            virtual ~KTGainVariationProcessor();

            Bool_t Configure(const KTPStoreNode* node);

            Double_t GetMinFrequency() const;
            void SetMinFrequency(Double_t freq);

            Double_t GetMaxFrequency() const;
            void SetMaxFrequency(Double_t freq);

            UInt_t GetMinBin() const;
            void SetMinBin(UInt_t bin);

            UInt_t GetMaxBin() const;
            void SetMaxBin(UInt_t bin);

            UInt_t GetNFitPoints() const;
            void SetNFitPoints(UInt_t nPoints);

        private:
            Double_t fMinFrequency;
            Double_t fMaxFrequency;
            UInt_t fMinBin;
            UInt_t fMaxBin;
            UInt_t fNFitPoints;
            Bool_t fCalculateMinBin;
            Bool_t fCalculateMaxBin;

        public:
            Bool_t CalculateGainVariation(KTFrequencySpectrumDataPolar& data);
            Bool_t CalculateGainVariation(KTFrequencySpectrumDataFFTW& data);

        private:
            //GainVariation* CreateGainVariation(KTSpline* spline, UInt_t nBins, Double_t rangeMin, Double_t rangeMax) const;

            //***************
            // Signals
            //***************

        private:
            KTSignalData fGainVarSignal;

            //***************
            // Slots
            //***************

        private:
            KTSlotDataOneType< KTFrequencySpectrumDataPolar > fFSPolarSlot;
            KTSlotDataOneType< KTFrequencySpectrumDataFFTW > fFSFFTWSlot;

    };

    inline Double_t KTGainVariationProcessor::GetMinFrequency() const
    {
        return fMinFrequency;
    }

    inline void KTGainVariationProcessor::SetMinFrequency(Double_t freq)
    {
        fMinFrequency = freq;
        fCalculateMinBin = true;
        return;
    }

    inline Double_t KTGainVariationProcessor::GetMaxFrequency() const
    {
        return fMaxFrequency;
    }

    inline void KTGainVariationProcessor::SetMaxFrequency(Double_t freq)
    {
        fMaxFrequency = freq;
        fCalculateMaxBin = true;
        return;
    }

    inline UInt_t KTGainVariationProcessor::GetMinBin() const
    {
        return fMinBin;
    }

    inline void KTGainVariationProcessor::SetMinBin(UInt_t bin)
    {
        fMinBin = bin;
        fCalculateMinBin = false;
        return;
    }

    inline UInt_t KTGainVariationProcessor::GetMaxBin() const
    {
        return fMaxBin;
    }

    inline void KTGainVariationProcessor::SetMaxBin(UInt_t bin)
    {
        fMaxBin = bin;
        fCalculateMaxBin = false;
        return;
    }

    inline UInt_t KTGainVariationProcessor::GetNFitPoints() const
    {
        return fNFitPoints;
    }

    inline void KTGainVariationProcessor::SetNFitPoints(UInt_t nPoints)
    {
        fNFitPoints = nPoints;
    }

} /* namespace Katydid */
#endif /* KTGAINVARIATIONPROCESSOR_HH_ */