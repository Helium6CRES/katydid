/*
 * KTEggReader2011.hh
 *
 *  Created on: Aug 20, 2012
 *      Author: nsoblath
 */

#ifndef KTEGGREADER2011_HH_
#define KTEGGREADER2011_HH_

#include "KTEggReader.hh"

#include <fstream>
#include <sstream>

namespace Katydid
{

    class KTEggReader2011 : public KTEggReader
    {
        private:
        public:
            struct HeaderInfo
            {
                int fSliceSize;
                int fFrameIDSize;
                int fRecordSize;
                int fTimeStampSize;
                double fRunLength;
                double fSampleRate;
                double fHertzPerSampleRateUnit;
                double fSecondsPerRunLengthUnit;

                HeaderInfo() :
                    fSliceSize(0),
                    fFrameIDSize(0),
                    fRecordSize(0),
                    fTimeStampSize(0),
                    fRunLength(0.),
                    fSampleRate(0.),
                    fHertzPerSampleRateUnit(1.),
                    fSecondsPerRunLengthUnit(1.)
                {}
            };

        public:
            KTEggReader2011();
            virtual ~KTEggReader2011();

        public:
            virtual KTEggHeader* BreakEgg(const std::string& filename);
            virtual boost::shared_ptr< KTData > HatchNextSlice();
            virtual bool CloseEgg();

            /// Returns the time since the run started in seconds
            Double_t GetTimeInRun() const;
            virtual Double_t GetIntegratedTime() const;

            virtual UInt_t GetNSlicesProcessed() const ;
            virtual UInt_t GetNRecordsProcessed() const;

        private:
            template< typename XReturnType, typename XArrayType >
            XReturnType ConvertFromArray(XArrayType* value);

            std::string fFileName;
            std::ifstream fEggStream;
            std::string fPrelude;
            unsigned fHeaderSize;
            std::string fHeader;

            HeaderInfo fHeaderInfo;

            UInt_t fRecordsRead;

            UInt_t fLastFrameID;

            static const std::ifstream::pos_type sPreludeSize;  // the prelude size is currently restricted to eight bytes

    };

    template< typename XReturnType, typename XArrayType >
    XReturnType KTEggReader2011::ConvertFromArray(XArrayType* value)
    {
        std::stringstream converter;
        XReturnType converted;
        converter << value;
        converter >> converted;
        return converted;
    }

    inline Double_t KTEggReader2011::GetTimeInRun() const
    {
        return Double_t(fRecordsRead * fHeaderInfo.fRecordSize) / fHeaderInfo.fSampleRate;
    }

    inline Double_t KTEggReader2011::GetIntegratedTime() const
    {
        return GetTimeInRun();
    }

    inline UInt_t KTEggReader2011::GetNSlicesProcessed() const
    {
        return fRecordsRead;
    }

    inline UInt_t KTEggReader2011::GetNRecordsProcessed() const
    {
        return fRecordsRead;
    }


} /* namespace Katydid */
#endif /* KTEGGREADER2011_HH_ */
