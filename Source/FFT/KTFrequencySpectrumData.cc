/*
 * KTFrequencySpectrumData.cc
 *
 *  Created on: Aug 24, 2012
 *      Author: nsoblath
 */

#include "KTFrequencySpectrumData.hh"

#include "KTFrequencySpectrum.hh"
#include "KTWriter.hh"

namespace Katydid
{
    std::string KTFrequencySpectrumData::fDefaultName("frequency-spectrum");

    const std::string& KTFrequencySpectrumData::StaticGetDefaultName()
    {
        return fDefaultName;
    }

    KTFrequencySpectrumData::KTFrequencySpectrumData(unsigned nChannels) :
            KTWriteableData(),
            fSpectra(nChannels)
    {
    }

    KTFrequencySpectrumData::~KTFrequencySpectrumData()
    {
        while (! fSpectra.empty())
        {
            delete fSpectra.back();
            fSpectra.pop_back();
        }
    }

    void KTFrequencySpectrumData::Accept(KTWriter* writer) const
    {
        writer->Write(this);
        return;
    }

} /* namespace Katydid */

