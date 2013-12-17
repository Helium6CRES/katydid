/*
 * KTFrequencyCandidate.hh
 *
 *  Created on: Dec 18, 2012
 *      Author: nsoblath
 */

#ifndef KTFREQUENCYCANDIDATE_HH_
#define KTFREQUENCYCANDIDATE_HH_

#include "Rtypes.h"

namespace Katydid
{

    class KTFrequencyCandidate
    {
        public:
            KTFrequencyCandidate();
            KTFrequencyCandidate(const KTFrequencyCandidate& orig);
            virtual ~KTFrequencyCandidate();

            KTFrequencyCandidate& operator=(const KTFrequencyCandidate& rhs);

            UInt_t GetFirstBin() const;
            void SetFirstBin(UInt_t bin);

            UInt_t GetLastBin() const;
            void SetLastBin(UInt_t bin);

            double GetMeanFrequency() const;
            void SetMeanFrequency(double freq);

            double GetPeakAmplitude() const;
            void SetPeakAmplitude(double amp);

            double GetAmplitudeSum() const;
            void SetAmplitudeSum(double amp);

        protected:
            UInt_t fFirstBin;
            UInt_t fLastBin;
            double fMeanFrequency;
            double fPeakAmplitude;
            double fAmplitudeSum;
    };

    inline UInt_t KTFrequencyCandidate::GetFirstBin() const
    {
        return fFirstBin;
    }

    inline void KTFrequencyCandidate::SetFirstBin(UInt_t bin)
    {
        fFirstBin = bin;
        return;
    }

    inline UInt_t KTFrequencyCandidate::GetLastBin() const
    {
        return fLastBin;
    }

    inline void KTFrequencyCandidate::SetLastBin(UInt_t bin)
    {
        fLastBin = bin;
        return;
    }

    inline double KTFrequencyCandidate::GetMeanFrequency() const
    {
        return fMeanFrequency;
    }

    inline void KTFrequencyCandidate::SetMeanFrequency(double freq)
    {
        fMeanFrequency = freq;
        return;
    }

    inline double KTFrequencyCandidate::GetPeakAmplitude() const
    {
        return fPeakAmplitude;
    }

    inline void KTFrequencyCandidate::SetPeakAmplitude(double amp)
    {
        fPeakAmplitude = amp;
        return;
    }

    inline double KTFrequencyCandidate::GetAmplitudeSum() const
    {
        return fAmplitudeSum;
    }

    inline void KTFrequencyCandidate::SetAmplitudeSum(double amp)
    {
        fAmplitudeSum = amp;
        return;
    }


} /* namespace Katydid */
#endif /* KTFREQUENCYCANDIDATE_HH_ */
