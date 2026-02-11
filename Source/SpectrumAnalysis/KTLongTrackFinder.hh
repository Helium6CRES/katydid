/**
 @file KTLongTrackFinder.hh
 @brief Contains KTLongTrackFinder
 @details Finds and creates track from discriminator data
 @author A. Gorman
 @author H.S. Harrington
 @date: March 7, 2024
 */

#ifndef KTLONGTRACKFINDER_HH_
#define KTLONGTRACKFINDER_HH_

#include "KTProcessor.hh"

#include "KTDiscriminatedPoints1DData.hh"
#include "KTDiscriminatedPoint.hh"
#include "KTKDTreeData.hh"

#include "KTMemberVariable.hh"
#include "KTSlot.hh"
#include "KTLongTrackData.hh"

#include <set>


namespace Katydid
{
    class KTEggHeader;
    class KTPowerSpectrum;
    class KTPowerSpectrumData;
    class KTLongTrackData;
    class KTSliceHeader;

    /**
	 * @class KTLongTrackFinder
	 * @brief Collects points from sparse spectrogram into linear tracks.
	 * 
	 * @details 
	 * KTLongTrackFinder is designed to identify long-duration, variable-slope tracks in the sparse spectrogram data. It operates on sequential KTDiscriminatedPoint1DData objects emitted by the discriminator. These are single time slices containing over-threshold points.
	 * Configuration name: "long-track-finder"

     * @section config Available configuration values:
     * - "min-frequency": minimum allowed frequency (has to be set)
     * - "max-frequency": max allowed frequency (has to be set)
     * - "min-bin": can be set instead of min frequency
     * - "max-bin": can be set instead of  max frequency
     * - "frequency-acceptance": maximum allowed frequency distance of point to an extrapolated line (in Hz)
     * - "time-gap-tolerance": max time gap before track is deemed over.
     * - "initial-frequency-acceptance": if the line that a point is being compared to, only has a single point so far, this is the accepted frequency acceptance. Default is frequency_acceptance
     * - "initial-time-acceptance": if the line that a point is being compared to, only has a single point so far, this is the accepted time window. Default is time-gap-tolerance
     * - "initial-slope": if a line has only one point, this is the line's slope
     * - "rel-slope-diff-to-expand" If the relative difference between the local slopes at the previous two time slices was greater than this, double the frequency acceptance. To recover from adding noise or to follow resonance.
     * - "n-slope-slices": maximum number of TIME SLICES to include in the slope calculation
     * - "min-points": a line only gets converted to a track if it has collected more than this many number of points
     * - "max-points": lines will be terminated after this many points. Good for curved tracks/resonances.
     * - "min-slope": a line only gets converted to a track if its slope is > than this slope (in Hz/s)

     * @section slots Slots:
     * - "disc-1d": void (KTDataPtr) -- clusters discriminated points to sequential lines candidates
     * - "done": void () -- Processes remaining active lines and emits clustering-done signal

     * @section signals Signals:
     * - "long-track-cand": void (KTDataPtr) -- Emitted when a candidate is ready; guarantees KTLongTrackData
     * - "clustering-done": void () -- Emitted when track clustering is complete
    */


    class KTLongTrackFinder : public Nymph::KTProcessor
    {
		public:

			/**
			 * @struct KTDiscriminedPointFrequencySorter
			 * @brief Comparator for sorting discriminated points by frequency.
			 *
			 * @details
			 * Function object that sorts points by frequency.
			 * This allows us to find points near to existing tracks efficiently.
			*/
			struct KTDiscriminedPointFrequencySorter
			{
				/**
				 * @brief Comparison operator
				 * @param lhs Left-hand side point
				 * @param rhs Right-hand point.
				 * @return true if lhs should be ordered before rhs, else false
				 */
				bool operator() (
						const KTDiscriminatedPoints1DData::Point& lhs, 
						const KTDiscriminatedPoints1DData::Point& rhs) const
				{
					return lhs.fAbscissa < rhs.fAbscissa;
				}
			};

			/// Type alias for frequency-sorted discriminated points container
			typedef std::set< KTDiscriminatedPoints1DData::Point, KTDiscriminedPointFrequencySorter > STFFrequencySortedPoints;


		public:
			/**
			 * @brief Constructor
			 * @param name Processor instance name
			 */
			KTLongTrackFinder(const std::string& name = "track-finder");

			/**
			 * @brief Destructor
			 */
			virtual ~KTLongTrackFinder();

			/**
			 * @brief Configure the processor.
			 * @param node Configuration parameter node.
			 * @return true on successful configuration.
			 */
			bool Configure(const scarab::param_node* node);

		public:
			// Parameters for point collection
			MEMBERVARIABLE(double, InitialSlope);
			MEMBERVARIABLE(signed, NSlopeSlices);
			MEMBERVARIABLE(double, FrequencyAcceptance);
			MEMBERVARIABLE(double, InitialFrequencyAcceptance);
			MEMBERVARIABLE(double, InitialTimeAcceptance);
			MEMBERVARIABLE(double, TimeGapTolerance);
			MEMBERVARIABLE(double, RelSlopeDiffToExpand);

			// Parameters for line post-processing
			MEMBERVARIABLE(unsigned, MinPoints);
			MEMBERVARIABLE(unsigned, MaxPoints);
			MEMBERVARIABLE(double, MinSlope);

			// Other configuration
			MEMBERVARIABLE(unsigned, MinBin);
			MEMBERVARIABLE(unsigned, MaxBin);
			MEMBERVARIABLE(bool, CalculateMinBin);
			MEMBERVARIABLE(bool, CalculateMaxBin);
			MEMBERVARIABLE(double, FreqBinWidth);
			MEMBERVARIABLE(double, TimeBinWidth);
			MEMBERVARIABLE(double, MinFrequency);
			MEMBERVARIABLE(double, MaxFrequency);

			// Internal tracking of number of candidates emitted
			MEMBERVARIABLE_PROTECTED(unsigned, NCandidatesEmitted);

		public:
			/**
			 * @brief Initialize processor using egg header information
			 * @param header Egg header object
			 * @return true on success
			 */
			bool InitializeWithHeader(KTEggHeader& header);

			/**
			 * @brief Collect discriminated points from a slice.
			 * @param slHeader Slice header.
			 * @param discrimPoints Discriminated points data.
			 * @return true on success
			 */
			bool CollectDiscrimPointsFromSlice(
					KTSliceHeader& slHeader, 
					KTDiscriminatedPoints1DData& discrimPoints);

			/**
			 * @brief Emit a pre-candidate track.
			 * @param track Track data to emit
			 */
			void EmitPreCandidate(KTLongTrackData& track);

			/**
			 * @brief Handle acquisition completion.
			 */
			void AcquisitionIsOver();

			/**
			 * @brief Get track candidates.
			 * @return Set of candidate data pointers.
			 */
			const std::set< Nymph::KTDataPtr >& GetCandidates() const;

		private:
			//***************
			// Tracks
			//***************
			// Tracks are implicitly sorted from oldest to newest
			/// Active track lines (oldest to newest)
			std::list<KTLongTrackData> fActiveLines;

			/// Completed candidate tracks
			std::set< Nymph::KTDataPtr > fCandidates;

		private:
			//***************
			// Signals
			//***************
			/// TODO: DOC
			Nymph::KTSignalData fLineSignal;

			/// TODO: DOC
			Nymph::KTSignalOneArg< void > fClusterDoneSignal;

		private:
			//***************
			// Slots
			//***************

			/// TODO: DOC
			Nymph::KTSlotDataOneType< KTEggHeader > fHeaderSlot;

			/// TODO: DOC
			Nymph::KTSlotDataTwoTypes< KTSliceHeader, KTDiscriminatedPoints1DData > fDiscrimSlot;

			/// TODO: DOC
			Nymph::KTSlotDone fDoneSlot;

			/**
			 * @brief Handle a finished track
			 * @param track Finished track
			 */
			void HandleFinishedTrack(KTLongTrackData& track);

			/**
			 * @brief Check if a new point matches existing line candidate.
			 * @param track Existing line candidate to compare to.
			 * @param newTime Time coordinate of new point.
			 * @param newFrequency Frequency coordinate of new point.
			 * @return true if match, else false.
			 */
			bool DoesPointMatchLine(const KTLongTrackData& track, 
									double newTime, 
									double newFrequency) const;

			/**
			 * @brief Add matching points to existing tracks
			 * @param points Available points from STF sorted by frequency.
			 * @param tracks Active tracks.
			 * @param timeInRunC Time in run.
			 * @param timeInAcqC Time in acquisition.
			 * @param acqID Acquisition ID.
			 */
			void AddPointsToExistingTracks(
					STFFrequencySortedPoints &points, 
					std::list<KTLongTrackData> &tracks, 
					double timeInRunC, 
					double timeInAcqC, 
					int acqID) const;

			/**
			 * @brief Retrieve points near a given track.
			 * @param sortedPoints Available points from STF sorted by frequency.
			 * @param track Track to compare against.
			 * @param timeInRunC Time in run.
			 * @return Vector of nearby points
			 */
			std::vector<KTDiscriminatedPoints1DData::Point> GetPointsNearTrack(
					const STFFrequencySortedPoints& sortedPoints, 
					const KTLongTrackData& track, 
					double timeInRunC) const;

			/**
			 * @brief Create new tracks from STF points.
			 * @param points Frequency-sorted points from STF
			 * @param timeInRunC Time in run.
			 * @param timeInAcqC Time in acquisition.
			 * @param acqID Acquisition ID.
			 * @return List of new tracks
			 */
			std::list<KTLongTrackData> CreateNewTracks(
					STFFrequencySortedPoints &points, 
					double timeInRunC, 
					double timeInAcqC, 
					int acqID) const;

			/**
			 * @brief Calculate the local slope of a track from a vector of points in the track.
			 * @param points Vector of points (t, f) forming a track.
			 * @return Local slope value in Hz/s (TODO: double check units)
			 */
			double CalculateLocalSlope(const std::vector<std::pair<double, double>>& points) const;

			/**
			 * @brief Create a track point from a discriminated point.
			 * @param point Input discriminated point.
			 * @param timeInRunC Time in run.
			 * @param timeInAcqC Time in acquisition.
			 * @param acqID Acquisition ID.
			 * @param trackFinderSlope Current slope estimate.
			 * @return Constructed track point.
			 */
			static KTLongTrackData::Point CreatePoint(
					const KTDiscriminatedPoints1DData::Point &point, 
					double timeInRunC, 
					double timeInAcqC, 
					int acqID, 
					double trackFinderSlope) ;
    };
    inline const std::set< Nymph::KTDataPtr >& KTLongTrackFinder::GetCandidates() const
    {
        return fCandidates;
    }

} /* namespace Katydid */
#endif /* KTLONGTRACKFINDER_HH_ */
