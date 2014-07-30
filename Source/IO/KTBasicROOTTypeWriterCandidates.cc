/*
 * KTBasicROOTTypeWriterCandidates.cc
 *
 *  Created on: Jul 29, 2014
 *      Author: nsoblath
 */

#include "KTBasicROOTTypeWriterCandidates.hh"

//#include "KT2ROOT.hh"
#include "KTLogger.hh"
#include "KTProcessedTrackData.hh"
#include "KTSliceHeader.hh"
#include "KTSparseWaterfallCandidateData.hh"
#include "KTTIFactory.hh"

#include "TCanvas.h"
#include "TGraph.h"
#include "TLine.h"

#include <sstream>



using std::stringstream;
using std::string;

namespace Katydid
{
    KTLOGGER(publog, "KTBasicROOTTypeWriterCandidates");


    static KTTIRegistrar< KTBasicROOTTypeWriter, KTBasicROOTTypeWriterCandidates > sBRTWCandidatesRegistrar;

    KTBasicROOTTypeWriterCandidates::KTBasicROOTTypeWriterCandidates() :
            KTBasicROOTTypeWriter()
    {
    }

    KTBasicROOTTypeWriterCandidates::~KTBasicROOTTypeWriterCandidates()
    {
    }


    void KTBasicROOTTypeWriterCandidates::RegisterSlots()
    {
        fWriter->RegisterSlot("track-and-swfc", this, &KTBasicROOTTypeWriterCandidates::WriteProcTrackAndSWFC);

        return;
    }


    //************************
    // Processed Track & Sparse Waterfall Candidate
    //************************

    void KTBasicROOTTypeWriterCandidates::WriteProcTrackAndSWFC(KTDataPtr data)
    {
        if (! data) return;

        static unsigned iCandidate = 0;

        KTINFO(publog, "Drawing track for candidate " << iCandidate);

        KTSparseWaterfallCandidateData& swfcData = data->Of< KTSparseWaterfallCandidateData >();
        KTProcessedTrackData& ptData = data->Of< KTProcessedTrackData >();

        if (ptData.GetIsCut())
        {
            KTINFO(publog, "Track was cut");
            ++iCandidate;
            return;
        }

        const KTSparseWaterfallCandidateData::Points& points = swfcData.GetPoints();

        if (! fWriter->OpenAndVerifyFile()) return;

        stringstream conv;
        conv << "track_" << iCandidate;

        TCanvas* trackCanv = new TCanvas(conv.str().c_str(), conv.str().c_str());

        // graph of points
        TGraph* grPoints = new TGraph(points.size());
        grPoints->SetName(conv.str().c_str());
        grPoints->SetMarkerStyle(20);
        grPoints->SetMarkerColor(4);

        unsigned iPoint = 0;
        for (KTSparseWaterfallCandidateData::Points::const_iterator pIt = points.begin(); pIt != points.end(); ++pIt)
        {
            grPoints->SetPoint(iPoint, pIt->fTimeInRun, pIt->fFrequency);
            KTDEBUG(publog, "Point " << iPoint << ": (" << pIt->fTimeInRun << ", " << pIt->fFrequency << ")");
            ++iPoint;
        }
        grPoints->Draw("ap");

        // line for track
        TLine* trackLine = new TLine(ptData.GetStartTimeInRun(), ptData.GetStartFrequency(), ptData.GetEndTimeInRun(), ptData.GetEndFrequency());
        trackLine->SetLineColor(1);
        trackLine->SetLineWidth(1);
        trackLine->Draw();
        KTDEBUG(publog, "Line drawn: (" << trackLine->GetX1() << ", " << trackLine->GetY1() << "), --> (" << trackLine->GetX2() << ", " << trackLine->GetY2() << ")");

        trackCanv->Write();

        KTDEBUG(publog, "Track drawing complete");

        ++iCandidate;
        return;
    }




} /* namespace Katydid */
