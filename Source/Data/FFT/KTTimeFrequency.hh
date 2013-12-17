/*
 * KTTimeFrequency.hh
 *
 *  Created on: Feb 8, 2013
 *      Author: nsoblath
 */

#ifndef KTTIMEFREQUENCY_HH_
#define KTTIMEFREQUENCY_HH_

#include "Rtypes.h"

//#include <string>

#ifdef ROOT_FOUND
class TH2D;
#endif

namespace Katydid
{
    //class KTPowerSpectrum;

    class KTTimeFrequency
    {
        public:
            KTTimeFrequency();
            virtual ~KTTimeFrequency();

            /// Get the size of the array using the KTTimeFrequency interface
            virtual UInt_t GetNTimeBins() const = 0;
            virtual UInt_t GetNFrequencyBins() const = 0;

            virtual double GetTimeBinWidth() const = 0;
            virtual double GetFrequencyBinWidth() const = 0;

            virtual double GetReal(UInt_t timebin, UInt_t freqbin) const = 0;
            virtual double GetImag(UInt_t timebin, UInt_t freqbin) const = 0;

            virtual void SetRect(UInt_t timebin, UInt_t freqbin, double real, double imag) = 0;

            virtual double GetAbs(UInt_t timebin, UInt_t freqbin) const = 0;
            virtual double GetArg(UInt_t timebin, UInt_t freqbin) const = 0;

            virtual void SetPolar(UInt_t timebin, UInt_t freqbin, double abs, double arg) = 0;

            virtual KTTimeFrequency& CConjugate() = 0;

            //virtual KTPowerSpectrum* CreatePowerSpectrum() const= 0;

#ifdef ROOT_FOUND
        public:
            virtual TH2D* CreateMagnitudeHistogram(const std::string& name = "hTimeFrequencyMag") const = 0;
            virtual TH2D* CreatePhaseHistogram(const std::string& name = "hTimeFrequencyPhase") const = 0;

            virtual TH2D* CreatePowerHistogram(const std::string& name = "hFrequencySpectrumPower") const = 0;

            //virtual TH1D* CreatePowerDistributionHistogram(const std::string& name = "hFrequencySpectrumPowerDist") const = 0;
#endif

    };

} /* namespace Katydid */
#endif /* KTTIMEFREQUENCY_HH_ */
