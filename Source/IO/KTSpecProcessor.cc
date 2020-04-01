#include<iostream>
#include <fstream>
#include "KTSpecProcessor.hh"
#include "KTCommandLineOption.hh"
#include "KTData.hh"
#include "KTProcSummary.hh"
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
            //fFreqChannels(8192),
            //fNSpectra(0),
            //fMinFreq(0.),
            //fMaxFreq(1600.0),
            //fBinWidth ((fMaxFreq - fMinFreq)/fFreqChannels),
            //fHeaderSignal("header", this),
            fDataSignal("psd", this)
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
            size = file.tellg();
            memblock = new char [size];
            file.seekg (0, ios::beg);
            file.read (memblock, size);
            file.close();
        }

        int a [8192];
        for (int x = 0; x < 8192; x++)
            {
            a[x] =  memblock[x+32];
            }

        //KTINFO(speclog, "Spec file has been opened and read");

        //KTDEBUG(speclog, "Parsing Spec file header");
        //read header
        //KTDEBUG(speclog, "Parsed header:\n" << fHeader);

        //KTSpecHeader& header = headerPtr->Of< KTEggHeader >();

        //fHeaderSignal(headerPtr);

        //initialize an object of type KTPowerSpectrum with all 0
        //values and 8192 channels from 0 to 1600 MHz
        size_t bins = 8192;
        double min = 0.0;
        double max = 1600.00;
        KTPowerSpectrum newSlice(a, bins, min, max);

        Nymph::KTDataPtr data(new Nymph::KTData());
        unsigned comp = 1;
        KTPowerSpectrumData& psData = data->Of< KTPowerSpectrumData >().SetNComponents(comp);

        psData.SetSpectrum(&newSlice, comp);

        fDataSignal(data);

        delete[] memblock;

        return true;
    }

} /* namespace Katydid */
