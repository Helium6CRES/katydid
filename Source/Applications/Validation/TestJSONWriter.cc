/*
 * TestBasicROOTFileWriter.cc
 *
 *  Created on: Aug 24, 2012
 *      Author: nsoblath
 */

#include "KTEggHeader.hh"
#include "KTJSONWriter.hh"
#include "KTJSONTypeWriterEgg.hh"
#include "KTLogger.hh"

using namespace Katydid;
using namespace std;

KTLOGGER(testlog, "katydid.applications.validation");

int main()
{
    KTINFO(testlog, "Preparing for test");

    // Setup a dummy header to print
    KTEggHeader header;
    header.SetFilename("awesome_data.egg");
    header.SetAcquisitionMode(1);
    header.SetSliceSize(512);
    header.SetRecordSize(4194304);
    header.SetNChannels(2);
    header.SetRunDuration(203985);
    header.SetAcquisitionRate(500.);

    // Set up the writer
    KTJSONWriter writer;
    writer.SetFilename("test_writer.json");
    writer.SetFileMode("w+");
    writer.SetPrettyJSONFlag(false);

    KTINFO(testlog, "Writing to file");

    // Writer the data
    writer.GetTypeWriter< KTJSONTypeWriterEgg >()->WriteEggHeader(&header);

    // Close the file
    writer.CloseFile();

    KTINFO(testlog, "Writing complete");

    // Switch setup to print to the terminal
    writer.SetFilename("stdout");
    writer.SetPrettyJSONFlag(true);

    KTINFO(testlog, "Writing to terminal");

    writer.GetTypeWriter< KTJSONTypeWriterEgg >()->WriteEggHeader(&header);

    // Close the file
    writer.CloseFile();

    KTINFO(testlog, "Writing complete");

    KTINFO(testlog, "Test complete; see file output in test_writer.json");

    return 0;

}
