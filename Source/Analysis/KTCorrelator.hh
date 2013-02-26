/**
 @file KTCorrelator.hh
 @brief Contains KTCorrelator
 @details Correlates frequency spectra from different channels
 @author: N. S. Oblath
 @date: Aug  20, 2012
 */

#ifndef KTCORRELATOR_HH_
#define KTCORRELATOR_HH_

#include "KTFrequencySpectrumDataPolar.hh"
#include "KTProcessor.hh"

#include <boost/shared_ptr.hpp>

#include <utility>
#include <vector>

namespace Katydid
{
    class KTCorrelationData : public KTFrequencySpectrumDataPolarCore, public KTExtensibleData< KTCorrelationData >
    {
        public:
            KTCorrelationData() :
                    KTFrequencySpectrumDataPolarCore(),
                    KTExtensibleData< KTCorrelationData >()
            {}
            virtual ~KTCorrelationData()
            {}

            inline const std::pair< UInt_t, UInt_t >& GetInputPair(UInt_t component = 0) const
            {
                return fComponentData[component];
            }

            inline void SetInputPair(UInt_t first, UInt_t second, UInt_t component = 0)
            {
                if (component >= fSpectra.size()) SetNComponents(component+1);
                fComponentData[component].first = first;
                fComponentData[component].second = second;
                return;
            }

            inline virtual KTCorrelationData& SetNComponents(UInt_t components)
            {
                fSpectra.resize(components);
                fComponentData.resize(components);
                return *this;
            }

        protected:
            std::vector< std::pair< UInt_t, UInt_t > > fComponentData;
    };



    class KTFrequencySpectrumDataFFTW;
    class KTFrequencySpectrumDataFFTWCore;
    class KTFrequencySpectrumFFTW;
    class KTNormalizedFSDataPolar;
    class KTNormalizedFSDataFFTW;

    typedef std::pair< UInt_t, UInt_t > KTCorrelationPair;


    /*!
     @class KTCorrelator
     @author N. S. Oblath

     @brief Correlates frequencey spectra from different channels.

     @details
     
     Available configuration values:
     \li \c "corr-pair": string -- channel pair to be correlated: "[first channel], [second channel]"; e.g. "0, 0" or "0, 1"
     \li \c "input-data-name": string -- name of the data to find when processing an event
     \li \c "output-data-name": string -- name to give to the data produced by a correlation

      Slots:
     \li \c "event": void ProcessEvent(boost::shared_ptr<KTEvent>)
     \li \c "fft-data": void ProcessTimeSeriesData(const KTTimeSeriesDataReal*)

     Signals:
     \li \c "correlation": void (const KTWriteableData*) emitted upon performance of a correlation.
    */



    class KTCorrelator : public KTProcessor
    {
        protected:
            typedef KTSignalConcept< void (boost::shared_ptr<KTData>) >::signal CorrelationSignal;
            typedef std::vector< KTCorrelationPair > PairVector;

        public:
            KTCorrelator();
            virtual ~KTCorrelator();

            Bool_t Configure(const KTPStoreNode* node);

            void AddPair(const KTCorrelationPair& pair);
            void SetPairVector(const PairVector& pairs);
            const PairVector& GetPairVector() const;
            void ClearPairs();

        protected:
            PairVector fPairs;

        public:

            Bool_t Correlate(KTFrequencySpectrumDataPolar& data);
            Bool_t Correlate(KTFrequencySpectrumDataFFTW& data);
            Bool_t Correlate(KTNormalizedFSDataPolar& data);
            Bool_t Correlate(KTNormalizedFSDataFFTW& data);

        protected:
            Bool_t CoreCorrelate(KTFrequencySpectrumDataPolarCore& data, KTCorrelationData& newData);
            Bool_t CoreCorrelate(KTFrequencySpectrumDataFFTWCore& data, KTCorrelationData& newData);

            KTFrequencySpectrumPolar* DoCorrelation(const KTFrequencySpectrumPolar* firstSpectrum, const KTFrequencySpectrumPolar* secondSpectrum);
            KTFrequencySpectrumPolar* DoCorrelation(const KTFrequencySpectrumFFTW* firstSpectrum, const KTFrequencySpectrumFFTW* secondSpectrum);

            //***************
            // Signals
            //***************

        private:
            CorrelationSignal fCorrSignal;

            //***************
            // Slots
            //***************

        public:
            void ProcessPolarData(boost::shared_ptr< KTData > data);
            void ProcessFFTWData(boost::shared_ptr< KTData > data);
            void ProcessNormalizedPolarData(boost::shared_ptr< KTData > data);
            void ProcessNormalizedFFTWData(boost::shared_ptr< KTData > data);
    };

    inline void KTCorrelator::AddPair(const KTCorrelationPair& pair)
    {
        fPairs.push_back(pair);
        return;
    }

    inline void KTCorrelator::SetPairVector(const PairVector& pairs)
    {
        fPairs = pairs;
        return;
    }

    inline const KTCorrelator::PairVector& KTCorrelator::GetPairVector() const
    {
        return fPairs;
    }

    inline void KTCorrelator::ClearPairs()
    {
        fPairs.clear();
        return;
    }

} /* namespace Katydid */
#endif /* KTCORRELATOR_HH_ */
