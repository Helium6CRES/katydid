/*
 * KTFrequencySpectrumDataPolar.hh
 *
 *  Created on: Aug 24, 2012
 *      Author: nsoblath
 */

#ifndef KTFREQUENCYSPECTRUMDATAPOLAR_HH_
#define KTFREQUENCYSPECTRUMDATAPOLAR_HH_

#include "KTData.hh"

#include "KTFrequencySpectrumPolar.hh"

#ifdef ROOT_FOUND
#include "TH1.h"
#endif

#include <vector>

namespace Katydid
{

    class KTFrequencySpectrumDataPolar : public KTExtensibleData< KTFrequencySpectrumDataPolar >
    {
        public:
            KTFrequencySpectrumDataPolar();
            virtual ~KTFrequencySpectrumDataPolar();

            UInt_t GetNComponents() const;

            const KTFrequencySpectrumPolar* GetSpectrumPolar(UInt_t component = 0) const;
            KTFrequencySpectrumPolar* GetSpectrumPolar(UInt_t component = 0);

            const KTFrequencySpectrum* GetSpectrum(UInt_t component = 0) const;
            KTFrequencySpectrum* GetSpectrum(UInt_t component = 0);

            void SetSpectrum(KTFrequencySpectrumPolar* record, UInt_t component = 0);

            KTFrequencySpectrumDataPolar& SetNComponents(UInt_t channels);

        protected:
            std::vector< KTFrequencySpectrumPolar* > fSpectra;

#ifdef ROOT_FOUND
        public:
            TH1D* CreateMagnitudeHistogram(UInt_t component = 0, const std::string& name = "hFrequencySpectrumMag") const;
            TH1D* CreatePhaseHistogram(UInt_t component = 0, const std::string& name = "hFrequencySpectrumPhase") const;

            TH1D* CreatePowerHistogram(UInt_t component = 0, const std::string& name = "hFrequencySpectrumPower") const;

            TH1D* CreatePowerDistributionHistogram(UInt_t component = 0, const std::string& name = "hFrequencySpectrumPowerDist") const;
#endif
    };

    inline const KTFrequencySpectrumPolar* KTFrequencySpectrumDataPolar::GetSpectrumPolar(UInt_t component) const
    {
        return fSpectra[component];
    }

    inline KTFrequencySpectrumPolar* KTFrequencySpectrumDataPolar::GetSpectrumPolar(UInt_t component)
    {
        return fSpectra[component];
    }

    inline const KTFrequencySpectrum* KTFrequencySpectrumDataPolar::GetSpectrum(UInt_t component) const
    {
        return fSpectra[component];
    }

    inline KTFrequencySpectrum* KTFrequencySpectrumDataPolar::GetSpectrum(UInt_t component)
    {
        return fSpectra[component];
    }

    inline UInt_t KTFrequencySpectrumDataPolar::GetNComponents() const
    {
        return UInt_t(fSpectra.size());
    }

    inline void KTFrequencySpectrumDataPolar::SetSpectrum(KTFrequencySpectrumPolar* record, UInt_t component)
    {
        if (component >= fSpectra.size()) fSpectra.resize(component+1);
        fSpectra[component] = record;
        return;
    }

    inline KTFrequencySpectrumDataPolar& KTFrequencySpectrumDataPolar::SetNComponents(UInt_t channels)
    {
        fSpectra.resize(channels);
        return *this;
    }

#ifdef ROOT_FOUND
    inline TH1D* KTFrequencySpectrumDataPolar::CreateMagnitudeHistogram(UInt_t component, const std::string& name) const
    {
        return fSpectra[component]->CreateMagnitudeHistogram(name);
    }
    inline TH1D* KTFrequencySpectrumDataPolar::CreatePhaseHistogram(UInt_t component, const std::string& name) const
    {
        return fSpectra[component]->CreatePhaseHistogram(name);
    }

    inline TH1D* KTFrequencySpectrumDataPolar::CreatePowerHistogram(UInt_t component, const std::string& name) const
    {
        return fSpectra[component]->CreatePowerHistogram(name);
    }

    inline TH1D* KTFrequencySpectrumDataPolar::CreatePowerDistributionHistogram(UInt_t component, const std::string& name) const
    {
        return fSpectra[component]->CreatePowerDistributionHistogram(name);
    }
#endif


} /* namespace Katydid */

#endif /* KTFREQUENCYSPECTRUMDATAPOLAR_HH_ */
