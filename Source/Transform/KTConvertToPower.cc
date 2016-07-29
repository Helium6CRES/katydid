/*
 * KTConvertToPower.cc
 *
 *  Created on: nsoblath
 *      Author: Aug 1, 2014
 */

#include "KTConvertToPower.hh"

#include "KTFrequencySpectrumFFTW.hh"
#include "KTFrequencySpectrumDataFFTW.hh"
#include "KTFrequencySpectrumDataPolar.hh"
#include "KTFrequencySpectrumPolar.hh"
#include "KTLogger.hh"
#include "KTNOFactory.hh"
#include "KTPowerSpectrum.hh"
#include "KTPowerSpectrumData.hh"

using boost::shared_ptr;

namespace Katydid
{
    KTLOGGER(pslog, "katydid.fft");

    // Register the processor
    KT_REGISTER_PROCESSOR(KTConvertToPower, "convert-to-power");

    KTConvertToPower::KTConvertToPower(const std::string& name) :
            KTProcessor(name),
            fFSPToPSSlot("fs-polar-to-ps", this, &KTConvertToPower::ToPowerSpectrum, &fPowerSpectrumSignal),
            fFSPToPSDSlot("fs-polar-to-psd", this, &KTConvertToPower::ToPowerSpectralDensity, &fPowerSpectralDensitySignal),
            fFSFToPSSlot("fs-fftw-to-ps", this, &KTConvertToPower::ToPowerSpectrum, &fPowerSpectrumSignal),
            fFSFToPSDSlot("fs-fftw-to-psd", this, &KTConvertToPower::ToPowerSpectralDensity, &fPowerSpectralDensitySignal),
            fPSDToPSSlot("psd-to-ps", this, &KTConvertToPower::ToPowerSpectrum, &fPowerSpectrumSignal),
            fPSToPSDSlot("ps-to-psd", this, &KTConvertToPower::ToPowerSpectralDensity, &fPowerSpectralDensitySignal),
            fPowerSpectrumSignal("ps", this),
            fPowerSpectralDensitySignal("psd", this)
    {
    }

    KTConvertToPower::~KTConvertToPower()
    {
    }

    bool KTConvertToPower::Configure(const scarab::param_node* node)
    {
        if (node == NULL) return true;

        // no parameters

        return true;
    }

    bool KTConvertToPower::ToPowerSpectrum(KTFrequencySpectrumDataPolar& data)
    {
        unsigned nComponents = data.GetNComponents();
        KTPowerSpectrumData& psData = data.Of< KTPowerSpectrumData >().SetNComponents(nComponents);
        for (unsigned iComponent = 0; iComponent < nComponents; ++iComponent)
        {
            KTPowerSpectrum* spectrum = data.GetSpectrum(iComponent)->CreatePowerSpectrum();
            spectrum->ConvertToPowerSpectrum();
            psData.SetSpectrum(spectrum, iComponent);
        }
        return true;
    }
    bool KTConvertToPower::ToPowerSpectralDensity(KTFrequencySpectrumDataPolar& data)
    {
        unsigned nComponents = data.GetNComponents();
        KTPowerSpectrumData& psData = data.Of< KTPowerSpectrumData >().SetNComponents(nComponents);
        for (unsigned iComponent = 0; iComponent < nComponents; ++iComponent)
        {
            KTPowerSpectrum* spectrum = data.GetSpectrum(iComponent)->CreatePowerSpectrum();
            spectrum->ConvertToPowerSpectralDensity();
            psData.SetSpectrum(spectrum, iComponent);
        }
        return true;
    }

    bool KTConvertToPower::ToPowerSpectrum(KTFrequencySpectrumDataFFTW& data)
    {
        unsigned nComponents = data.GetNComponents();
        KTPowerSpectrumData& psData = data.Of< KTPowerSpectrumData >().SetNComponents(nComponents);
        for (unsigned iComponent = 0; iComponent < nComponents; ++iComponent)
        {
            KTPowerSpectrum* spectrum = data.GetSpectrum(iComponent)->CreatePowerSpectrum();
            spectrum->ConvertToPowerSpectrum();
            psData.SetSpectrum(spectrum, iComponent);
        }
        return true;
    }

    bool KTConvertToPower::ToPowerSpectralDensity(KTFrequencySpectrumDataFFTW& data)
    {
        unsigned nComponents = data.GetNComponents();
        KTPowerSpectrumData& psData = data.Of< KTPowerSpectrumData >().SetNComponents(nComponents);
        for (unsigned iComponent = 0; iComponent < nComponents; ++iComponent)
        {
            KTPowerSpectrum* spectrum = data.GetSpectrum(iComponent)->CreatePowerSpectrum();
            spectrum->ConvertToPowerSpectralDensity();
            psData.SetSpectrum(spectrum, iComponent);
        }
        return true;
    }

    bool KTConvertToPower::ToPowerSpectrum(KTPowerSpectrumData& data)
    {
        unsigned nComponents = data.GetNComponents();
        for (unsigned iComponent = 0; iComponent < nComponents; ++iComponent)
        {
            data.GetSpectrum(iComponent)->ConvertToPowerSpectrum();
        }
        return true;
    }

    bool KTConvertToPower::ToPowerSpectralDensity(KTPowerSpectrumData& data)
    {
        unsigned nComponents = data.GetNComponents();
        for (unsigned iComponent = 0; iComponent < nComponents; ++iComponent)
        {
            data.GetSpectrum(iComponent)->ConvertToPowerSpectralDensity();
        }
        return true;
    }

} /* namespace Katydid */
