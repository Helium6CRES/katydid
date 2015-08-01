/*
 * KTBasicROOTTypeWriterSpectrumAnalysis.hh
 *
 *  Created on: Jan 3, 2013
 *      Author: nsoblath
 */

#ifndef KTBASICROOTTYPEWRITERSPECTRUMANALYSIS_HH_
#define KTBASICROOTTYPEWRITERSPECTRUMANALYSIS_HH_

#include "KTBasicROOTFileWriter.hh"

#include "KTData.hh"

namespace Katydid
{
    using namespace Nymph;
    class KTBasicROOTTypeWriterSpectrumAnalysis : public KTBasicROOTTypeWriter
    {
        public:
            KTBasicROOTTypeWriterSpectrumAnalysis();
            virtual ~KTBasicROOTTypeWriterSpectrumAnalysis();

            void RegisterSlots();

            //************************
            // SNR
            //************************
        public:
            void WriteSNRPower(KTDataPtr data);

            //************************
            // Normalized Frequency Spectrum Data
            //************************
        public:
            void WriteNormalizedFSDataPolar(KTDataPtr data);
            void WriteNormalizedFSDataFFTW(KTDataPtr data);
            void WriteNormalizedFSDataPolarPhase(KTDataPtr data);
            void WriteNormalizedFSDataFFTWPhase(KTDataPtr data);
            void WriteNormalizedFSDataPolarPower(KTDataPtr data);
            void WriteNormalizedFSDataFFTWPower(KTDataPtr data);

            void WriteNormalizedPSData(KTDataPtr data);

            //************************
            // Analytic Associate Data
            //************************
        public:
            void WriteAnalyticAssociateData(KTDataPtr data);
            void WriteAnalyticAssociateDataDistribution(KTDataPtr data);

            //************************
            // Correlation Data
            //************************
        public:
            void WriteCorrelationData(KTDataPtr data);
            void WriteCorrelationDataDistribution(KTDataPtr data);

            //************************
            // Correlation TS Data
            //************************
        public:
            //void WriteCorrelationTSData(KTDataPtr data);
            //void WriteCorrelationTSDataDistribution(KTDataPtr data);

            //************************
            // Hough Transform Data
            //************************
        public:
            void WriteHoughData(KTDataPtr data);

            //************************
            // Gain Variation Data
            //************************
        public:
            void WriteGainVariationData(KTDataPtr data);

            //************************
            // WignerVille Data
            //************************
        public:
            void WriteWignerVilleData(KTDataPtr data);
            void WriteWignerVilleDataDistribution(KTDataPtr data);
            void WriteWV2DData(KTDataPtr data);

#ifdef ENABLE_TUTORIAL
            //************************
            // LPF Tutorial Data
            //************************
        public:
            void WriteLowPassFilteredFSDataPolar(KTDataPtr data);
            void WriteLowPassFilteredFSDataFFTW(KTDataPtr data);
            void WriteLowPassFilteredPSData(KTDataPtr data);
#endif /* ENABLE_TUTORIAL */

    };

} /* namespace Katydid */
#endif /* KTBASICROOTTYPEWRITERSPECTRUMANALYSIS_HH_ */