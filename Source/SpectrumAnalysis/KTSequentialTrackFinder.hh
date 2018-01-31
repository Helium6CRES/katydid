/**
 @file KTSequentialTrackFinder.hh
 @brief Contains KTSeqTrackFinder
 @details Creates a track
 @author: Christine
 @date: Aug 4, 2017
 */

#ifndef KTSEQUENTIALTRACKFINDER_HH_
#define KTSEQUENTIALTRACKFINDER_HH_

#include "KTProcessor.hh"
#include "KTData.hh"
#include "KTMemberVariable.hh"
#include "KTSlot.hh"

#include "KTSliceHeader.hh"
#include "KTPowerSpectrum.hh"
#include "KTPowerSpectrumData.hh"
#include "KTGainVariationData.hh"
#include "KTSeqLine.hh"
#include "KTProcessedTrackData.hh"
#include "KTDiscriminatedPoints1DData.hh"

#include <iostream>
#include <set>


namespace Katydid
{
    /*!
     @class KTSeqTrackFinder
     @author E. Christine

     @brief Implementation of a slightly modified version of Dan Furse's algorithm

     @details
     Collects points on a linear track

     Configuration name: "dans-track-finding-algorithm"

     Available configuration values:
     - "snr-threshold-power": discrimination snr for point candidates
     - "min-frequency": minimum allowed frequency (has to be set)
     - "max-frequency": max allowed frequency (has to be set)
     - "min-bin": can be set instead of min frequency
     - "max-bin": can be set instead of  max frequency
     - "trimming-factor": before a line is converted to a track its edges get trimmed. If the last or first line point power is less than the trimming-factor times the power threshold of the points frequency bins in the power spectrum slice, these points get cut off the line
     - "half-line-width": the power that is assigned to a line point is the sum of the power_spectrum[point_bin - line_width: point_bin + line_width]
     - "time-gap-tolerance": maximum gap between points in a line (in seconds)
     - "minimum-line-bin-distance": if a point is less than this distance (in bins) away from the last point it will be skipped
     - "search-radius": before a point is added to a line, the weighted average of the points frequency neighborhood (+/- search-radius in bins) is taken and the point updated until the frequency converges
     - "converge-delta": defines when convergence has been reached (in bins)
     - "frequency-acceptance": maximum allowed frequency distance of point to an extrapolated line (in Hz)
     - "initial-slope": if a line has only one point, this is the line's slope
     - "min-points": a line only gets converted to a track if it has collected more than this many number of points
     - "min-slope": a line only gets converted to a track if its slope is > than this slope (in Hz/s)
     - "apply-power-cut" (bool): if true, a power threshold will be applied to a line before converting it to a processed track
     - "apply-point-density-cut" (bool): if true, a number-of-points/time-length threshold will be applied to a line before converting it to a processed track
     - "power-threshold": threshold for power cut
     - "point-density-threshold": threshold for point density cut in points/millisecond


     Slots:
     - "disc-1d": needs sparse spectrogram for thresholding 
     - "gv": needs gain variation for thresholding
     - "ps-in": power spectrum to collect points from
     - "done": connect with egg:done. Processes remaining active lines and emits clustering-done signal

     Signals:
     - "pre-candidate": KTProcessedTrackData
     - "clustering-done": void () -- Emitted when track clustering is complete
    */


    class KTSequentialTrackFinder : public Nymph::KTProcessor
    {

        private:
            // actually, currently only the eSNR_Power mode is implemented
            enum ThresholdMode
            {
                eSNR_Amplitude,
                eSNR_Power,
                eSigma
            };

            KTGainVariationData fGVData;

        public:
            KTSequentialTrackFinder(const std::string& name = "seq-clustering");
            virtual ~KTSequentialTrackFinder();

            bool Configure(const scarab::param_node* node);

        public:
            MEMBERVARIABLE(ThresholdMode, Mode);
            MEMBERVARIABLE(double, TrimmingFactor);
            MEMBERVARIABLE(int, LinePowerWidth);
            MEMBERVARIABLE(double, PointAmplitudeAfterVisit);
            MEMBERVARIABLE(int, MinFreqBinDistance);
            MEMBERVARIABLE(double, TimeGapTolerance);
            MEMBERVARIABLE(double, FrequencyAcceptance);
            MEMBERVARIABLE(double, SNRPowerThreshold);
            MEMBERVARIABLE(double, ReferenceThreshold);
            MEMBERVARIABLE(int, SearchRadius);
            MEMBERVARIABLE(double, ConvergeDelta);
            MEMBERVARIABLE(unsigned, MinPoints);
            MEMBERVARIABLE(double, MinSlope);
            MEMBERVARIABLE(double, InitialSlope);
            MEMBERVARIABLE(unsigned, MinBin);
            MEMBERVARIABLE(bool, CalculateMinBin);
            MEMBERVARIABLE(unsigned, MaxBin);
            MEMBERVARIABLE(double, BinWidth);
            MEMBERVARIABLE(bool, CalculateMaxBin);
            MEMBERVARIABLE(double, MinFrequency);
            MEMBERVARIABLE(double, MaxFrequency);
            MEMBERVARIABLE(unsigned, NLines);
            MEMBERVARIABLE(bool, ApplyPowerCut);
            MEMBERVARIABLE(bool, ApplyDensityCut);
            MEMBERVARIABLE(double, PowerThreshold);
            MEMBERVARIABLE(double, DensityThreshold);



        private:
            std::vector< LineRef> fActiveLines;


        public:
            bool SetPreCalcGainVar(KTGainVariationData& gvData);
            bool CollectPointsFromSlice(KTSliceHeader& slHeader, KTPowerSpectrumData& spectrum);
            bool CollectDiscrimPointsFromSlice(KTSliceHeader& slHeader, KTPowerSpectrumData& spectrum, KTDiscriminatedPoints1DData& discrimPoints);
            bool CollectPoints(const KTSliceHeader& slHeader, const KTPowerSpectrumData& spectrum, const KTGainVariationData& gvData);
            bool CollectDiscrimPoints(const KTSliceHeader& slHeader, const KTPowerSpectrumData& spectrum, const KTDiscriminatedPoints1DData& discrimPoints);
            //bool LoopOverHighPowerPoints(std::vector<double>& slice, std::vector<Point>& points, unsigned component);
            bool LoopOverHighPowerPoints(KTPowerSpectrum& powerSpectrum, std::vector<Point>& points, unsigned component);

            //void SearchTrueLinePoint(Point& point, std::vector<double>& slice);
            void SearchTrueLinePoint(Point& point, KTPowerSpectrum& slice);
            //void WeightedAverage(const std::vector<double>& slice, unsigned& frequencyBin, double& frequency);
            void WeightedAverage(const KTPowerSpectrum& slice, unsigned& frequencyBin, double& frequency);
            void ProcessNewTrack( KTProcessedTrackData& myNewTrack );
            bool EmitPreCandidate(LineRef line);
            void AcquisitionIsOver();



            //***************
            // Signals
            //***************

        private:
            Nymph::KTSignalData fTrackSignal;
            Nymph::KTSignalOneArg< void > fClusterDoneSignal;

            //***************
            // Slots
            //***************

        private:
            //Nymph::KTSlotDataTwoTypes< KTSliceHeader, KTPowerSpectrumData > fSeqTrackSlot;
            Nymph::KTSlotDataOneType< KTGainVariationData > fGainVarSlot;
            Nymph::KTSlotDataTwoTypes< KTSliceHeader, KTPowerSpectrumData > fPSSlot;
            Nymph::KTSlotDataThreeTypes < KTSliceHeader, KTPowerSpectrumData, KTDiscriminatedPoints1DData > fDiscrimSlot;
            Nymph::KTSlotDone fDoneSlot;

    };

} /* namespace Katydid */
#endif /* KTSEQUENTIALTRACKFinder_HH_ */
