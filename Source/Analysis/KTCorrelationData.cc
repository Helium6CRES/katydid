/*
 * KTCorrelationData.cc
 *
 *  Created on: Aug 24, 2012
 *      Author: nsoblath
 */

#include "KTCorrelationData.hh"

#include "KTFrequencySpectrum.hh"
#include "KTWriter.hh"

namespace Katydid
{
    std::string KTCorrelationData::fName("correlation");

    const std::string& KTCorrelationData::StaticGetName()
    {
        return fName;
    }

    KTCorrelationData::KTCorrelationData(UInt_t nPairs) :
            KTWriteableData(),
            fData(nPairs)
    {
    }

    KTCorrelationData::~KTCorrelationData()
    {
        while (! fData.empty())
        {
            delete fData.back().fCorrelation;
            fData.pop_back();
        }
    }

    void KTCorrelationData::Accept(KTWriter* writer) const
    {
        writer->Write(this);
        return;
    }


} /* namespace Katydid */
