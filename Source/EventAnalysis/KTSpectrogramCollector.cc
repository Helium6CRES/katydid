/*
 * KTSpectrogramCollector.cc
 *
 *  Created on: Oct 9, 2015
 *      Author: ezayas
 */

#include "KTSpectrogramCollector.hh"

#include "KTLogger.hh"
#include "KTNOFactory.hh"
#include "KTSliceHeader.hh"
#include "KTSpectrumCollectionData.hh"
#include "KTProcessedTrackData.hh"
#include "KTPowerSpectrum.hh"
#include "KTPowerSpectrumData.hh"
#include "KTData.hh"

#include <set>

namespace Katydid
{
    KTLOGGER(evlog, "KTSpectrogramCollector");

    // Register the processor
    KT_REGISTER_PROCESSOR(KTSpectrogramCollector, "spectrogram-collector");

    KTSpectrogramCollector::KTSpectrogramCollector(const std::string& name) :
            KTProcessor(name),
            fMinFrequency(0.),
            fMaxFrequency(1.),
            fMinBin(0),
            fMaxBin(1),
            fCalculateMinBin(true),
            fCalculateMaxBin(true),
            fLeadTime(0.002),
            fTrailTime(0.002),
            fWaterfallSignal("waterfall", this),
            fTrackSlot("track", this, &KTSpectrogramCollector::ReceiveTrack)
    {
        RegisterSlot( "ps", this, &KTSpectrogramCollector::SlotFunctionPSData );
    }

    KTSpectrogramCollector::~KTSpectrogramCollector()
    {
    }

    // Emit Signal
    void KTSpectrogramCollector::FinishSC( KTDataPtr data )
    {
        fWaterfallSignal( data );
    }

    bool KTSpectrogramCollector::Configure(const scarab::param_node* node)
    {
        if (node == NULL) return false;

        if (node->has("min-frequency"))
        {
            SetMinFrequency(node->get_value< double >("min-frequency"));
        }
        if (node->has("max-frequency"))
        {
            SetMaxFrequency(node->get_value< double >("max-frequency"));
        }

        SetMinBin(node->get_value< unsigned >("min-bin", fMinBin));
        SetMaxBin(node->get_value< unsigned >("max-bin", fMaxBin));
        SetLeadTime(node->get_value< double >("lead-time", fLeadTime));
        SetTrailTime(node->get_value< double >("trail-time", fTrailTime));

        return true;
    }

    bool KTSpectrogramCollector::AddTrack( KTProcessedTrackData& trackData, unsigned component )
    {
        // Create new KTDataPtr and PSCollectionData
        KTDataPtr ptr( new KTData() );
        KTProcessedTrackData* newTrack = &ptr->Of< KTProcessedTrackData >();
        KTPSCollectionData* newWaterfall = &ptr->Of< KTPSCollectionData >();

        // Configure the track to retain only the slope information
        newTrack->SetSlope( trackData.GetSlope() );
        newTrack->SetStartTimeInRunC( trackData.GetStartTimeInRunC() );
        newTrack->SetEndTimeInRunC( trackData.GetEndTimeInRunC() );
        newTrack->SetSlopeSigma( trackData.GetSlopeSigma() );

        // Configure PSCollectionData timestamps
        newWaterfall->SetStartTime( trackData.GetStartTimeInRunC() - fLeadTime );
        newWaterfall->SetEndTime( trackData.GetEndTimeInRunC() + fTrailTime );
        newWaterfall->SetFilling( false );

        // Add to fWaterfallSets
        fWaterfallSets[component].insert( std::make_pair( ptr, newWaterfall ) );

        KTINFO(evlog, "Added track to component " << component << ". Now listening to a total of " << fWaterfallSets[component].size() << " tracks");
        KTINFO(evlog, "Track length: " << trackData.GetEndTimeInRunC() - trackData.GetStartTimeInRunC());
        KTINFO(evlog, "Track slope: " << trackData.GetSlope());

        return true;
    }

    bool KTSpectrogramCollector::ConsiderSpectrum( KTPowerSpectrum& ps, KTSliceHeader& slice, unsigned component, bool forceEmit )
    {
        // Iterate through each track which has been added
        for( std::set< std::pair< KTDataPtr, KTPSCollectionData* >, KTTrackCompare >::const_iterator it = fWaterfallSets[component].begin(); it != fWaterfallSets[component].end(); ++it )
        {
            // If the slice time coincides with the track time window, add the spectrum
            // The forceEmit flag overrides this; essentially guarantees the spectrum will be interpreted as outside the track window
            if( !forceEmit && slice.GetTimeInRun() >= it->second->GetStartTime() && slice.GetTimeInRun() <= it->second->GetEndTime() )
            {
                it->second->AddSpectrum( slice.GetTimeInRun(), &ps );
                it->second->SetDeltaT( slice.GetSliceLength() );
                it->second->SetFilling( true );
            }
            else
            {
                // If GetFilling() is true, we've reached the end of the track time window
                // forceEmit=true sends all tracks to this clause, and those still filling will be closed & signals emitted
                if( it->second->GetFilling() )
                {
                    it->second->SetFilling( false );

                    // Emit signal
                    KTINFO(evlog, "Finished a track; emitting signal");
                    FinishSC( it->first );
                }
                else
                    it->second->SetFilling( false );
            }
        }

        return true;
    }

    bool KTSpectrogramCollector::ReceiveTrack( KTProcessedTrackData& data )
    {
        if( data.GetIsCut() )
        {
            KTINFO(evlog, "Processed track failed cuts, skipping it");
            return true;
        }

        unsigned iComponent = data.GetComponent();

        // Increase size of fWaterfallSets if necessary
        int fWSsize = fWaterfallSets.size();
        std::set< std::pair< KTDataPtr, KTPSCollectionData* >, KTTrackCompare > blankSet;
        for( int i = fWSsize; i <= iComponent; i++ )
            fWaterfallSets.push_back( blankSet );

        // Add track
        if( !AddTrack( data, iComponent ) )
        {
            KTERROR(evlog, "Spectrogram collector could not add track! (component " << iComponent << ")" );
        }

        return true;
    }
    
    bool KTSpectrogramCollector::ReceiveSpectrum( KTPowerSpectrumData& data, KTSliceHeader& sliceData, bool forceEmit )
    {
        KTDEBUG(evlog, "Receiving Spectrum");
        if (fCalculateMinBin)
        {
            SetMinBin(data.GetSpectrum(0)->FindBin(fMinFrequency));
            KTDEBUG(evlog, "Minimum bin set to " << fMinBin);
        }
        if (fCalculateMaxBin)
        {
            SetMaxBin(data.GetSpectrum(0)->FindBin(fMaxFrequency));
            KTDEBUG(evlog, "Maximum bin set to " << fMaxBin);
        }

        unsigned nComponents = data.GetNComponents();
        
        for (unsigned iComponent=0; iComponent<nComponents; ++iComponent)
        {
            if (! ConsiderSpectrum(*data.GetSpectrum(iComponent), sliceData, iComponent, forceEmit))
            {
                KTERROR(evlog, "Spectrogram collector could not receive spectrum! (component " << iComponent << ")");
                return false;
            }
        }
        KTINFO(evlog, "Spectrum finished processing. Awaiting next spectrum");

        return true;
    }
} // namespace Katydid
