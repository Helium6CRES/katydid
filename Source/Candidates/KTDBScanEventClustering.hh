/**
 @file KTDBScanEventClustering.hh
 @brief Contains KTDBScanEventClustering
 @details Clusters tracks into events
 @author: N.S. Oblath
 @date: Aug 4, 2014
 */

#ifndef KTDBSCANEVENTCLUSTERING_HH_
#define KTDBSCANEVENTCLUSTERING_HH_

#include "KTPrimaryProcessor.hh"

#include "KTDBScan.hh"
#include "KTSlot.hh"
#include "KTData.hh"

#include <algorithm>
#include <set>
#include <vector>

namespace Katydid
{
    class KTParamNode;
    class KTProcessedTrackData;

    // Track distance
    // Vector format for representing tracks: (tstart, fstart, tend, fend)
    // Dimension t: for tstart_1 < tstart_2, Dt = max(0, tstart_2 - tend_1)
    // Dimension f: Df = fstart_2 - fend_1
    // Dist = sqrt(Dt^2 + Df^2)
    template < typename VEC_T >
    class TrackDistance
    {
        protected:
            typedef VEC_T vector_type;

            double GetDistance(const VEC_T v1, const VEC_T v2)
            {
                double deltaT, deltaF;
                if (v1(0) < v2(0))
                {
                    deltaT = std::max(0., v2(0) - v1(2));
                    deltaF = v2(1) - v1(3);
                }
                else
                {
                    deltaT = std::max(0., v1(0) - v2(2));
                    deltaF = v1(1) - v2(3);
                }
                return sqrt(deltaT * deltaT + deltaF * deltaF);
            };

            double GetDistance(const VEC_T v1, const VEC_T v2, const VEC_T w)
            {
                double deltaT, deltaF;
                if (v1(0) < v2(0))
                {
                    deltaT = std::max(0., w(0)*v2(0) - w(2)*v1(2));
                    deltaF = w(1)*v2(1) - w(3)*v1(3);
                }
                else
                {
                    deltaT = std::max(0., w(0)*v1(0) - w(2)*v2(2));
                    deltaF = w(1)*v1(1) - w(3)*v2(3);
                }
                return sqrt(deltaT * deltaT + deltaF * deltaF);
            };
    };


    /*!
     @class KTDBScanEventClustering
     @author N.S. Oblath

     @brief Clustering for finding events using the DBSCAN algorithm

     @details
     Normalization of the axes:
     The DBSCAN algorithm expects expects that all of the dimensions that describe a points will have the same scale,
     such that a single radius parameter can describe a sphere in the parameter space that's used to cluster points together.
     For track clustering, two radii are specified, one for the time dimension and one for the frequency dimension.
     For clustering, a scaling factor is calculated for each axis such that the ellipse formed by the two radii is
     scaled to a unit circle.  Those scaling factors are applied to every point before the data is passed to the
     DBSCAN algorithm.

     Configuration name: "dbscan-event-clustering"

     Available configuration values:
     - "radii": double[2] -- array used to describe the distances that will be used to cluster tracks together; [time, frequency]
     - "min-points": unsigned int -- minimum number of tracks required to have a cluster

     Slots:
     - "track": void (shared_ptr<KTData>) -- If this is a new acquisition, triggers the clustering algorithm; Adds tracks to the internally-stored set of points; Requires KTSliceHeader and KTDiscriminatedPoints1DData.

     Signals:
     - "event": void (shared_ptr<KTData>) -- Emitted for each cluster found; Guarantees KT???Data.
    */

    class KTDBScanEventClustering : public KTPrimaryProcessor
    {
        public:
            typedef KTDBScan::Point DBScanPoint;
            typedef KTDBScan::Points DBScanPoints;
            typedef KTDBScan::Weights DBScanWeights;

            const static unsigned fNDimensions;
            const static unsigned fNPointsPerTrack;

        public:
            KTDBScanEventClustering(const std::string& name = "dbscan-event-clustering");
            virtual ~KTDBScanEventClustering();

            bool Configure(const KTParamNode* node);

            unsigned GetMinPoints() const;
            void SetMinPoints(unsigned pts);

            const DBScanWeights& GetRadii() const;
            void SetRadii(const DBScanWeights& radii);

        private:
            KTDBScan fDBScan;

            // dimension weighting
            DBScanWeights fRadii;

            //minimum number of points
            unsigned fMinPoints;

        public:
            // Store point information locally
            bool TakeTrack(KTProcessedTrackData& track);
            //bool TakeTrack(double startTime, double startFreq, double endTime, double endFreq, unsigned component=0);

            void SetNComponents(unsigned nComps);
            void SetTimeBinWidth(double bw);
            void SetFreqBinWidth(double bw);

            void TriggerClustering();

            bool Run();

            bool DoClustering();

            const std::set< KTDataPtr >& GetCandidates() const;
            unsigned GetDataCount() const;

        private:

            double fTimeBinWidth;
            double fFreqBinWidth;

            std::vector< std::vector< KTProcessedTrackData > > fCompTracks; // input tracks

            std::set< KTDataPtr > fCandidates;
            unsigned fDataCount;

            //***************
            // Signals
            //***************

        private:
            KTSignalData fEventSignal;
            KTSignalOneArg< void > fClusterDoneSignal;

            //***************
            // Slots
            //***************

        private:
            KTSlotDataOneType< KTProcessedTrackData > fTakeTrackSlot;
            //KTSlotDataOneType< KTInternalSignalWrapper > fDoClusterSlot;

    };

    inline unsigned KTDBScanEventClustering::GetMinPoints() const
    {
        return fMinPoints;
    }
    inline void KTDBScanEventClustering::SetMinPoints(unsigned pts)
    {
        fMinPoints = pts;
        return;
    }

    inline const KTDBScanEventClustering::DBScanWeights& KTDBScanEventClustering::GetRadii() const
    {
        return fRadii;
    }

    inline void KTDBScanEventClustering::SetTimeBinWidth(double bw)
    {
        fTimeBinWidth = bw;
        return;
    }
    inline void KTDBScanEventClustering::SetFreqBinWidth(double bw)
    {
        fFreqBinWidth = bw;
        return;
    }

    inline const std::set< KTDataPtr >& KTDBScanEventClustering::GetCandidates() const
    {
        return fCandidates;
    }
    inline unsigned KTDBScanEventClustering::GetDataCount() const
    {
        return fDataCount;
    }


}
 /* namespace Katydid */
#endif /* KTDBSCANEVENTCLUSTERING_HH_ */
