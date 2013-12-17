/*
 * KTSwitchFFTWPolar.cc
 *
 *  Created on: Mar 19, 2013
 *      Author: nsoblath
 */

#include "KTSwitchFFTWPolar.hh"

#include "KTNOFactory.hh"
#include "KTFrequencySpectrumDataPolar.hh"
#include "KTFrequencySpectrumDataFFTW.hh"
#include "KTFrequencySpectrumFFTW.hh"
#include "KTFrequencySpectrumPolar.hh"
#include "KTNormalizedFSData.hh"
#include "KTLogger.hh"
#include "KTNormalizedFSData.hh"
#include "KTPStoreNode.hh"
#include "KTWignerVilleData.hh"

using std::string;


namespace Katydid
{
    KTLOGGER(swlog, "katydid.analysis");

    static KTDerivedNORegistrar< KTProcessor, KTSwitchFFTWPolar > sSwitchFFTWPolarRegistrar("switch-fftw-polar");

    KTSwitchFFTWPolar::KTSwitchFFTWPolar(const std::string& name) :
            KTProcessor(name),
            fUseNegFreqs(true),
            fFSPolarSignal("fs-polar", this),
            //fFSFFTWSignal("fs-fftw", this),
            fFSFFTWSlot("fs-fftw", this, &KTSwitchFFTWPolar::SwitchToPolar, &fFSPolarSignal),
            fNormFSFFTWSlot("norm-fs-fftw", this, &KTSwitchFFTWPolar::SwitchToPolar, &fFSPolarSignal),
            fWignerVilleSlot("wv", this, &KTSwitchFFTWPolar::SwitchToPolar, &fFSPolarSignal)
    {
    }

    KTSwitchFFTWPolar::~KTSwitchFFTWPolar()
    {
    }

    Bool_t KTSwitchFFTWPolar::Configure(const KTPStoreNode* node)
    {
        if (node == NULL) return false;

        SetUseNegFreqs(node->GetData< Bool_t >("use-neg-freqs", fUseNegFreqs));

        return true;
    }


    Bool_t KTSwitchFFTWPolar::SwitchToPolar(KTFrequencySpectrumDataFFTW& fsData)
    {
        unsigned nComponents = fsData.GetNComponents();

        KTFrequencySpectrumDataPolar& newData = fsData.Of< KTFrequencySpectrumDataPolar >().SetNComponents(nComponents);

        for (unsigned iComponent=0; iComponent<nComponents; iComponent++)
        {
            KTFrequencySpectrumPolar* newSpectrum = fsData.GetSpectrumFFTW(iComponent)->CreateFrequencySpectrumPolar(fUseNegFreqs);
            if (newSpectrum == NULL)
            {
                KTERROR(swlog, "Switch of spectrum " << iComponent << " (fftw->polar) failed for some reason. Continuing processing.");
                continue;
            }
            else
            {
                KTFrequencySpectrumFFTW* oldSpectrum = fsData.GetSpectrumFFTW(iComponent);
                KTDEBUG(swlog, "fftw array: " << oldSpectrum->size() << " bins; range: " << oldSpectrum->GetRangeMin() << " - " << oldSpectrum->GetRangeMax());
                KTDEBUG(swlog, "polar array: " << newSpectrum->size() << " bins; range: " << newSpectrum->GetRangeMin() << " - " << newSpectrum->GetRangeMax());
            }
            newData.SetSpectrum(newSpectrum, iComponent);
        }
        KTINFO(swlog, "Completed switch (fftw->polar) of " << nComponents << " frequency spectra (polar)");

        return true;
    }

    Bool_t KTSwitchFFTWPolar::SwitchToPolar(KTNormalizedFSDataFFTW& fsData)
    {
        unsigned nComponents = fsData.GetNComponents();

        KTFrequencySpectrumDataPolar& newData = fsData.Of< KTFrequencySpectrumDataPolar >().SetNComponents(nComponents);

        for (unsigned iComponent=0; iComponent<nComponents; iComponent++)
        {
            KTFrequencySpectrumPolar* newSpectrum = fsData.GetSpectrumFFTW(iComponent)->CreateFrequencySpectrumPolar(fUseNegFreqs);
            if (newSpectrum == NULL)
            {
                KTERROR(swlog, "Switch of spectrum " << iComponent << " (fftw->polar) failed for some reason. Continuing processing.");
                continue;
            }
            else
            {
                KTFrequencySpectrumFFTW* oldSpectrum = fsData.GetSpectrumFFTW(iComponent);
                KTDEBUG(swlog, "fftw array: " << oldSpectrum->size() << " bins; range: " << oldSpectrum->GetRangeMin() << " - " << oldSpectrum->GetRangeMax());
                KTDEBUG(swlog, "polar array: " << newSpectrum->size() << " bins; range: " << newSpectrum->GetRangeMin() << " - " << newSpectrum->GetRangeMax());
            }
            newData.SetSpectrum(newSpectrum, iComponent);
        }
        KTINFO(swlog, "Completed switch (fftw->polar) of " << nComponents << " frequency spectra (polar)");

        return true;
    }

    Bool_t KTSwitchFFTWPolar::SwitchToPolar(KTWignerVilleData& fsData)
    {
        unsigned nComponents = fsData.GetNComponents();

        KTFrequencySpectrumDataPolar& newData = fsData.Of< KTFrequencySpectrumDataPolar >().SetNComponents(nComponents);

        for (unsigned iComponent=0; iComponent<nComponents; iComponent++)
        {
            KTFrequencySpectrumPolar* newSpectrum = fsData.GetSpectrumFFTW(iComponent)->CreateFrequencySpectrumPolar(fUseNegFreqs);
            if (newSpectrum == NULL)
            {
                KTERROR(swlog, "Switch of spectrum " << iComponent << " (fftw->polar) failed for some reason. Continuing processing.");
                continue;
            }
            else
            {
                KTFrequencySpectrumFFTW* oldSpectrum = fsData.GetSpectrumFFTW(iComponent);
                KTDEBUG(swlog, "fftw array: " << oldSpectrum->size() << " bins; range: " << oldSpectrum->GetRangeMin() << " - " << oldSpectrum->GetRangeMax());
                KTDEBUG(swlog, "polar array: " << newSpectrum->size() << " bins; range: " << newSpectrum->GetRangeMin() << " - " << newSpectrum->GetRangeMax());
            }
            newData.SetSpectrum(newSpectrum, iComponent);
        }
        KTINFO(swlog, "Completed switch (fftw->polar) of " << nComponents << " frequency spectra (polar)");

        return true;
    }

} /* namespace Katydid */
