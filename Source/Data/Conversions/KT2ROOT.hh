/*
 * KT2ROOT.hh
 *
 *  Created on: Dec 23, 2013
 *      Author: nsoblath
 */

#ifndef KT2ROOT_HH_
#define KT2ROOT_HH_

#include <string>

#include "KTPhysicalArray.hh"

class TH1I;
class TH1D;
class TH2D;

namespace Katydid
{
    class KTRawTimeSeries;
    class KTTimeSeriesDist;
    class KTTimeSeriesFFTW;
    class KTTimeSeriesReal;

    class KT2ROOT
    {
        public:
            KT2ROOT();
            virtual ~KT2ROOT();

            static TH1I* CreateHistogram(const KTRawTimeSeries* ts, const std::string& histName = "hRawTimeSeries");
            static TH1I* CreateHistogram(const KTTimeSeriesDist* tsDist, const std::string& histName = "hTSDist");
            //static TH1I* CreateAmplitudeDistributionHistogram(const KTRawTimeSeries* ts, const std::string& histName = "hRawTSDist");

            static TH1D* CreateHistogram(const KTTimeSeriesFFTW* ts, const std::string& histName = "hTimeSeries");
            //static TH1D* CreateAmplitudeDistributionHistogram(const KTTimeSeriesFFTW* ts, const std::string& histName = "hTimeSeriesDist");

            static TH1D* CreateHistogram(const KTTimeSeriesReal* ts, const std::string& histName = "hTimeSeries");
            //static TH1D* CreateAmplitudeDistributionHistogram(const KTTimeSeriesReal* ts, const std::string& histName = "hTimeSeriesDist");

            static TH2D* CreateHistogram(const KTPhysicalArray< 2, double >* ht, const std::string& histName = "hHoughData");
    };

} /* namespace Katydid */
#endif /* KT2ROOT_HH_ */
