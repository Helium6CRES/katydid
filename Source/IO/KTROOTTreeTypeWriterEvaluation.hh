/*
 * KTROOTTreeTypeWriterEvaluation.hh
 *
 *  Created on: May 31, 2013
 *      Author: nsoblath
 */

#ifndef KTROOTTREETYPEWRITEREVALUATION_HH_
#define KTROOTTREETYPEWRITEREVALUATION_HH_

#include "KTROOTTreeWriter.hh"

#include "KTData.hh"

class TH2D;
class TTree;

namespace Katydid
{
    struct TCCLocustMC
    {
        Double_t fLength;
        Double_t fdfdt;
        Double_t fPower;
        Double_t fEfficiency;
        Double_t fFalseRate;
    };

    class KTROOTTreeTypeWriterEvaluation : public KTROOTTreeTypeWriter
    {
        public:
            KTROOTTreeTypeWriterEvaluation();
            virtual ~KTROOTTreeTypeWriterEvaluation();

            void RegisterSlots();

        public:
            void WriteMetaCCLocustMC(KTDataPtr data);

        public:
            TTree* GetMetaCCLocustMCTree() const;

        private:
            bool SetupMetaCCLocustMCTree();

            TTree* fMetaCCLocustMCTree;

            TCCLocustMC fCCLocustMC;

    };

    inline TTree* KTROOTTreeTypeWriterEvaluation::GetMetaCCLocustMCTree() const
    {
        return fMetaCCLocustMCTree;
    }

} /* namespace Katydid */


#endif /* KTROOTTREETYPEWRITEREVALUATION_HH_ */
