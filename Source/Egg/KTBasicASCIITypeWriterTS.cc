#include "KTBasicASCIITypeWriterTS.hh"

#include "KTSliceHeader.hh"
#include "KTTimeSeries.hh"
#include "KTTimeSeriesData.hh"

using boost::shared_ptr;

namespace Katydid 
{

    KTLOGGER(ats_log, "katydid.output.ascii.tw.ts");

    static KTDerivedTIRegistrar< KTBasicASCIITypeWriter, KTBasicASCIITypeWriterTS > sBATWReg;

    KTBasicASCIITypeWriterTS::KTBasicASCIITypeWriterTS() :
            KTBasicASCIITypeWriter()
    {/* no-op */}

    KTBasicASCIITypeWriterTS::~KTBasicASCIITypeWriterTS()
    {/* no-op */}

    void KTBasicASCIITypeWriterTS::RegisterSlots()
    {
        fWriter->RegisterSlot("ts-data",
                this,
                &KTBasicASCIITypeWriterTS::WriteTimeSeriesData,
                "void (shared_ptr< KTData >)");
    }

    void KTBasicASCIITypeWriterTS::WriteTimeSeriesData(shared_ptr< KTData > data)
    {
        if (! data) return;

        ULong64_t sliceNumber = data->Of<KTSliceHeader>().GetSliceNumber();

        KTTimeSeriesData& tsData = data->Of<KTTimeSeriesData>();
        UInt_t nCh = tsData.GetNComponents();

        if( fWriter->CanWrite() == true ) {

            for(UInt_t iCh = 0; iCh < nCh; iCh++) {
                std::ofstream* file_ptr = fWriter->GetStream();
                const KTTimeSeries* sCh = tsData.GetTimeSeries(iCh);
                if(sCh != NULL) {
                    for(UInt_t iB = 0; iB < sCh->GetNTimeBins(); iB++) {
                        (*file_ptr) << sliceNumber
                                << ","
                                << iCh
                                << ","
                                << iB
                                << ","
                                << sCh->GetValue(iB)
                                << std::endl;
                    }
                }
                else {
                    KTWARN(ats_log, "Channel #" << iCh << " was missing from slice!  Logic error?");
                }
            }
        } // if CanWrite
        else {
            KTWARN(ats_log, "Writer for ASCII TS type-writer cannot write.  No data will be written!");
        } // if cannot write
    }

}; // namespace katydid
