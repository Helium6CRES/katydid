/*
 * KTMultiFSDataFFTWCore.hh
 *
 *  Created on: May 21, 2013
 *      Author: nsoblath
 */

#ifndef KTMULTIFSDATAFFTW_HH_
#define KTMULTIFSDATAFFTW_HH_

#include "KTData.hh"

#include "KTFrequencySpectrumFFTW.hh"

#ifdef ROOT_FOUND
#include "TH2.h"
#endif

#include <vector>

namespace Katydid
{
    

    class KTMultiFSDataFFTWCore
    {
        public:
            KTMultiFSDataFFTWCore();
            virtual ~KTMultiFSDataFFTWCore();

            const KTPhysicalArray< 1, KTFrequencySpectrumFFTW* >* GetSpectra(unsigned component = 0) const;
            KTPhysicalArray< 1, KTFrequencySpectrumFFTW* >* GetSpectra(unsigned component = 0);
            unsigned GetNComponents() const;

            void SetSpectra(KTPhysicalArray< 1, KTFrequencySpectrumFFTW* >* spectra, unsigned component = 0);
            void SetSpectrum(KTFrequencySpectrumFFTW* spectrum, unsigned iSpect, unsigned component = 0);

            void SetNSpectra(unsigned nSpectra);
            virtual KTMultiFSDataFFTWCore& SetNComponents(unsigned components) = 0;

        protected:
            void DeleteSpectra(unsigned component = 0);

            std::vector< KTPhysicalArray< 1, KTFrequencySpectrumFFTW* >* > fSpectra;

#ifdef ROOT_FOUND
        public:
            virtual TH2D* CreateMagnitudeHistogram(unsigned component = 0, const std::string& name = "hMultiFSMag") const;
            virtual TH2D* CreatePhaseHistogram(unsigned component = 0, const std::string& name = "hMultiFSPhase") const;

            virtual TH2D* CreatePowerHistogram(unsigned component = 0, const std::string& name = "hMultiFSPower") const;
#endif

    };

    inline const KTPhysicalArray< 1, KTFrequencySpectrumFFTW* >* KTMultiFSDataFFTWCore::GetSpectra(unsigned component) const
    {
        return fSpectra[component];
    }

    inline KTPhysicalArray< 1, KTFrequencySpectrumFFTW* >* KTMultiFSDataFFTWCore::GetSpectra(unsigned component)
    {
        return fSpectra[component];
    }

    inline unsigned KTMultiFSDataFFTWCore::GetNComponents() const
    {
        return unsigned(fSpectra.size());
    }

    inline void KTMultiFSDataFFTWCore::SetSpectra(KTPhysicalArray< 1, KTFrequencySpectrumFFTW* >* spectra, unsigned component)
    {
        if (component >= fSpectra.size())
            SetNComponents(component+1);
        DeleteSpectra(component);
        fSpectra[component] = spectra;
        return;
    }

    inline void KTMultiFSDataFFTWCore::DeleteSpectra(unsigned component)
    {
        if (component >= fSpectra.size())
            return;
        for (KTPhysicalArray< 1, KTFrequencySpectrumFFTW* >::iterator iter = fSpectra[component]->begin(); iter != fSpectra[component]->end(); iter++)
        {
            delete *iter;
        }
        delete fSpectra[component];
        fSpectra[component] = NULL;
        return;
    }



    class KTMultiFSDataFFTW : public KTMultiFSDataFFTWCore, public Nymph::KTExtensibleData< KTMultiFSDataFFTW >
    {
        public:
            KTMultiFSDataFFTW();
            virtual ~KTMultiFSDataFFTW();

            KTMultiFSDataFFTW& SetNComponents(unsigned component);

        public:
            static const std::string sName;

    };

} /* namespace Katydid */

#endif /* KTMULTIFSDATAFFTW_HH_ */
