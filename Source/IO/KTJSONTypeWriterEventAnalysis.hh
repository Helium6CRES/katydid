/*
 * KTJSONTypeWriterEventAnalysis.hh
 *
 *  Created on: Jan 4, 2013
 *      Author: nsoblath
 */

#ifndef KTJSONTYPEWRITEREVENTANALYSIS_HH_
#define KTJSONTYPEWRITEREVENTANALYSIS_HH_

#include "KTJSONWriter.hh"

#include "KTData.hh"

namespace Katydid
{
    using namespace Nymph;
    class KTJSONTypeWriterEventAnalysis : public KTJSONTypeWriter//, public KTTypeWriterEventAnalysis
    {
        public:
            KTJSONTypeWriterEventAnalysis();
            virtual ~KTJSONTypeWriterCandidates();

            void RegisterSlots();

        public:
            void WriteFrequencyCandidates(KTDataPtr data);

    };

} /* namespace Katydid */
#endif /* KTJSONTYPEWRITEREVENTANALYSIS_HH_ */
