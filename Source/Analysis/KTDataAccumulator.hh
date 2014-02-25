/*
 * KTDataAccumulator.hh
 *
 *  Created on: Oct 21, 2013
 *      Author: nsoblath
 */

#ifndef KTDATAACCUMULATOR_HH_
#define KTDATAACCUMULATOR_HH_

#include "KTProcessor.hh"

#include "KTData.hh"
#include "KTLogger.hh"
#include "KTSlot.hh"
#include "KTSliceHeader.hh"
#include "KTTimeSeriesDistData.hh"

#include <map>
#include <typeinfo>

namespace Katydid
{
    KTLOGGER(avlog_hh, "KTDataAccumulator.hh");

    class KTFrequencySpectrumDataFFTW;
    class KTFrequencySpectrumDataFFTWCore;
    class KTFrequencySpectrumDataPolar;
    class KTFrequencySpectrumDataPolarCore;
    class KTPStoreNode;
    class KTTimeSeriesData;

    /*!
     @class KTDataAccumulator
     @author N. S. Oblath

     @brief Averages data objects

     @details


     Configuration name: "data-accumulator"

     Available configuration options:
     - "number-to-average": unsigned -- Number of slices to average
     - "signal-interval": unsigned -- Number of slices between signaling; set to 0 to stop slice signals

     Slots:
     - "ts": void (KTDataPtr) -- add to the ts sum; Requires KTTimeSeriesData; Emits signal "ts"
     - "ts-dist": void (KTDataPtr) -- add to the ts-dist sum; Requires KTTimeSeriesDistData; Emits signal "ts-dist"
     - "fs-polar": void (KTDataPtr) -- add to the fs-polar sum; Requires KTFrequencySpectrumPolar; Emits signal "fs-polar"
     - "fs-fftw": void (KTDataPtr) -- add to the fs-fftw sum; Requires KTFrequencySpectrumFFTW; Emits signal "fs-fftw"

     Signals:
     - "ts": void (KTDataPtr) -- emitted when the ts sum is updated; guarantees KTTimeSeriesData
     - "ts-dist": void (KTDataPtr) -- emitted when the ts-dist sum is updated; guarantees KTTimeSeriesDistData
     - "fs-polar": void (KTDataPtr) -- emitted when the fs-polar sum is updated; guarantees KTFrequencySpectrumDataPolar
     - "fs-fftw": void (KTDataPtr) -- emitted when the fs-fftw sum is updated; guarantees KTFrequencySpectrumDataFFTW
     - "ts-finished": void (KTDataPtr) -- emitted when the <finish> slot is called; guarantees KTTimeSeriesData
     - "ts-dist-finished": void (KTDataPtr) -- emitted when the <finish> slot is called; guarantees KTTimeSeriesDistData
     - "fs-polar-finished": void (KTDataPtr) -- emitted when the <finish> slot is called; guarantees KTFrequencySpectrumDataPolar
     - "fs-fftw-finished": void (KTDataPtr) -- emitted when the <finish> slot is called; guarantees KTFrequencySpectrumDataFFTW
    */

    class KTDataAccumulator : public KTProcessor
    {
        public:
            struct CompareTypeInfo
            {
                bool operator() (const std::type_info* lhs, const std::type_info* rhs)
                {
                    return lhs->before(*rhs);
                }
            };

            struct Accumulator
            {
                KTDataPtr fData;
                KTSliceHeader& fSliceHeader;

                void IncrementSlice();
                Accumulator() : fData(new KTData()), fSliceHeader(fData->Of<KTSliceHeader>())
                {
                }
                unsigned GetSliceNumber() const
                {
                    return fSliceHeader.GetSliceNumber();
                }
                void BumpSliceNumber()
                {
                    fSliceHeader.SetSliceNumber(fSliceHeader.GetSliceNumber() + 1);
                    return;
                }
            };

            typedef std::map< const std::type_info*, Accumulator > AccumulatorMap;
            typedef AccumulatorMap::iterator AccumulatorMapIt;

            struct SignalSet
            {
                    unsigned fSignalCount;
                    KTSignalData* fAccumulatingSignal;
                    KTSignalData* fFinishedSignal;
                    SignalSet(KTSignalData* accSig, KTSignalData* finishedSig) :
                        fSignalCount(0),
                        fAccumulatingSignal(accSig),
                        fFinishedSignal(finishedSig)
                    {}
            };
            typedef std::map< const std::type_info*, SignalSet > SignalMap;
            typedef SignalMap::iterator SignalMapIt;
            typedef SignalMap::value_type SignalMapValue;

        public:
            KTDataAccumulator(const std::string& name = "data-accumulator");
            virtual ~KTDataAccumulator();

            bool Configure(const KTPStoreNode* node);

            unsigned GetAccumulatorSize() const;
            double GetAveragingFrac() const;
            void SetAccumulatorSize(unsigned size);

            unsigned GetSignalInterval() const;
            void SetSignalInterval(unsigned interval);

        private:
            unsigned fAccumulatorSize;
            double fAveragingFrac;
            unsigned fSignalInterval;

        public:
            bool AddData(KTTimeSeriesData& data);
            bool AddData(KTTimeSeriesDistData& data);
            bool AddData(KTFrequencySpectrumDataPolar& data);
            bool AddData(KTFrequencySpectrumDataFFTW& data);

            const AccumulatorMap& GetAccumulators() const;
            template< class XDataType >
            const Accumulator& GetAccumulator() const;

        private:
            template< class XDataType >
            Accumulator& GetOrCreateAccumulator();

            bool CoreAddTSDataReal(KTTimeSeriesData& data, Accumulator& avDataStruct, KTTimeSeriesData& avData);
            bool CoreAddTSDataFFTW(KTTimeSeriesData& data, Accumulator& avDataStruct, KTTimeSeriesData& avData);

            bool CoreAddData(KTTimeSeriesDistData& data, Accumulator& avDataStruct, KTTimeSeriesDistData& avData);

            bool CoreAddData(KTFrequencySpectrumDataPolarCore& data, Accumulator& avDataStruct, KTFrequencySpectrumDataPolarCore& avData);
            bool CoreAddData(KTFrequencySpectrumDataFFTWCore& data, Accumulator& avDataStruct, KTFrequencySpectrumDataFFTWCore& avData);

            AccumulatorMap fDataMap;
            mutable Accumulator* fLastAccumulatorPtr;


            //***************
            // Signals
            //***************

        private:
            KTSignalData fTSSignal;
            KTSignalData fTSDistSignal;
            KTSignalData fFSPolarSignal;
            KTSignalData fFSFFTWSignal;

            KTSignalData fTSFinishedSignal;
            KTSignalData fTSDistFinishedSignal;
            KTSignalData fFSPolarFinishedSignal;
            KTSignalData fFSFFTWFinishedSignal;

            SignalMap fSignalMap;

            //***************
            // Slots
            //***************

        private:
            template< class XDataType >
            void SlotFunction(KTDataPtr data);

    };

    inline unsigned KTDataAccumulator::GetAccumulatorSize() const
    {
        return fAccumulatorSize;
    }

    inline double KTDataAccumulator::GetAveragingFrac() const
    {
        return fAveragingFrac;
    }

    inline void KTDataAccumulator::SetAccumulatorSize(unsigned size)
    {
        fAccumulatorSize = size;
        fAveragingFrac = 1. / (double)fAccumulatorSize;
        return;
    }

    inline unsigned KTDataAccumulator::GetSignalInterval() const
    {
        return fSignalInterval;
    }

    inline void KTDataAccumulator::SetSignalInterval(unsigned interval)
    {
        fSignalInterval = interval;
        return;
    }

    inline const KTDataAccumulator::AccumulatorMap& KTDataAccumulator::GetAccumulators() const
    {
        return fDataMap;
    }

    template< class XDataType >
    const KTDataAccumulator::Accumulator& KTDataAccumulator::GetAccumulator() const
    {
        fLastAccumulatorPtr = const_cast< Accumulator* >(&fDataMap.at(&typeid(XDataType)));
        return *fLastAccumulatorPtr;
    }

    template< class XDataType >
    KTDataAccumulator::Accumulator& KTDataAccumulator::GetOrCreateAccumulator()
    {
        fLastAccumulatorPtr = &fDataMap[&typeid(XDataType)];
        return *fLastAccumulatorPtr;
    }

    template< class XDataType >
    void KTDataAccumulator::SlotFunction(KTDataPtr data)
    {
        // Standard data slot pattern:
        // Check to ensure that the required data type is present
        if (! data->Has< XDataType >())
        {
            KTERROR(avlog_hh, "Data not found with type <" << typeid(XDataType).name() << ">");
            return;
        }
        // Call the function
        if (! AddData(data->Of< XDataType >()))
        {
            KTERROR(avlog_hh, "Something went wrong while analyzing data with type <" << typeid(XDataType).name() << ">");
            return;
        }
        // If there's a signal pointer, emit the signal
        SignalMapIt sigIt = fSignalMap.find(&typeid(XDataType));
        if (sigIt != fSignalMap.end())
        {
            sigIt->second.fSignalCount++;
            unsigned sigCount = sigIt->second.fSignalCount;
            if (sigCount == fSignalInterval)
            {
                (*sigIt->second.fAccumulatingSignal)(fLastAccumulatorPtr->fData);
                sigIt->second.fSignalCount = 0;
            }
            if (data->fLastData)
            {
                (*sigIt->second.fFinishedSignal)(fLastAccumulatorPtr->fData);
            }
        }
        return;
    }



} /* namespace Katydid */
#endif /* KTDATAACCUMULATOR_HH_ */
