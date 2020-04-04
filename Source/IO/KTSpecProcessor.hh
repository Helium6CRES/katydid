#ifndef KTEGGPROCESSOR_HH_
#define KTEGGPROCESSOR_HH_

#include "KTPrimaryProcessor.hh"
#include "KTData.hh"
//#include "KTSpecHeader.hh"
#include "KTSpecReader.hh"
#include "KTSlot.hh"

namespace Katydid
{

    class KTPowerSpectrumData;

    /*!
     @class KTSpecProcessor
     @author B. Graner

     @brief reads a file with power spectrum data

     Configuration name: "spec-processor"

     Available configuration options:
     - "progress-report-interval": unsigned -- Interval (# of slices) between
        reports (mainly relevant for RELEASE builds); turn off by setting to 0
     - "filename": string -- Spec filename to use
        (will take priority over \"filenames\")
     - "filenames": array of strings -- Spec filenames to use
        (\"filename\" will take priority over this)

     Command-line options defined
     - -s (spec-file): spec filename to use

     Signals:
     - "header": void (Nymph::KTDataPtr) -- emitted when the header is parsed.
     - "psd": void (Nymph::KTDataPtr) -- emitted when the new power spectrum is
        produced; Guarantees KTPowerSpectrumData
     - "spec-done": void () --  emitted when a file is finished.
     - "summary": void (const KTProcSummary*) -- emitted when a file is
        finished (after "spec-done")

    */
    class KTSpecProcessor : public Nymph::KTPrimaryProcessor
    {
        public:
            KTSpecProcessor(const std::string& name = "spec-processor");
            virtual ~KTSpecProcessor();

            bool Configure(const scarab::param_node* node);

            bool Run();

            bool ProcessSpec();

            MEMBERVARIABLE(unsigned, ProgressReportInterval);

            MEMBERVARIABLEREF(KTSpecReader::path_vec, Filenames);

        private:
            int fNSpectra;
            int fPacketHeaderSize;
            size_t fFreqBins;
            double fFreqMin;
            double fFreqMax;

            //Nymph::KTSignalData fHeaderSignal;

            Nymph::KTSignalData fDataSignal;


    };

    inline bool KTSpecProcessor::Run()
    {
        return ProcessSpec();
    }


} /* namespace Katydid */

#endif /* KTSPECPROCESSOR_HH_ */
