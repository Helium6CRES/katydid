/**
 @file ChannelCorrelator.cc
 @brief Executable to produce correlations between channels
 @details Produces correlations between channels from Egg events
 @author: N. S. Oblath
 @date: Sept 5, 2012
 */

#include "KTApplication.hh"
#include "KTBasicROOTFileWriter.hh"
#include "KTEggProcessor.hh"
#include "KTLogger.hh"
#include "KTSimpleFFT.hh"
#include "KTCorrelator.hh"

#include <string>

using namespace std;
using namespace Katydid;

KTLOGGER(corrlog, "katydid.applications.main");


int main(int argc, char** argv)
{
    KTApplication* app = new KTApplication(argc, argv);
    app->ReadConfigFile();

    string appConfigName("channel-correlation");

    // Setup the processors and their signal/slot connections
    KTEggProcessor procEgg;
    KTSimpleFFT procFFT;
    KTCorrelator procCorr;
    KTBasicROOTFileWriter procPub;


    // Configure the processors
    app->Configure(&procEgg, appConfigName);
    app->Configure(&procFFT, appConfigName);
    app->Configure(&procCorr, appConfigName);
    app->Configure(&procPub, appConfigName);

    try
    {
        // When procEgg parses the header, the info is passed to PrepareFFT
        procEgg.ConnectASlot("header", &procFFT, "header");

        // When procEgg hatches an event, procFFT.ProcessEvent and procCorr.ProcessEvent will be called
        procEgg.ConnectASlot("event", &procFFT, "event", 0);
        procEgg.ConnectASlot("event", &procCorr, "event", 1);

        // This will get the output histogram when an FFT and correlation are complete
        procCorr.ConnectASlot("correlation", &procPub, "write-data");
    }
    catch (std::exception& e)
    {
        KTERROR(corrlog, "An error occured while connecting signals and slots:\n"
                << '\t' << e.what());
        return -1;
    }



    // Process the egg file
    Bool_t success = procEgg.ProcessEgg();


    if (! success) return -1;
    return 0;
}
