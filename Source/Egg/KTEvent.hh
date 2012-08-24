/**
 @file KTEvent.hh
 @brief Contains KTEvent
 @details Contains the information from a single Egg event in the form of a 1-D std::vector of unsigned integers.
 The data are the time series of the event.
 @author: N. S. Oblath
 @date: Sep 9, 2011
 */

#ifndef KTEVENT_HH_
#define KTEVENT_HH_

#include "KTAbstractEvent.hh"

#include "MonarchTypes.hpp"

#include <algorithm>
#include <vector>

#ifdef ROOT_FOUND
class TH1C;
class TH1I;
#endif

namespace Katydid
{
    class KTEvent : public KTAbstractEvent
    {
        public:
            KTEvent(unsigned nChannels=1);
            virtual ~KTEvent();

#ifdef ROOT_FOUND
            virtual TH1C* CreateEventHistogram(unsigned channelNum = 0) const;
            virtual TH1I* CreateAmplitudeDistributionHistogram(unsigned channelNum = 0) const;
#endif

            unsigned GetNRecords() const;

            unsigned GetRecordSize() const;
            double GetSampleRate() const;
            double GetRecordLength() const;
            double GetBinWidth() const;

            ClockType GetTimeStamp(unsigned channelNum = 0) const;
            ChIdType GetChannelID(unsigned channelNum = 0) const;
            AcqIdType GetAcquisitionID(unsigned channelNum = 0) const;
            RecIdType GetRecordID(unsigned channelNum = 0) const;

            const std::vector< DataType >& GetRecord(unsigned channelNum = 0) const;

            //unsigned GetRecordSize(unsigned channelNum = 0) const;
            DataType GetRecordAt(unsigned int iBin, unsigned channelNum = 0) const;
            template< typename XType >
            XType GetRecordAt(unsigned int iBin, unsigned channelNum = 0) const;
            DataType GetRecordAtTime(double time, unsigned channelNum = 0) const; /// time is in seconds and >= 0
            template< typename XType >
            XType GetRecordAtTime(double time, unsigned channelNum = 0) const; /// time is in seconds and >= 0

            void SetNChannels(unsigned channels);

            void SetRecordSize(unsigned size);
            void SetSampleRate(double sampleRate);
            void SetRecordLength(double recordLength);
            void SetBinWidth(double binWidth);
            void CalculateBinWidthAndRecordLength();

            void SetTimeStamp(ClockType timeStamp, unsigned channelNum = 0);
            void SetChannelID(ChIdType chId, unsigned channelNum = 0);
            void SetAcquisitionID(AcqIdType acqId, unsigned channelNum = 0);
            void SetRecordID(RecIdType recId, unsigned channelNum = 0);

            void SetRecord(std::vector< DataType >* record, unsigned channelNum = 0);

        private:
            unsigned fRecordSize; // number of bins
            double fSampleRate; // in Hz
            double fRecordLength; // in sec
            double fBinWidth; // in sec

            std::vector< PerChannelData > fChannelData;
            /*
        private:
            /// Round to nearest integer. Rounds half integers to the nearest even integer.
            /// Based on ROOT's TMath::Nint(Double_t)
            int nint(double x) const;
            */
    };

    template< typename XType >
    XType KTEvent::GetRecordAt(unsigned iPoint, unsigned channelNum) const
    {
        return (XType)GetRecordAt(iPoint, channelNum);
    }

    template< typename XType >
    XType KTEvent::GetRecordAtTime(double time, unsigned channelNum) const
    {
        return this->GetRecordAt< XType >((unsigned)(nint(std::max(0., time) / GetBinWidth())), channelNum);
    }

    inline unsigned KTEvent::GetNRecords() const
    {
        return unsigned(fChannelData.size());
    }

    inline DataType KTEvent::GetRecordAt(unsigned int iPoint, unsigned channelNum) const
    {
        return fChannelData[channelNum].fRecord->at(iPoint);
    }

    inline DataType KTEvent::GetRecordAtTime(double time, unsigned channelNum) const
    {
        return this->GetRecordAt((unsigned)(nint(std::max(0., time) / fBinWidth)), channelNum);
    }
    /*
    template< typename XType >
    XType KTEvent::GetRecordAt(unsigned iPoint, unsigned channelNum) const
    {
        return (XType)GetRecordAt(iPoint, channelNum);
    }

    template< typename XType >
    XType KTEvent::GetRecordAtTime(double time, unsigned channelNum) const
    {
        return this->GetRecordAt< XType >((unsigned)(nint(std::max(0., time) / fBinWidth)), channelNum);
    }
    */
    inline unsigned KTEvent::GetRecordSize() const
    {
        return fRecordSize;
    }

    inline double KTEvent::GetRecordLength() const
    {
        return fRecordLength;
    }

    inline double KTEvent::GetSampleRate() const
    {
        return fSampleRate;
    }

    inline double KTEvent::GetBinWidth() const
    {
        return fBinWidth;
    }

    inline ClockType KTEvent::GetTimeStamp(unsigned channelNum) const
    {
        return fChannelData[channelNum].fTimeStamp;
    }

    inline ChIdType KTEvent::GetChannelID(unsigned channelNum) const
    {
        return fChannelData[channelNum].fChannelID;
    }

    inline AcqIdType KTEvent::GetAcquisitionID(unsigned channelNum) const
    {
        return fChannelData[channelNum].fAcquisitionID;
    }

    inline RecIdType KTEvent::GetRecordID(unsigned channelNum) const
    {
        return fChannelData[channelNum].fRecordID;
    }

    inline const std::vector< DataType >& KTEvent::GetRecord(unsigned channelNum) const
    {
        return (*fChannelData[channelNum].fRecord);
    }

    inline void KTEvent::SetRecordSize(unsigned recordSize)
    {
        this->fRecordSize = recordSize;
    }

    inline void KTEvent::SetRecordLength(double recordLength)
    {
        this->fRecordLength = recordLength;
    }

    inline void KTEvent::SetSampleRate(double sampleRate)
    {
        this->fSampleRate = sampleRate;
    }

    inline void KTEvent::SetBinWidth(double binWidth)
    {
        this->fBinWidth = binWidth;
    }

    inline void KTEvent::CalculateBinWidthAndRecordLength()
    {
        SetBinWidth(1. / fSampleRate);
        SetRecordLength(double(fRecordSize) * fBinWidth);
        return;
    }

    inline void KTEvent::SetNChannels(unsigned channels)
    {
        fChannelData.resize(channels);
        return;
    }

    inline void KTEvent::SetTimeStamp(ClockType timeStamp, unsigned channelNum)
    {
        if (channelNum >= fChannelData.size()) fChannelData.resize(channelNum+1);
        fChannelData[channelNum].fTimeStamp = timeStamp;
        return;
    }

    inline void KTEvent::SetChannelID(ChIdType chId, unsigned channelNum)
    {
        if (channelNum >= fChannelData.size()) fChannelData.resize(channelNum+1);
        fChannelData[channelNum].fChannelID = chId;
        return;
    }

    inline void KTEvent::SetAcquisitionID(AcqIdType acqId, unsigned channelNum)
    {
        if (channelNum >= fChannelData.size()) fChannelData.resize(channelNum+1);
        fChannelData[channelNum].fAcquisitionID = acqId;
        return;
    }

    inline void KTEvent::SetRecordID(RecIdType recId, unsigned channelNum)
    {
        if (channelNum >= fChannelData.size()) fChannelData.resize(channelNum+1);
        fChannelData[channelNum].fRecordID = recId;
        return;
    }

    inline void KTEvent::SetRecord(std::vector< DataType >* record, unsigned channelNum)
    {
        if (channelNum >= fChannelData.size()) fChannelData.resize(channelNum+1);
        fChannelData[channelNum].fRecord = record;
    }


    /*
    inline int KTEvent::nint(double x) const
    {
        int i;
        if (x >= 0.)
        {
            i = int(x + 0.5);
            if (x + 0.5 == double(i) && (i & 1)) i--;
        }
        else
        {
            i = int(x - 0.5);
            if (x - 0.5 == double(i) && (i & 1)) i++;
        }
        return i;
    }
    */

} /* namespace Katydid */

#endif /* KTEVENT_HH_ */
