#include <fstream>
#include "KTSpecProcessor.hh"
#include "KTCommandLineOption.hh"
#include "KTData.hh"
#include "KTProcSummary.hh"
#include "KTPowerSpectrumData.hh"

using std::string;

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
            fFreqChannels(8192),
            fMinFreq(0.),
            fMaxFreq(2000.0),
            fBinWidth = (fMaxFreq - fMinFreq) / fFreqChannels,
            fHeaderSignal("header", this),
            fDataSignal("psd", this),
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

        //fstream::open(fFilenames[0].native());
        //fCurrentFileIt = fFilenames.begin();

        // open the file and load its contents into memblock
        KTINFO(speclog, "Opening spec file <" << fFilenames[0] << ">");
        streampos size;
        char * memblock;
        fstream file (fFilenames[0], ios::in|ios::binary|ios::ate);
        if (file.is_open())
        {
            size = file.tellg();
            memblock = new char [size];
            file.seekg (0, ios::beg);
            file.read (memblock, size);
            file.close();
        }
        KTINFO(speclog, "Spec file has been opened and read");

        KTDEBUG(speclog, "Parsing Spec file header");
        //read header
        KTDEBUG(speclog, "Parsed header:\n" << fHeader);

        KTSpecHeader& header = headerPtr->Of< KTEggHeader >();

        fHeaderSignal(headerPtr);

        Nymph::KTDataPtr newData(new Nymph::KTData());

        delete[] memblock;

        // finally, set the records in the new data object
        KTPowerSpectrumData& psData = newData->Of< KTRawPowerSpectrumData >().SetNComponents(nChannels);
        for (unsigned iChannel = 0; iChannel < nChannels; ++iChannel)
        {
            psData.SetTimeSeries(newSlices[iChannel], iChannel);
        }

        fDataSignal(psData);
        return true;
    }

} /* namespace Katydid */
