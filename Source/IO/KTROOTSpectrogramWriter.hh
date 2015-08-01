/**
 @file KTROOTSpectrogramWriter.hh
 @brief Contains KTROOTSpectrogramWriter
 @details Writes 2-D histograms of spectrograms
 @author: N. S. Oblath
 @date: June 18, 2015
 */

#ifndef KTROOTSPECTROGRAMWRITER_HH_
#define KTROOTSPECTROGRAMWRITER_HH_

#include "KTWriter.hh"

#include "KTData.hh"
#include "KTFrequencySpectrum.hh"
#include "KTMemberVariable.hh"
#include "KTPowerSpectrum.hh"
#include "KTSliceHeader.hh"

#include "TFile.h"
#include "TH2.h"

#include <vector>

namespace Katydid
{
    using namespace Nymph;
    class KTFrequencyDomainArrayData;
    class KTROOTSpectrogramWriter;

    class KTROOTSpectrogramTypeWriter : public KTDerivedTypeWriter< KTROOTSpectrogramWriter >
    {
        public:
            KTROOTSpectrogramTypeWriter();
            virtual ~KTROOTSpectrogramTypeWriter();

        protected:
            struct SpectrogramData {
                TH2D* fSpectrogram;
                unsigned fFirstFreqBin; // frequency-axis bin 0 is this bin in the incoming data
                unsigned fLastFreqBin; // frequency-axis last-bin is this bin in the incoming data
                unsigned fNextTimeBinToFill; // keep track of the progress filling the spectrogram from slice to slice
            };

            /// Checks to see if new spectrograms are needed, and creates them if so
            void CreateNewSpectrograms(const KTFrequencyDomainArrayData& data, unsigned nComponents, double startTime, unsigned sliceLength, std::vector< SpectrogramData >& spectrograms, std::string histNameBase);

            template< typename XDataType >
            void AddFrequencySpectrumDataHelper(KTDataPtr data, std::vector< SpectrogramData >& spectrograms, std::string histNameBase);

            template< typename XDataType >
            void AddPowerSpectrumDataCoreHelper(KTDataPtr data, std::vector< SpectrogramData >& spectrograms, std::string histNameBase);
            template< typename XDataType >
            void AddPowerSpectralDensityDataCoreHelper(KTDataPtr data, std::vector< SpectrogramData >& spectrograms, std::string histNameBase);
    };

  /*!
     @class KTROOTSpectrogramWriter
     @author N. S. Oblath

     @brief Outputs a spectrogram in the form of a 2D histogram to a ROOT file

     @details 

     Configuration name: "root-spectrogram-writer"

     Available configuration values:
     - "output-file": string -- output filename
     - "file-flag": string -- TFile option: CREATE, RECREATE, or UPDATE
     - "min-time": double -- start time for the spectrograms
     - "max-time": double -- end time for the spectrograms
     - "min-freq": double -- start frequency for the spectrograms
     - "max-freq": double -- end frequency for the spectrograms

     Slots:
     - "fs-fftw": void (KTDataPtr) -- Contribute a spectrum to a FS-FFTW spectrogram.
     - "fs-polar": void (KTDataPtr) -- Contribute a spectrum to a FS-polar spectrogram.
     - "power": void (KTDataPtr) -- Contribute a spectrum to a power spectrogram.
     - "psd": void (KTDataPtr) -- Contribute a spectrum to a PSD spectrogram.

     
    */

    class KTROOTSpectrogramWriter : public KTWriterWithTypists< KTROOTSpectrogramWriter, KTROOTSpectrogramTypeWriter >//public KTWriter
    {
        public:
            KTROOTSpectrogramWriter(const std::string& name = "root-spectrogram-writer");
            virtual ~KTROOTSpectrogramWriter();

            bool Configure(const KTParamNode* node);

        public:
            TFile* OpenFile(const std::string& filename, const std::string& flag);
            void CloseFile();

            MEMBERVARIABLEREF(std::string, Filename);
            MEMBERVARIABLEREF(std::string, FileFlag);

            MEMBERVARIABLE(double, MinTime); // in sec
            MEMBERVARIABLE(double, MaxTime); // in sec

            MEMBERVARIABLE(double, MinFreq); // in Hz
            MEMBERVARIABLE(double, MaxFreq); // in Hz

            MEMBERVARIABLE_NOSET(TFile*, File);

            bool OpenAndVerifyFile();

    };

    inline TFile* KTROOTSpectrogramWriter::OpenFile(const std::string& filename, const std::string& flag)
    {
        CloseFile();
        fFile = new TFile(filename.c_str(), flag.c_str());
        return fFile;
    }

    inline void KTROOTSpectrogramWriter::CloseFile()
    {
        if (fFile != NULL)
        {
            fFile->Close();
            delete fFile;
            fFile = NULL;
        }
        return;
    }



    //****************************
    // KTROOTSpectrogramTypeWriter
    //****************************

     template< class XDataType >
     void KTROOTSpectrogramTypeWriter::AddFrequencySpectrumDataHelper(KTDataPtr data, std::vector< SpectrogramData >& spectrograms, std::string histNameBase)
     {
         KTSliceHeader& sliceHeader = data->Of< KTSliceHeader >();
         double timeInRun = sliceHeader.GetTimeInRun();
         double sliceLength = sliceHeader.GetSliceLength();
         // Check if this is a slice we should care about.
         // The first slice of interest will contain the writer's min time;
         // The last slice of interest will contain the writer's max time.
         if (timeInRun + sliceLength >= fWriter->GetMinTime() && timeInRun <= fWriter->GetMaxTime())
         {
             // Ok, this is a slice we should pay attention to.
             XDataType& fsData = data->Of< XDataType >();
             unsigned nComponents = fsData.GetNComponents();

             KTROOTSpectrogramTypeWriter::CreateNewSpectrograms(fsData, nComponents, timeInRun, sliceLength, spectrograms, histNameBase);

             // add this slice's data to the spectrogram
             for (unsigned iComponent = 0; iComponent < nComponents; ++iComponent)
             {
                 const KTFrequencySpectrum* spectrum = fsData.GetSpectrum(iComponent);
                 unsigned iSpectFreqBin = 0;
                 for (unsigned iFreqBin = spectrograms[iComponent].fFirstFreqBin; iFreqBin <= spectrograms[iComponent].fLastFreqBin; ++iFreqBin)
                 {
                     //std::cout << "spectrum bin: " << iFreqBin << "   spectrogram bins (" << fFSFFTWSpectrograms[iComponent].fNextTimeBinToFill << ", " << iSpectFreqBin << "    value: " << spectrum->GetAbs(iFreqBin) << std::endl;
                     spectrograms[iComponent].fSpectrogram->SetBinContent(spectrograms[iComponent].fNextTimeBinToFill, iSpectFreqBin, spectrum->GetAbs(iFreqBin));
                     ++iSpectFreqBin;
                 }
                 spectrograms[iComponent].fNextTimeBinToFill += 1;
             }
         }

         return;
     }

     template< class XDataType >
     void KTROOTSpectrogramTypeWriter::AddPowerSpectrumDataCoreHelper(KTDataPtr data, std::vector< SpectrogramData >& spectrograms, std::string histNameBase)
     {
         KTSliceHeader& sliceHeader = data->Of< KTSliceHeader >();
         double timeInRun = sliceHeader.GetTimeInRun();
         double sliceLength = sliceHeader.GetSliceLength();
         // Check if this is a slice we should care about.
         // The first slice of interest will contain the writer's min time;
         // The last slice of interest will contain the writer's max time.
         if (timeInRun + sliceLength >= fWriter->GetMinTime() && timeInRun <= fWriter->GetMaxTime())
         {
             // Ok, this is a slice we should pay attention to.
             XDataType& fsData = data->Of< XDataType >();
             unsigned nComponents = fsData.GetNComponents();

             KTROOTSpectrogramTypeWriter::CreateNewSpectrograms(fsData, nComponents, timeInRun, sliceLength, spectrograms, histNameBase);

             // add this slice's data to the spectrogram
             for (unsigned iComponent = 0; iComponent < nComponents; ++iComponent)
             {
                 KTPowerSpectrum* spectrum = fsData.GetSpectrum(iComponent);
                 spectrum->ConvertToPowerSpectrum();
                 unsigned iSpectFreqBin = 0;
                 for (unsigned iFreqBin = spectrograms[iComponent].fFirstFreqBin; iFreqBin <= spectrograms[iComponent].fLastFreqBin; ++iFreqBin)
                 {
                     //std::cout << "spectrum bin: " << iFreqBin << "   spectrogram bins (" << fFSFFTWSpectrograms[iComponent].fNextTimeBinToFill << ", " << iSpectFreqBin << "    value: " << spectrum->GetAbs(iFreqBin) << std::endl;
                     spectrograms[iComponent].fSpectrogram->SetBinContent(spectrograms[iComponent].fNextTimeBinToFill, iSpectFreqBin, (*spectrum)(iFreqBin));
                     ++iSpectFreqBin;
                 }
                 spectrograms[iComponent].fNextTimeBinToFill += 1;
             }
         }

         return;
     }

     template< class XDataType >
     void KTROOTSpectrogramTypeWriter::AddPowerSpectralDensityDataCoreHelper(KTDataPtr data, std::vector< SpectrogramData >& spectrograms, std::string histNameBase)
     {
         KTSliceHeader& sliceHeader = data->Of< KTSliceHeader >();
         double timeInRun = sliceHeader.GetTimeInRun();
         double sliceLength = sliceHeader.GetSliceLength();
         // Check if this is a slice we should care about.
         // The first slice of interest will contain the writer's min time;
         // The last slice of interest will contain the writer's max time.
         if (timeInRun + sliceLength >= fWriter->GetMinTime() && timeInRun <= fWriter->GetMaxTime())
         {
             // Ok, this is a slice we should pay attention to.
             XDataType& fsData = data->Of< XDataType >();
             unsigned nComponents = fsData.GetNComponents();

             KTROOTSpectrogramTypeWriter::CreateNewSpectrograms(fsData, nComponents, timeInRun, sliceLength, spectrograms, histNameBase);

             // add this slice's data to the spectrogram
             for (unsigned iComponent = 0; iComponent < nComponents; ++iComponent)
             {
                 KTPowerSpectrum* spectrum = fsData.GetSpectrum(iComponent);
                 spectrum->ConvertToPowerSpectralDensity();
                 unsigned iSpectFreqBin = 0;
                 for (unsigned iFreqBin = spectrograms[iComponent].fFirstFreqBin; iFreqBin <= spectrograms[iComponent].fLastFreqBin; ++iFreqBin)
                 {
                     //std::cout << "spectrum bin: " << iFreqBin << "   spectrogram bins (" << fFSFFTWSpectrograms[iComponent].fNextTimeBinToFill << ", " << iSpectFreqBin << "    value: " << spectrum->GetAbs(iFreqBin) << std::endl;
                     spectrograms[iComponent].fSpectrogram->SetBinContent(spectrograms[iComponent].fNextTimeBinToFill, iSpectFreqBin, (*spectrum)(iFreqBin));
                     ++iSpectFreqBin;
                 }
                 spectrograms[iComponent].fNextTimeBinToFill += 1;
             }
         }

         return;
     }

} /* namespace Katydid */
#endif /* KTROOTSPECTROGRAMWRITER_HH_ */