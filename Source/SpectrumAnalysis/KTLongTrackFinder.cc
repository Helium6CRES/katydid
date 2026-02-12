/*
 * @file KTLongTrackFinder.cc
 * @brief KTLongTrackFinder processor
 * @details Finds and creates track from discriminator data
 *  Builds track candidates slice-by slice by trying to add over-threshold points to active tracks.
 *  When track is found to end, it is emitted as a new Data Pointer of type KTLongTrackData. Each track has bulk properties calculated and also contains all constituent points. Each point has properties: Frequency, TimeInRunC, TimeInAcqC, AcquisitionID, Ordinate, Threshold, NSP, NoiseMean, NoiseTau, NoiseVariance, LocalSlope
 *
 * Documentation WIP by L. Malavasi 2/11/26, todos are notes to self
 * @author A. Gorman
 * @author H.S. Harrington
 * @date: March 7, 2024
 */

// TODO: what is a candidate vs a pre-candidate?

#include "KTLongTrackFinder.hh"

#include "KTLogger.hh"

#include "KTEggHeader.hh"
#include "KTSliceHeader.hh"
#include "KTDiscriminatedPoints1DData.hh"

#include <cmath>

using std::vector;


namespace Katydid
{
    KTLOGGER(stflog, "KTLongTrackFinder");

    KT_REGISTER_PROCESSOR(KTLongTrackFinder, "long-track-finder");

	/**
	 * @brief Construct a KTLongTrackFinder processor.
	 * @details
	 * Initializes configuration parameters, internal state, signals,
	 * and slots required for track finding.
	 *
	 * @param name Name of this processor instance.
	 */
    KTLongTrackFinder::KTLongTrackFinder(const std::string& name) :
            KTProcessor(name),
            fTimeGapTolerance(0.0005),
            fFrequencyAcceptance(56166.0528183),
            fRelSlopeDiffToExpand(0.2),
            fInitialFrequencyAcceptance(0.0),
            fInitialTimeAcceptance(0.0),
            fMinPoints(3),
            fMaxPoints(1e8),
            fMinSlope(0.0),
            fInitialSlope(3.0*pow(10,8)),
            fNSlopeSlices(8),
            fMinBin(0),
            fMaxBin(1),
            fFreqBinWidth(0.0),
            fTimeBinWidth(0.0),
            fMinFrequency(0.),
            fMaxFrequency(1.),
            fCalculateMinBin(true),
            fCalculateMaxBin(true),
            fActiveLines(),
            fNCandidatesEmitted(0),
            fLineSignal("long-track-cand", this),
            fClusterDoneSignal("clustering-done", this),
            fHeaderSlot("header", this, &KTLongTrackFinder::InitializeWithHeader),
            fDiscrimSlot("disc-1d", this, &KTLongTrackFinder::CollectDiscrimPointsFromSlice),
            fDoneSlot("done", this, &KTLongTrackFinder::AcquisitionIsOver, &fClusterDoneSignal)
    {
    }

	/**
	 * @brief Destructor
	 */
    KTLongTrackFinder::~KTLongTrackFinder()
    {
    }

	/**
	 * @brief Configure the processor from a parameter node.
	 *
	 * @details
	 * Reads configuration values and sets internal parameters controlling LongTrackFinder behavior.
	 *
	 * @param param_node Pointer to configuration parameter node.
	 * @return true if configuration succeeds
	 * @return false if configuration fails or if node is NULL
	 */
    bool KTLongTrackFinder::Configure(const scarab::param_node* node)
    {
        if (node == NULL) return false;

        SetMinFrequency(node->get_value("min-frequency", GetMinFrequency()));
        SetMaxFrequency(node->get_value("max-frequency", GetMaxFrequency()));
        SetRelSlopeDiffToExpand(node->get_value("rel-slope-diff-to-expand", GetRelSlopeDiffToExpand()));

        SetMinPoints(node->get_value("min-points", GetMinPoints()));
        SetMaxPoints(node->get_value("max-points", GetMaxPoints()));
        SetMinSlope(node->get_value("min-slope", GetMinSlope()));

        SetTimeGapTolerance(node->get_value("time-gap-tolerance", GetTimeGapTolerance()));
        SetFrequencyAcceptance(node->get_value("frequency-acceptance", GetFrequencyAcceptance()));
        SetInitialSlope(node->get_value("initial-slope", GetInitialSlope())); // MODIFY GetInitialSlope FOR LARMOR SLOPE CALCULATION IF INITIAL SLOPE NOT SPECIFIED

        if (node->has("min-bin"))
        {
            SetMinBin(node->get_value< unsigned >("min-bin"));
            SetCalculateMinBin(false);
        }
        if (node->has("max-bin"))
        {
            SetMaxBin(node->get_value< unsigned >("max-bin"));
            SetCalculateMaxBin(false);
        }
        if (node->has("initial-frequency-acceptance"))
        {
            SetInitialFrequencyAcceptance(node->get_value("initial-frequency-acceptance", GetInitialFrequencyAcceptance()));
        }
        else
        {
            SetInitialFrequencyAcceptance(node->get_value("frequency-acceptance", GetInitialFrequencyAcceptance()));
        }
        if (node->has("initial-time-acceptance"))
        {
            SetInitialTimeAcceptance(node->get_value("initial-time-acceptance", GetInitialTimeAcceptance()));
        }
        else
        {
            SetInitialTimeAcceptance(node->get_value("time-gap-tolerance", GetInitialTimeAcceptance()));
        }
        if (node->has("n-slope-points"))
        {
            SetNSlopeSlices(node->get_value("n-slope-slices", GetNSlopeSlices()));
        }

        return true;
    }

	/**
	 * @brief Initialize processor using acquisition header information.
	 * @details
	 * Extracts time bin width and frequency bin width from acquisition rate and FFT slice size.
	 *
	 * @param header Egg header containing acquisition metadata.
	 * @return true on success
	 */
    bool KTLongTrackFinder::InitializeWithHeader(KTEggHeader& header)
    {
        fTimeBinWidth = 1. / header.GetAcquisitionRate();
        fFreqBinWidth = 1. / (fTimeBinWidth * header.GetChannelHeader(0)->GetSliceSize());

        return true;
    }


	/**
	 * @brief Process discriminated points in a single time slice
	 * @details Identifies points within frequency acceptance of existing line candidates. Start new line candidates at any line that is not matched up to existing line candidates.
	 * TODO: this seems to do similar things to DoesPointMatchLine and GetPointsNearTrack, review with Heather what the difference is and where each lives in the analysis chain.
	 *
	 * @param slHeader Slice header containing time metadata.
	 * @param discrimPoints Discriminated point data from the slice.
	 * @return true on successful processing.
	 */
    bool KTLongTrackFinder::CollectDiscrimPointsFromSlice(
			KTSliceHeader& slHeader, 
			KTDiscriminatedPoints1DData& discrimPoints)
    {
        unsigned minBin = fCalculateMinBin ? fMinFrequency / (double) slHeader.GetBinWidth() : fMinBin;
        unsigned maxBin = fCalculateMaxBin ? fMaxFrequency / (double) slHeader.GetBinWidth() : fMaxBin;

        fFreqBinWidth = slHeader.GetBinWidth();
        fTimeBinWidth = slHeader.GetSliceLength();
        KTDEBUG( stflog, "fTimeBinWidth "<<fTimeBinWidth<<" fFreqBinWidth "<<fFreqBinWidth);


		// TODO: what is a "component"? This could be more descriptive...
		// TODO: having a hard time following the logical flow in this block
        unsigned nComponents = 1;

        for (unsigned iComponent = 0; iComponent < nComponents; ++iComponent)
        {
			// Record time at center of time slice. Header records left edge, add half of slice length.
            double timeInRunC = slHeader.GetTimeInRun() + 0.5 * slHeader.GetSliceLength();
            double timeInAcqC = slHeader.GetTimeInAcq() + 0.5 * slHeader.GetSliceLength();
            int acqID = slHeader.GetAcquisitionID();

			// select discriminated points with frequency bin between minBin and maxBin
			// TODO: are point.first and point.second time and frequency coordinates or something else? Confused. Could be more descriptive
            STFFrequencySortedPoints points;
            for (auto& point : discrimPoints.GetSetOfPoints(iComponent))
            {
                KTDEBUG( stflog, "Min Bin "<<minBin<<" Max Bin "<<maxBin<<" Point bin: "<<point.first);
                if (point.first >= minBin and point.first <= maxBin)
                {
                    points.insert(point.second);
                }
            }

            KTDEBUG( stflog, "Collected "<<points.size()<<" points");

			// Assign discriminated points to existing track candidates
            AddPointsToExistingTracks(points, fActiveLines, timeInRunC, timeInAcqC, acqID);

			
			// Iterate over active lines including points from new slice
			// If new point is more than TimeGapTolerance from previous point in track
			// or number of points in track is greater than MaxPoints (not currently used, 
			// fMaxPoints = 1e8 to ignore it) then end the active line not incl. new point.
            for(auto trackIt = fActiveLines.begin(); trackIt != fActiveLines.end();) {
                if(timeInRunC - trackIt->GetPoints().back().TimeInRunC > fTimeGapTolerance 
						or trackIt->GetPoints().size() >= fMaxPoints) {
                    KTDEBUG(stflog, "trackIt->GetPoints().back().TimeInRunC "<<trackIt->GetPoints().back().TimeInRunC);
                    HandleFinishedTrack(*trackIt); // let the vector sort itself by earliest track instead of longest track
                    trackIt = fActiveLines.erase(trackIt);
                } else {
                    trackIt++;
                }
            }

			// create new track candidates from the remaining points, add to list of active lines
            auto newTracks = CreateNewTracks(points, timeInRunC, timeInAcqC, acqID);
            fActiveLines.splice(fActiveLines.end(), newTracks);
        }
        return true;
    }

	/**
	 * @brief Create new tracks from SequentialTrackFinder points
	 * @details Start new track candidates from points that have not been identified as part of an active track candidate
	 *
	 * @param points Frequency-sorted points from SequentialTrackFinder
	 * @param timeInRunC Time in run.
	 * @param timeInAcqC Time in acquisition.
	 * @param acqID Acquisition ID.
	 * @return List of new tracks
	 */
    std::list<KTLongTrackData> KTLongTrackFinder::CreateNewTracks(
			STFFrequencySortedPoints& points, 
			double timeInRunC, 
			double timeInAcqC, 
			int acqID) const 
	{
        auto newTracks = std::list<KTLongTrackData>();

        // TODO: Pick out best point from cluster, like we do when adding to existing tracks. For now, all non-claimed points.
        for(auto pointIt = points.begin(); pointIt != points.end(); pointIt = points.erase(pointIt)) {
            KTLongTrackData newLine;
            newLine.AddPoint(CreatePoint(*pointIt, timeInRunC, timeInAcqC, acqID, fInitialSlope));
            newTracks.push_back(newLine);
        }

        if(!newTracks.empty() ) { KTWARN(stflog, "Starting "<< newTracks.size() << " new tracks") }

        return newTracks;
    }

	/**
	 * @brief Adds matching points to existing tracks
	 * @details 
	 * 1. Gather all points (track + matching) into one structure
	 * 2. Select points from most recent fNSlopeSlices unique time slices
	 * 3. Compute the slope
	 *
	 * @param points Points in slice which match existing tracks.
	 * @param tracks Tracks to which to append points.
	 * @param timeInRunC Time in run.
	 * @param timeInAcqC Time in acquisition.
	 * @param acqID Acquisition ID.
	 */
    void KTLongTrackFinder::AddPointsToExistingTracks(
            STFFrequencySortedPoints& points,
            std::list<KTLongTrackData>& tracks,
            double timeInRunC, 
			double timeInAcqC, 
			int acqID) const 
	{
        for (auto& track : tracks) {
			// Break early when we run out of points
            if(points.empty()) { 
				break; 
			}  
			// KTDEBUG(stflog, "Checking for matched for track with track.GetPoints().size() "<<track.GetPoints().size());

            std::vector<KTDiscriminatedPoints1DData::Point> matchingPoints = GetPointsNearTrack(points, track, timeInRunC);

            if(matchingPoints.size()>0){
                KTDEBUG(stflog, "Number of matching points: " << matchingPoints.size());
            }
            
            if(!matchingPoints.empty()) {
                for (const auto& p : matchingPoints) {
                    KTDEBUG(stflog, "Frequency: " << p.fAbscissa 
							     << ", Power: " << p.fOrdinate
								 << ", Tau: " << p.fTau
								 << ", SNR: " << p.fOrdinate/p.fTau);
                }

                // 1. Gather all points (track + matching) into one structure
                //
				// This map groups time slices (TimeInRunC) → list of (TimeInRunC, Frequency) pairs.
				// Keys: TimeInRunC values (the slice centers)
                // Values: std::vector<std::pair<double, double>>, i.e., all (TimeInRunC, Frequency) points in that slice
                // std::greater<> ensures descending time order (most recent first)
                std::map<double, std::vector<std::pair<double, double>>, std::greater<>> timeToTimeFreqPairs;

                // Track points: KTLongTrackData::Point
                for (const auto& p : track.GetPoints()) {
                    timeToTimeFreqPairs[p.TimeInRunC].emplace_back(p.TimeInRunC, p.Frequency);
                }

                // Matching points: KTDiscriminatedPoints1DData::Point
                for (const auto& p : matchingPoints) {
                    timeToTimeFreqPairs[timeInRunC].emplace_back(timeInRunC, p.fAbscissa);  // fAbscissa is the frequency
                }

                // 2. Select points from most recent fNSlopeSlices unique time slices
                std::vector<std::pair<double, double>> slopeCalcPoints;
                int slicesIncluded = 0;
                for (const auto& timeFreqPair : timeToTimeFreqPairs) {
                    const auto& time = timeFreqPair.first;
                    const auto& tfPairs = timeFreqPair.second;
                    slopeCalcPoints.insert(slopeCalcPoints.end(), tfPairs.begin(), tfPairs.end());
                    if (++slicesIncluded >= fNSlopeSlices) break;
                }

                // 3. Compute the slope
                double localSlope = CalculateLocalSlope(slopeCalcPoints);
                //track.AddPoint(CreatePoint(*bestPoint, timeInRunC, localSlope));
                
                // Consume all points that were near the track. This is because tracks are
                // assumed to be spaced quite far apart, so it's just easier to use a more naive algorithm here.
                for (auto& matchingPoint : matchingPoints) {
                    track.AddPoint(CreatePoint(matchingPoint, timeInRunC, timeInAcqC, acqID, localSlope));
                    points.erase(matchingPoint); 
                }
                if(matchingPoints.size()>0){
                    KTDEBUG(stflog, "Number of points left in slice after adding those to track: " << points.size());
                }
                

                // Throw out all other points that were near the track, even if we didn't select them. This is because tracks are
                // assumed to be spaced quite far apart, so it's just easier to use a more naive algorithm here.
                //for (auto& matchingPoint : matchingPoints) { points.erase(matchingPoint); }

            }
        }
    }



	/**
	 * @brief Constructs a vector of all points near a track in a slice.
	 * @details
	 * Iterates over all points and adds them to the output vector if the point matches the line according toKTLongTrackFinder::DoesPointMatchLine().
	 *
	 * @param sortedPoints Frequency-sorted points from in one slice SequentialTrackFinder. 
	 * @param track Track data to compare against.
	 * @param timeInRunC Slice time in run.
	 * @return Vector of points near track to consider.
	 */
    std::vector<KTDiscriminatedPoints1DData::Point> KTLongTrackFinder::GetPointsNearTrack(
            const STFFrequencySortedPoints& sortedPoints, 
			const KTLongTrackData& track, 
			double timeInRunC) const 
	{

        // We can do some cool binary search or whatever to speed things up.
        // This will just look at every point to find matching points
        std::vector<KTDiscriminatedPoints1DData::Point> pointsToConsider;
        for (auto& point : sortedPoints) {
            if(DoesPointMatchLine(track, timeInRunC, point.fAbscissa)) {
                pointsToConsider.push_back(point);
                KTDEBUG(stflog, "Added point to line!");
            } 
        }

        return pointsToConsider;
    }


	/**
	 * @brief Determines if a new point matches an existing track candidate.
	 * @details Decision tree to determine if a new point should belong to an active track candidate. For tracks with more than 2 points, compute previous local slopes and project frequency acceptance band. Add point if it falls within frequency acceptance and does not lower minimum slope below fMinSlope.
	 * Fallback for tracks with 2 or fewer points: just decide based on if new point falls within frequency acceptance based on most recent point group. Slopes can be noisy with too few points. 
	 *
	 * @param track Existing line candidate to compare against.
	 * @param newTime Time coordinate of new point.
	 * @param newFrequency Frequency coordinate of new point.
	 * @return true if point matches
	 */
    bool KTLongTrackFinder::DoesPointMatchLine(
			const KTLongTrackData& track, 
			double newTime, 
			double newFrequency) const 
	{
        auto trackPoints = track.GetPoints();
        double effectiveAcceptance = fFrequencyAcceptance;
        double predictedActualFrequencyDelta = std::numeric_limits<double>::max();

        if (trackPoints.size() >= 3) {
            // Group points by TimeInRunC (descending)
            std::map<double, std::vector<const Katydid::KTLongTrackData::Point*>, std::greater<>> timeToPoints;
            for (auto it = trackPoints.rbegin(); it != trackPoints.rend(); ++it) {
                timeToPoints[it->TimeInRunC].push_back(&(*it));
                if (timeToPoints.size() >= 2) break;
            }

            // Median freq and slope from last two slices
            std::vector<std::pair<double, double>> medFreqAndSlope;
            for (const auto& time_pointPair : timeToPoints) {
                const double& time = time_pointPair.first;
                const auto& pointPtrs = time_pointPair.second;

                std::vector<double> freqs;
                for (const auto* pt : pointPtrs) {
                    freqs.push_back(pt->Frequency);
                }

                std::sort(freqs.begin(), freqs.end());
                double medianFreq = (freqs.size() % 2 == 1)
                    ? freqs[freqs.size() / 2]
                    : 0.5 * (freqs[freqs.size() / 2 - 1] + freqs[freqs.size() / 2]);

                double slope = pointPtrs.front()->TrackFinderLocalSlope;
                medFreqAndSlope.emplace_back(medianFreq, slope);
            }

            // Compare slopes, expand effective acceptance if there is a significant difference
            double slope0 = medFreqAndSlope[0].second;
            double slope1 = medFreqAndSlope[1].second;
            double relativeSlopeDiff = std::abs(slope0 - slope1) / std::abs(slope1);

            if (relativeSlopeDiff > fRelSlopeDiffToExpand) {
                effectiveAcceptance *= 2.0;
                KTDEBUG(stflog, "Previous slopes differ significantly (" << slope0 << " vs " << slope1 << "). Doubling acceptance to " << effectiveAcceptance);
            }

            // Predict frequency using most recent median + slope
            double t0 = timeToPoints.begin()->first;
            double f0 = medFreqAndSlope[0].first;
            double slope = medFreqAndSlope[0].second;
            double predictedFrequency = f0 + slope * (newTime - t0);
            predictedActualFrequencyDelta = std::abs(newFrequency - predictedFrequency);

            // Final decision:
			// 1. If predicted frequency is within the acceptance band, AND
			// 2. If new slope from adding this point is NOT less than fMinSlope
            if (predictedActualFrequencyDelta < effectiveAcceptance) {
                KTDEBUG(stflog, "Point matches track. Acceptance: " << effectiveAcceptance
                           << ", predicted delta: " << predictedActualFrequencyDelta);
                // Final slope check including the new point
                std::vector<std::pair<double, double>> slopeCalcPoints;

                // Add all points from the last fNSlopeSlices slices
                int slicesIncluded = 0;
                for (auto it = timeToPoints.begin(); it != timeToPoints.end(); ++it) {
                    const double& time = it->first;
                    const std::vector<const Katydid::KTLongTrackData::Point*>& pointPtrs = it->second;

                    for (const auto* pt : pointPtrs) {
                        slopeCalcPoints.emplace_back(pt->TimeInRunC, pt->Frequency);
                    }

                    if (++slicesIncluded >= fNSlopeSlices) break;
                }

                slopeCalcPoints.emplace_back(newTime, newFrequency);

                // Calculate the new local slope that would result from just adding this point. If < fMinSlope, don't add it!
                double newSlope = CalculateLocalSlope(slopeCalcPoints);
                if (std::abs(newSlope) < fMinSlope) {
                    KTDEBUG(stflog, "New slope after adding point is too small: " << newSlope << " < " << fMinSlope);
                    return false;
                }
                // Add new point
                return true;
            }

            return false;
        }
        else {
            // Fallback for short tracks: use most recent point group only
            std::map<double, std::vector<const Katydid::KTLongTrackData::Point*>, std::greater<>> timeToPoints;
            for (auto it = trackPoints.rbegin(); it != trackPoints.rend(); ++it) {
                timeToPoints[it->TimeInRunC].push_back(&(*it));
                if (timeToPoints.size() >= 1) break;
            }

            auto& pointPtrs = timeToPoints.begin()->second;
            std::vector<double> freqs;
            for (const auto* pt : pointPtrs) {
                freqs.push_back(pt->Frequency);
            }

            std::sort(freqs.begin(), freqs.end());
            double medianFreq = (freqs.size() % 2 == 1)
                ? freqs[freqs.size() / 2]
                : 0.5 * (freqs[freqs.size() / 2 - 1] + freqs[freqs.size() / 2]);

            double t0 = timeToPoints.begin()->first;
            double slope = pointPtrs.front()->TrackFinderLocalSlope;
            double predictedFrequency = medianFreq + slope * (newTime - t0);
            predictedActualFrequencyDelta = std::abs(newFrequency - predictedFrequency);

            // Final decision
            if (predictedActualFrequencyDelta < effectiveAcceptance) {
                KTDEBUG(stflog, "Point matches track. Acceptance: " << effectiveAcceptance
                           << ", predicted delta: " << predictedActualFrequencyDelta);
                // Add new point
                return true;
            }

            return false;
        }

    }

	/**
	 * @brief Handle and emit or discard a finished track
	 * @details
	 * If track is sufficiently long (# points > fMinPoints) and slope > min slope, then emit the line pre-candidate.
	 *
	 * @param track Finished track.
	 */
    void KTLongTrackFinder::HandleFinishedTrack(KTLongTrackData& track) {
        if (track.GetPoints().size() >= fMinPoints and track.GetBulkSlope() >= fMinSlope) {
            KTWARN(stflog, "Found line candidate");
            EmitPreCandidate(track);
        }
    }

	/**
	 * @brief Routine for emitting a line pre-candidate
	 * @details
	 * Set up a Nymph data object. Set its track/event ID, other metadata. Write track points and compute track statistics.
	 *
	 * @param track Finished track to emit as a pre-candidate.
	 */
    void KTLongTrackFinder::EmitPreCandidate(KTLongTrackData& track)
    {
        KTDEBUG(stflog, "emitting candidate");
        KTDEBUG(stflog, "emitting candidate with " << track.GetPoints().size() << " points");

        // Set up new data object
        Nymph::KTDataPtr data(new Nymph::KTData());
        auto& newCand = data->Of<KTLongTrackData>();
        newCand.SetTrackId(fNCandidatesEmitted);
        newCand.SetEventId(fNCandidatesEmitted); //Set the eventID to the trackID for now, can be re-assigned by a later event-builder
        newCand.SetBandNumber(0); //All set to 0 (main band) for now, can be re-assigned by a later event-builder
        newCand.SetPoints(track.GetPoints());
        // Compute and store track stats
        newCand.CalculateTrackStats(fTimeBinWidth,fFreqBinWidth);

        ++fNCandidatesEmitted;

        KTDEBUG(stflog, "Emitting track signal");
        fCandidates.insert(data);
        fLineSignal(data);
    }

	/**
	 * @brief Handle acquisition completeion
	 * @details Handles any remaining track candidates that have not been emitted at the end of acquisition.
	 */
    void KTLongTrackFinder::AcquisitionIsOver()
    {
        KTINFO(stflog, "Got egg-done signal. Checking remaining line candidates");

        auto lineIt = fActiveLines.begin();
        while( lineIt != fActiveLines.end())
        {
	 		// TODO: this code seems redundant with HandleFinishedTrack, can it be simplified?
            if (lineIt->GetPoints().size() >= fMinPoints and lineIt->GetPoints().size() <= fMaxPoints
                and lineIt->GetBulkSlope() > fMinSlope) {
                EmitPreCandidate(*lineIt);
            }

            lineIt = fActiveLines.erase(lineIt);
        }
        KTDEBUG(stflog, "Now there should be no lines left over " << fActiveLines.empty());
    }

	/**
	 * @brief Calculates the slope of the end of the track using least squares.
	 * @details
	 * Takes a vector of (time,frequency) pairs, ordered by time, oldest to most recent
	 *
	 * @param points Points in track
	 * @return Local slope value in Hz/s (TODO: double check units)
	 */
    double KTLongTrackFinder::CalculateLocalSlope(const std::vector<std::pair<double, double>>& points) const {
        for (const auto& p : points) {
            KTDEBUG(stflog, "Time: " << p.first << ", Frequency: " << p.second);
        }
        if(points.size() == 1) {
            return fInitialSlope;
        }

        //int maxPoints = std::min((int) points.size(), 5);
        int maxPoints = (int) points.size();

        auto&& pointIt = points.begin(); //DONT want to do this in reverse!
        double sumX = 0, sumY = 0, sumXY = 0, sumXX = 0;
        for (int i = 0; i < maxPoints; i++) {
            sumX += pointIt->first;
            sumY += pointIt->second;
            sumXY += pointIt->first * pointIt->second;
            sumXX += pointIt->first * pointIt->first;
            pointIt++;
        }
        KTDEBUG(stflog, "CalcLocalSlope: " << (maxPoints * sumXY - sumX * sumY)/(sumXX * maxPoints - sumX * sumX));
        return (maxPoints * sumXY - sumX * sumY)/(sumXX * maxPoints - sumX * sumX);
    }

	/**
	 * @brief Create a track point from a discriminated point.
	 * @details Constructs KTLongTrackData::Point from KTDiscriminatedPoints1DData::Point and track information.
	 * @param point Input discriminated point.
	 * @param timeInRunC Time in run.
	 * @param timeInAcqC Time in acquisition.
	 * @param acqID Acquisition ID.
	 * @param trackFinderSlope Current slope estimate.
	 * @return Constructed track point.
	 */
	KTLongTrackData::Point KTLongTrackFinder::CreatePoint(
            const KTDiscriminatedPoints1DData::Point &point, 
			double timeInRunC, 
			double timeInAcqC, 
			int acqID, 
			double trackFinderSlope) {
        return {
                timeInRunC,
                timeInAcqC,
                acqID,
                point.fAbscissa,
                point.fOrdinate,
                point.fThreshold,
                point.fOrdinate / point.fTau,
                point.fMean,
                point.fTau,
                point.fVariance,
                trackFinderSlope};
    }

} /* namespace Katydid */
