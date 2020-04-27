/*
 * KTPowerSpectrumData.cc
 *
 *  Created on: Aug 1, 2014
 *      Author: nsoblath
 */

#include "KTLPFData.hh"


namespace Katydid
{

    const std::string KTFrequencySpectrumDataFFTW_LPF::sName("lpf-fs-fftw");

    KTFrequencySpectrumDataFFTW_LPF::KTFrequencySpectrumDataFFTW_LPF() 
    {}

    KTFrequencySpectrumDataFFTW_LPF::~KTFrequencySpectrumDataFFTW_LPF()
    {}

    KTFrequencySpectrumDataFFTW_LPF& KTFrequencySpectrumDataFFTW_LPF::SetNComponents(unsigned components)
    {
        unsigned oldSize = fSpectra.size();
        // if components < oldSize
        for (unsigned iComponent = components; iComponent < oldSize; ++iComponent)
        {
            delete fSpectra[iComponent];
        }
        fSpectra.resize(components);
        // if components > oldSize
        for (unsigned iComponent = oldSize; iComponent < components; ++iComponent)
        {
            fSpectra[iComponent] = NULL;
        }
        return *this;
    }


    const std::string KTFrequencySpectrumDataPolar_LPF::sName("lpf-fs-polar");

    KTFrequencySpectrumDataPolar_LPF::KTFrequencySpectrumDataPolar_LPF() 
    {}

    KTFrequencySpectrumDataPolar_LPF::~KTFrequencySpectrumDataPolar_LPF()
    {}

    KTFrequencySpectrumDataPolar_LPF& KTFrequencySpectrumDataPolar_LPF::SetNComponents(unsigned components)
    {
        unsigned oldSize = fSpectra.size();
        // if components < oldSize
        for (unsigned iComponent = components; iComponent < oldSize; ++iComponent)
        {
            delete fSpectra[iComponent];
        }
        fSpectra.resize(components);
        // if components > oldSize
        for (unsigned iComponent = oldSize; iComponent < components; ++iComponent)
        {
            fSpectra[iComponent] = NULL;
        }
        return *this;
    }


    const std::string KTPowerSpectrumData_LPF::sName("lpf-ps");

    KTPowerSpectrumData_LPF::KTPowerSpectrumData_LPF() 
    {}

    KTPowerSpectrumData_LPF::~KTPowerSpectrumData_LPF()
    {}

    KTPowerSpectrumData_LPF& KTPowerSpectrumData_LPF::SetNComponents(unsigned num)
    {
        unsigned oldSize = fSpectra.size();
        // if num < oldSize
        for (unsigned iComponent = num; iComponent < oldSize; ++iComponent)
        {
            delete fSpectra[iComponent];
        }
        fSpectra.resize(num);
        // if num > oldSize
        for (unsigned iComponent = oldSize; iComponent < num; ++iComponent)
        {
            fSpectra[iComponent] = NULL;
        }
        return *this;
    }

} /* namespace Katydid */

