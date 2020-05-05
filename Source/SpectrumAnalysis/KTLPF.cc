/*
 * KTProcessorTemplate.cc
 *
 *  Created on: Oct 13, 2016
 *      Author: ezayas
 */


#include "KTLPF.hh"
#include "KTLPFData.hh"

#include "KTFrequencySpectrumDataPolar.hh"
#include "KTFrequencySpectrumDataFFTW.hh"
#include "KTFrequencySpectrumFFTW.hh"
#include "KTFrequencySpectrumPolar.hh"
#include "KTLogger.hh"
#include "KTMath.hh"
#include "KTPowerSpectrum.hh"
#include "KTPowerSpectrumData.hh"

#include "param.hh"


// DO I need the old fs-fftw and such data classes included up here? 

namespace Katydid
{
    KTLOGGER(gclog, "KTLPF");  // How do we know this is supposed to be gclog?

    // Register the processor
    KT_REGISTER_PROCESSOR(KTLPF, "lp-filter");

    // Constructor
    KTLPF::KTLPF(const std::string& name) :   // name is declared in the header
            KTProcessor(name),
            fTimeConst(2.0e-8),  // RC time constant for the LPF initialized to 0. 

            //Signals Why are we naming these the same? And not matching them to 
            // the name of the output data type: "lpf-fs-fftw" and such? Is this to be able to use 
            // downstream processors that can receive these signals?
            // 4/24/20: Note I'm changing these names. Though they should only matter in the config file. 
            fFSPolarSignal("fs-polar-lpf", this),
            fFSFFTWSignal("fs-fftw-lpf", this),
            fPSSignal("ps-lpf", this),

            //Slots: 
            // 3rd and 4th arguments: memory loc for the function, and memory loc of the signal??
            fFSPolarSlot("fs-polar", this, &KTLPF::Filter, &fFSPolarSignal),
            fFSFFTWSlot("fs-fftw", this, &KTLPF::Filter, &fFSFFTWSignal),
            fPSSlot("ps", this, &KTLPF::Filter, &fPSSignal)
    {
        //So the constructor doesn't actually run any code it just sets the 
        //above member variables?
    }

    //Destructor
    KTLPF::~KTLPF()
    {

    }
    //Comments on the Methods to follow is in a seperate document that will be attached to the tutorial. 
    bool KTLPF::Configure(const scarab::param_node* node)
    // I don't really get what this does: How does it relate to the macro MEMBERVARIABLE 
    // in the header file?
    {
        if (node == NULL) return false;

        //This getter and setter is defined in the MEMBERVARIABLE macro in the header file. 
        SetTimeConst(node->get_value< double >("TimeConst", GetTimeConst()));
        
        return true;
    }

    bool KTLPF::Filter(KTFrequencySpectrumDataPolar& fsData)
    // what does the above accomplish and why do we use fsdata twice?
    {
        // unsigned nComponents = fsData.GetNComponents();
        // KTFrequencySpectrumDataPolar_LPF& newData = fsData.Of< KTFrequencySpectrumDataPolar_LPF >().SetNComponents(nComponents);

        // for (unsigned iComponent=0; iComponent<nComponents; ++iComponent)
        // {
        //     KTFrequencySpectrumPolar* newSpectrum = Filter(fsData.GetSpectrumPolar(iComponent));
        //     if (newSpectrum == NULL)
        //     {
        //         KTERROR(gclog, "Low-pass filter of spectrum " << iComponent << " failed for some reason. Continuing processing.");
        //         continue;
        //     }
        //     KTDEBUG(gclog, "Computed low-pass filter");
        //     newData.SetSpectrum(newSpectrum, iComponent);
        // }
        // KTINFO(gclog, "Completed low-pass filter of " << nComponents << " frequency spectra (polar)");
       
    

        return true;
    }

    bool KTLPF::Filter(KTFrequencySpectrumDataFFTW& fsData)
    {
        unsigned nComponents = fsData.GetNComponents();
        KTFrequencySpectrumDataFFTW_LPF& newData = fsData.Of< KTFrequencySpectrumDataFFTW_LPF >().SetNComponents(nComponents); 

        for (unsigned iComponent=0; iComponent<nComponents; ++iComponent)
        {
            KTFrequencySpectrumFFTW* newSpectrum = Filter(fsData.GetSpectrumFFTW(iComponent)); 
            if (newSpectrum == NULL)
            {
                KTERROR(gclog, "Low-pass filter of spectrum " << iComponent << " failed for some reason. Continuing processing.");
                continue;
            }
            KTDEBUG(gclog, "Computed low-pass filter");
            newData.SetSpectrum(newSpectrum, iComponent);
        }
        KTINFO(gclog, "Completed low-pass filter of " << nComponents << " frequency spectra (FFTW)");

        return true;
    }

    bool KTLPF::Filter(KTPowerSpectrumData& psData)
    {
        // unsigned nComponents = psData.GetNComponents();
        // KTPowerSpectrumData_LPF& newData = psData.Of< KTPowerSpectrumData_LPF >().SetNComponents(nComponents);

        // for (unsigned iComponent=0; iComponent<nComponents; ++iComponent)
        // {
        //     KTPowerSpectrum* newSpectrum = Filter(psData.GetSpectrum(iComponent));
        //     if (newSpectrum == NULL)
        //     {
        //         KTERROR(gclog, "Low-pass filter of spectrum " << iComponent << " failed for some reason. Continuing processing.");
        //         continue;
        //     }
        //     KTDEBUG(gclog, "Computed low-pass filter");
        //     newData.SetSpectrum(newSpectrum, iComponent);
        // }
        // KTINFO(gclog, "Completed low-pass filter of " << nComponents << " power spectra");

        return true;
    }

    KTFrequencySpectrumFFTW* KTLPF::Filter(const KTFrequencySpectrumFFTW* frequencySpectrum) const
    {
        KTDEBUG(gclog, "Creating new FS for filtered data");
        unsigned nBins = frequencySpectrum->size();
        KTFrequencySpectrumFFTW* newSpectrum = new KTFrequencySpectrumFFTW(nBins, frequencySpectrum->GetRangeMin(), frequencySpectrum->GetRangeMax());
        newSpectrum->SetNTimeBins(frequencySpectrum->GetNTimeBins());

        double twoPiRCf, real, imag, denom, newReal, newImag;
        for (unsigned iBin = 0; iBin < nBins; ++iBin)
        {
            real = (*frequencySpectrum)(iBin)[0];
            imag = (*frequencySpectrum)(iBin)[1];
            twoPiRCf = KTMath::TwoPi() * fTimeConst * frequencySpectrum->GetBinCenter(iBin);
            denom = 1 + twoPiRCf * twoPiRCf;
            (*newSpectrum)(iBin)[0] = (real + imag * twoPiRCf) / denom;
            (*newSpectrum)(iBin)[1] = (imag - real * twoPiRCf) / denom;
        }

        return newSpectrum;
    }
} // namespace Katydid
