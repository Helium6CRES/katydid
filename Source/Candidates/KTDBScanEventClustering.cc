/*
 * KTDBScanEventClustering.cc
 *
 *  Created on: Aug 4, 2014
 *      Author: N.S. Oblath
 */

#include "KTDBScanEventClustering.hh"

#include "KTLogger.hh"
#include "KTMath.hh"
#include "KTMultiTrackEventData.hh"
#include "KTNOFactory.hh"
#include "KTParam.hh"
#include "KTProcessedTrackData.hh"

using std::set;
using std::vector;

namespace Katydid
{
    KTLOGGER(tclog, "katydid.fft");

    KT_REGISTER_PROCESSOR(KTDBScanEventClustering, "dbscan-event-clustering");

    // dimensions: (t_start, f_start, t_end, f_end)
    const unsigned KTDBScanEventClustering::fNDimensions = 4;
    // points in a track: (start, end)
    const unsigned KTDBScanEventClustering::fNPointsPerTrack = 2;

    KTDBScanEventClustering::KTDBScanEventClustering(const std::string& name) :
            KTPrimaryProcessor(name),
            fDBScan(),
            fRadii(fNDimensions / fNPointsPerTrack),
            fMinPoints(3),
            fTimeBinWidth(1),
            fFreqBinWidth(1.),
            fCompTracks(1, vector< KTProcessedTrackData >()),
            fCandidates(),
            fDataCount(0),
            fEventSignal("event", this),
            fClusterDoneSignal("cluster-done", this),
            fTakeTrackSlot("track", this, &KTDBScanEventClustering::TakeTrack)
    //        fDoClusterSlot("do-cluster-trigger", this, &KTDBScanEventClustering::Run)
    {
        RegisterSlot("do-cluster-trigger", this, &KTDBScanEventClustering::TriggerClustering);
        fRadii(0) = 1. / sqrt(fNDimensions);
        fRadii(1) = 1. / sqrt(fNDimensions);
    }

    KTDBScanEventClustering::~KTDBScanEventClustering()
    {
    }

    bool KTDBScanEventClustering::Configure(const KTParamNode* node)
    {
        if (node == NULL) return false;

        SetMinPoints(node->GetValue("min-points", GetMinPoints()));

        if (node->Has("radii"))
        {
            const KTParamArray* radii = node->ArrayAt("radii");
            if (radii->Size() != fNDimensions)
            {
                KTERROR(tclog, "Radii array does not have the right number of dimensions: <" << radii->Size() << "> instead of <" << fNDimensions << ">");
                return false;
            }
            fRadii(0) = radii->GetValue< double >(0);
            fRadii(1) = radii->GetValue< double >(1);
        }

        return true;
    }

    void KTDBScanEventClustering::SetRadii(const DBScanWeights& radii)
    {
        if (radii.size() != fNDimensions)
        {
            KTERROR(tclog, "Weights vector has the wrong number of dimensions: " << radii.size() << " != " << fNDimensions);
            return;
        }
        fRadii = radii;
        return;
    }

    bool KTDBScanEventClustering::TakeTrack(KTProcessedTrackData& track)
    {
        // verify that we have the right number of components
        if (track.GetComponent() >= fCompTracks.size())
        {
            SetNComponents(track.GetComponent() + 1);
        }

        // copy the full track data
        fCompTracks[track.GetComponent()].push_back(track);

        KTDEBUG(tclog, "Taking track: (" << track.GetStartTimeInRunC() << ", " << track.GetStartFrequency() << ", " << track.GetEndTimeInRunC() << ", " << track.GetEndFrequency());

        return true;
    }

    /*
    bool KTDBScanEventClustering::TakePoint(double time, double frequency *//*, double amplitude*//*, unsigned component)
    {
        if (component >= fCompPoints.size())
        {
            SetNComponents(component + 1);
        }

        KTDBScan::Point newPoint(fNDimensions);
        newPoint(0) = time;
        newPoint(1) = frequency;
        fCompPoints[component].push_back(newPoint);

        KTDEBUG(tclog, "Point " << fCompPoints[component].size()-1 << " is now " << fCompPoints[component].back());

        return true;
    }
    */

    void KTDBScanEventClustering::TriggerClustering()
    {
        if (! Run())
        {
            KTERROR(tclog, "An error occurred while running the clustering");
        }
        return;
    }

    bool KTDBScanEventClustering::Run()
    {
        return DoClustering();
    }

    bool KTDBScanEventClustering::DoClustering()
    {
        KTPROG(tclog, "Starting DBSCAN event clustering");

        fDBScan.SetRadius(1.);
        fDBScan.SetMinPoints(fMinPoints);
        KTINFO(tclog, "DBScan configured");

        for (unsigned iComponent = 0; iComponent < fCompTracks.size(); ++iComponent)
        {
            KTDEBUG(tclog, "Clustering component " << iComponent);

            if (fCompTracks[iComponent].empty() )
                continue;

            // calculate the scaling
            DBScanWeights scale = fRadii;
            for (DBScanPoint::iterator dIt = scale.begin(); dIt != scale.end(); ++dIt)
            {
                *dIt = 1. / (*dIt * KTMath::Sqrt2());
            }

            // new array for normalized points
            DBScanPoints normPoints(fCompTracks[iComponent].size());
            DBScanPoint newPoint;
            // normalize points
            KTDEBUG(tclog, "Scale: " << scale);
            unsigned iPoint = 0;
            //for (DBScanPoints::iterator pIt = fCompPoints[iComponent].begin(); pIt != fCompPoints[iComponent].end(); ++pIt)
            for (vector< KTProcessedTrackData >::const_iterator pIt = fCompTracks[iComponent].begin(); pIt != fCompTracks[iComponent].end(); ++pIt)
            {
                newPoint(0) = pIt->GetStartTimeInRunC() * scale(0); // start time
                newPoint(1) = pIt->GetStartFrequency() * scale(1);  // start freq
                newPoint(2) = pIt->GetEndTimeInRunC() * scale(0);   // end time
                newPoint(3) = pIt->GetEndFrequency() * scale(1);    // end freq

//#ifndef NDEBUG
//                std::stringstream ptStr;
//                ptStr << "Point -- before: " << *pIt;
//#endif
                //KTDEBUG(tclog, ptStr.str() << " -- after: " << newPoint);
                normPoints[iPoint++] = newPoint;
            }

            // do the clustering!
            if (! fDBScan.RunDBScan< TrackDistance< KTDBScan::Point > >(normPoints))
            {
                KTERROR(tclog, "An error occurred while clustering");
                return false;
            }
            KTDEBUG(tclog, "DBSCAN finished");

            // loop over the clusters found, and create data objects for them
            const vector< KTDBScan::Cluster >& clusters = fDBScan.GetClusters();
            KTDEBUG(tclog, "Found " << clusters.size() << " clusters; creating candidate events");
            for (vector< KTDBScan::Cluster >::const_iterator clustIt = clusters.begin(); clustIt != clusters.end(); ++clustIt)
            {
                if (clustIt->empty())
                {
                    KTWARN(tclog, "Empty cluster");
                    continue;
                }

                KTDEBUG(tclog, "Creating event " << fDataCount << "; includes " << clustIt->size() << " points");

                ++fDataCount;

                KTDataPtr data(new KTData());

                KTMultiTrackEventData& eventData = data->Of< KTMultiTrackEventData >();
                eventData.SetComponent(iComponent);
                eventData.SetEventID(fDataCount);

                for (KTDBScan::Cluster::const_iterator pointIdIt = clustIt->begin(); pointIdIt != clustIt->end(); ++pointIdIt)
                {
                    eventData.AddTrack(fCompTracks[iComponent][*pointIdIt]);
                }

                eventData.ProcessTracks();

                fCandidates.insert(data);
                fEventSignal(data);
            } // loop over clusters

            fCompTracks[iComponent].clear();

        } // loop over components

        KTDEBUG(tclog, "Clustering complete");
        fClusterDoneSignal();

        return true;
    }

    void KTDBScanEventClustering::SetNComponents(unsigned nComps)
    {
        fCompTracks.resize(nComps, vector< KTProcessedTrackData >());
        return;
    }

} /* namespace Katydid */
