/*
 * KTTimeSeriesData.cc
 *
 *  Created on: Sep 9, 2011
 *      Author: nsoblath
 */

#include "KTTimeSeriesData.hh"

namespace Katydid
{
    KTTimeSeriesDataCore::KTTimeSeriesDataCore() :
            fTimeSeries(1)
    {
        fTimeSeries[0] = NULL;
    }

    KTTimeSeriesDataCore::~KTTimeSeriesDataCore()
    {
        while (! fTimeSeries.empty())
        {
            delete fTimeSeries.back();
            fTimeSeries.pop_back();
        }
    }

    KTTimeSeriesData::KTTimeSeriesData() :
            KTTimeSeriesDataCore(),
            KTExtensibleData< KTTimeSeriesData >()
    {
    }

    KTTimeSeriesData::~KTTimeSeriesData()
    {
    }

    KTTimeSeriesData& KTTimeSeriesData::SetNComponents(UInt_t num)
    {
        UInt_t oldSize = fTimeSeries.size();
        // if num < oldSize
        for (UInt_t iComponent = num; iComponent < oldSize; iComponent++)
        {
            delete fTimeSeries[iComponent];
        }
        fTimeSeries.resize(num);
        // if num > oldSize
        for (UInt_t iComponent = oldSize; iComponent < num; iComponent++)
        {
            fTimeSeries[iComponent] = NULL;
        }
        return *this;
    }

} /* namespace Katydid */