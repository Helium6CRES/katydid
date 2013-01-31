/*
 * KTMultiEventROOTTypeWriterFFT.cc
 *
 *  Created on: Aug 24, 2012
 *      Author: nsoblath
 */

#include "KTMultiEventROOTTypeWriterFFT.hh"

#include "KTEggHeader.hh"
#include "KTEvent.hh"
#include "KTTIFactory.hh"
#include "KTLogger.hh"
#include "KTFrequencySpectrum.hh"
#include "KTFrequencySpectrumFFTW.hh"
#include "KTFrequencySpectrumData.hh"
#include "KTFrequencySpectrumDataFFTW.hh"

#include "TH1.h"
#include "TStyle.h"

#include <sstream>

using std::stringstream;
using std::string;
using std::vector;

namespace Katydid
{
    KTLOGGER(publog, "katydid.output");

    static KTDerivedTIRegistrar< KTMultiEventROOTTypeWriter, KTMultiEventROOTTypeWriterFFT > sMERTWFRegistrar;

    KTMultiEventROOTTypeWriterFFT::KTMultiEventROOTTypeWriterFFT() :
            KTMEROOTTypeWriterBase(),
            //KTTypeWriterFFT()
            fFSHists(),
            fFSFFTWHists()
    {
    }

    KTMultiEventROOTTypeWriterFFT::~KTMultiEventROOTTypeWriterFFT()
    {
        ClearHistograms();
    }

    void KTMultiEventROOTTypeWriterFFT::StartNewHistograms()
    {
        ClearHistograms();
        // At this point the vector is size 0
        return;
    }

    void KTMultiEventROOTTypeWriterFFT::FinishHistograms()
    {
        OutputHistograms();
        ClearHistograms();
        return;
    }

    void KTMultiEventROOTTypeWriterFFT::OutputHistograms()
    {
        if (! fWriter->OpenAndVerifyFile()) return;

        gStyle->SetOptStat(0);
        for (UInt_t iChannel=0; iChannel < fFSHists.size(); iChannel++)
        {
            /*// code for priting to image files
            stringstream conv;
            conv << "_" << iChannel << "." << fOutputFileType;
            string fileName = fOutputFilenameBase + conv.str();
            if (! fOutputFilePath.empty()) fileName = fOutputFilePath + '/' + fileName;

            TCanvas* cPrint = new TCanvas("cPrint", "cPrint");
            cPrint->SetLogy(1);
            fAveragePSHists[iChannel]->Draw();

            cPrint->Print(fileName.c_str(), fOutputFileType.c_str());
            KTINFO(psavglog, "Printed file " << fileName);
            delete cPrint;
            */

            // Writing to ROOT file
            fFSHists[iChannel]->SetDirectory(fWriter->GetFile());
            fFSHists[iChannel]->Write();
        }
        for (UInt_t iChannel=0; iChannel < fFSFFTWHists.size(); iChannel++)
        {
            fFSFFTWHists[iChannel]->SetDirectory(fWriter->GetFile());
            fFSFFTWHists[iChannel]->Write();
        }

        return;
    }

    void KTMultiEventROOTTypeWriterFFT::ClearHistograms()
    {
        for (vector<TH1D*>::iterator it=fFSHists.begin(); it != fFSHists.end(); it++)
        {
            delete *it;
        }
        for (vector<TH1D*>::iterator it=fFSFFTWHists.begin(); it != fFSFFTWHists.end(); it++)
        {
            delete *it;
        }
        fFSHists.clear();
        fFSFFTWHists.clear();
        return;
    }

    void KTMultiEventROOTTypeWriterFFT::RegisterSlots()
    {
        fWriter->RegisterSlot("fs-data", this, &KTMultiEventROOTTypeWriterFFT::AddFrequencySpectrumData, "void (const KTFrequencySpectrumData*)");
        fWriter->RegisterSlot("fs-fftw-data", this, &KTMultiEventROOTTypeWriterFFT::AddFrequencySpectrumDataFFTW, "void (const KTFrequencySpectrumDataFFTW*)");
        return;
    }


    //*****************
    // Time Series Data
    //*****************

    void KTMultiEventROOTTypeWriterFFT::AddFrequencySpectrumData(const KTFrequencySpectrumData* data)
    {
        if (fFSHists.size() == 0)
        {
            fFSHists.resize(data->GetNChannels());

            std::string histNameBase("PowerSpectrum");
            for (UInt_t iChannel=0; iChannel < data->GetNChannels(); iChannel++)
            {
                std::stringstream conv;
                conv << iChannel;
                std::string histName = histNameBase + conv.str();
                TH1D* newPS = data->GetSpectrum(iChannel)->CreatePowerHistogram(histName);
                fFSHists[iChannel] = newPS;
            }
        }
        else
        {
            for (UInt_t iChannel=0; iChannel < data->GetNChannels(); iChannel++)
            {
                TH1D* newPS = data->GetSpectrum(iChannel)->CreatePowerHistogram();
                fFSHists[iChannel]->Add(newPS);
                delete newPS;
            }
        }
        return;
    }

    void KTMultiEventROOTTypeWriterFFT::AddFrequencySpectrumDataFFTW(const KTFrequencySpectrumDataFFTW* data)
    {
        if (fFSFFTWHists.size() == 0)
        {
            fFSFFTWHists.resize(data->GetNChannels());

            std::string histNameBase("PowerSpectrum");
            for (UInt_t iChannel=0; iChannel < data->GetNChannels(); iChannel++)
            {
                std::stringstream conv;
                conv << iChannel;
                std::string histName = histNameBase + conv.str();
                TH1D* newPS = data->GetSpectrum(iChannel)->CreatePowerHistogram(histName);
                fFSFFTWHists[iChannel] = newPS;
            }
        }
        else
        {
            for (UInt_t iChannel=0; iChannel <data->GetNChannels(); iChannel++)
            {
                TH1D* newPS = data->GetSpectrum(iChannel)->CreatePowerHistogram();
                fFSFFTWHists[iChannel]->Add(newPS);
                delete newPS;
            }
        }
        return;
    }
} /* namespace Katydid */
