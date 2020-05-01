#include<iostream>
#include <fstream>
#include "KTSpecProcessor.hh"
#include "KTCommandLineOption.hh"
#include "KTData.hh"
#include "KTSliceHeader.hh"
#include "KTPowerSpectrum.hh"
#include "KTPowerSpectrumData.hh"

using std::string;
using namespace std;

namespace Katydid
{
    static Nymph::KTCommandLineOption< string > sFilenameCLO("Spec Processor",
    "Spec filename to open", "spec-file", 's');

    KTLOGGER(speclog, "KTSpecProcessor");

    KT_REGISTER_PROCESSOR(KTSpecProcessor, "spec-processor");

    KTSpecProcessor::KTSpecProcessor(const std::string& name) :
            KTPrimaryProcessor(name),
            fProgressReportInterval(1),
            fFilenames(),
            fFreqBins(8192),
            fNSpectra(0),
            fFreqMin(0.),
            fFreqMax(1.6E09),
            //fHeaderPtr(new Nymph::KTData()),
            //fHeader(fHeaderPtr->Of< KTEggHeader >()),
            //fMasterSliceHeader(),
            fDataSignal("ps", this),
            fSpecDoneSignal("spec-done", this)
    {
    }

    KTSpecProcessor::~KTSpecProcessor()
    {
    }

    bool KTSpecProcessor::Configure(const scarab::param_node* node)
    {
        // Config-file settings
        if (node != NULL)
        {
            if (node->has("filename"))
            {
                KTDEBUG(speclog, "Adding single file to spec processor");
                fFilenames.clear();
                fFilenames.push_back( std::move(scarab::expand_path(node->get_value( "filename" ))) );
                KTINFO(speclog, "Added file to spec processor: <" << fFilenames.back() << ">");
            }
            else if (node->has("filenames"))
            {
                KTDEBUG(speclog, "Adding multiple files to spec processor");
                fFilenames.clear();
                const scarab::param_array* t_filenames = node->array_at("filenames");
                for(scarab::param_array::const_iterator t_file_it = t_filenames->begin(); t_file_it != t_filenames->end(); ++t_file_it)
                {
                    fFilenames.push_back( std::move(scarab::expand_path((*t_file_it)->as_value().as_string())) );
                    KTINFO(speclog, "Added file to spec processor: <" << fFilenames.back() << ">");
                }
            }
            fNSpectra = node->get_value< unsigned >("spectra", fNSpectra);
            fFreqBins = node->get_value< double >("freq-bins", fFreqBins);
            fFreqMin = node->get_value< unsigned >("min-freq", fFreqMin);
            fFreqMax = node->get_value< unsigned >("max-freq", fFreqMax);
            fPacketHeaderSize = node->get_value< unsigned >("header-bytes", fPacketHeaderSize);

        }

        // Command-line settings
        if (fCLHandler->IsCommandLineOptSet("spec-file"))
        {
            KTDEBUG(speclog, "Adding single file to spec processor from the CL");
            fFilenames.clear();
            fFilenames.push_back( std::move(scarab::expand_path(fCLHandler->GetCommandLineValue< string >("spec-file"))));
            KTINFO(speclog, "Added file to spec processor: <" << fFilenames.back() << ">");
        }

        return true;
    }

    bool KTSpecProcessor::ProcessSpec()
    {
/*
    fMasterSliceHeader.SetSampleRate(fHeader.GetAcquisitionRate());
    fMasterSliceHeader.SetRawSliceSize(fSliceSize);
    fMasterSliceHeader.SetSliceSize(fSliceSize);
    fMasterSliceHeader.CalculateBinWidthAndSliceLength();
    fMasterSliceHeader.SetNonOverlapFrac((double)fStride / (double)fSliceSize);
    fMasterSliceHeader.SetRecordSize(fHeader.GetChannelHeader(0)->GetRecordSize());

      return fHeaderPtr; */
        if (fFilenames.size() == 0)
        {
            KTERROR(speclog, "No files have been specified");
            return false;
        }

        // open the file and load its contents into memblock
        KTINFO(speclog, "Opening spec file <" << fFilenames[0] << ">");
        streampos size;
        char * memblock;
        //ifstream file ("/home/brent/Desktop/SpecFiles/Freq_data_2020-03-25-17-36-18_000000.spec", ios::in|ios::binary|ios::ate);

        std::ifstream file(fFilenames[0].c_str(), ios::in|ios::binary|ios::ate);

        if (file.is_open())
        {
            KTINFO(speclog, "Spec file <" << fFilenames[0] << "> opened");
            uint a; //spectra must be treated as unsigned 8-bit values (0-255)
            int slice[fFreqBins]; //holder array for spectrum data
            int position = 0; //variable for read position start
            int blockSize = fPacketHeaderSize+fFreqBins; //add header length

            //declare array of 1-byte chars to read from binary file
            char memblock [blockSize];

            vector <KTPowerSpectrum*> newSpec(1);

            //loop over # of spectra to be read
            for(int i = 0; i < fNSpectra; i++)
            {
                if (i == 0) KTINFO(speclog, "Preparing to read first spectrum");
                position = i*(blockSize);
                file.seekg (position, ios::beg); //set read pointer location
                file.read (memblock, blockSize); //read 1 spectrum of data


                for (int x = 0; x < fFreqBins; x++)
                {
                    a =  memblock[x+fPacketHeaderSize];
                    slice[x] = a;
                }

                unsigned comp = 0;
                //initialize an object of type KTPowerSpectrum with all 0
                //values and 8192 Bins from 0 to 1600 MHz
                Nymph::KTDataPtr data(new Nymph::KTData());

                KTSliceHeader& sliceHeader = data->Of< KTSliceHeader >().SetNComponents(1);
                sliceHeader.SetTimeInRun(1.0);
                sliceHeader.SetSliceNumber(i);
                sliceHeader.SetSliceLength(4.09e-05);
                KTINFO(speclog, "Slice length = " << sliceHeader.GetSliceLength());

                newSpec[0] = new KTPowerSpectrum(slice, fFreqBins, fFreqMin, fFreqMax);
                KTPowerSpectrumData& psData = data->Of< KTPowerSpectrumData >().SetNComponents(1);
                psData.SetSpectrum(newSpec[0], comp);
                psData.GetArray(comp)->GetAxis().SetBinsRange(0.0, 1.6e9, 8192);

                if (i == 0)
                {
                    double min = psData.GetArray(comp)->GetAxis().GetRangeMin();
                    KTINFO(speclog, "First spectrum min freq = " << min);

                    double max = psData.GetArray(comp)->GetAxis().GetRangeMax();
                    KTINFO(speclog, "First spectrum max freq = " << max);

                    double width = psData.GetArray(comp)->GetAxis().GetBinWidth();
                    KTINFO(speclog, "First spectrum bin width = " << width);
                }


                if (i == 0) KTINFO(speclog, "Set first spectrum object");

                if(i == fNSpectra -1)
                {
                    data->Of< Nymph::KTData >().SetLastData(true);
                    KTINFO(speclog, "fLastData set to TRUE");
                }

                fDataSignal(data);
                if (i == 0) KTINFO(speclog, "First spectrum data signal output");

            }
            fSpecDoneSignal();
            KTINFO(speclog, "Spec-done signal output");
        }
        file.close();
        return true;
    }
} /* namespace Katydid */
