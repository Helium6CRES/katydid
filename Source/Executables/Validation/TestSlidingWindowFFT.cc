/*
 * TestSlidingWindowFFT.cc
 *
 *  Created on: Dec 22, 2011
 *      Author: nsoblath
 *
 *  Usage:
 *      > TestSlidingWindowFFT filename.egg [fftw]
 *
 *      If "fftw" is given as the second argument, KTSlidingWindowFFTW is used instead of KTSlidingWindowFFT.
 */

#include "KTEgg.hh"
#include "KTEggHeader.hh"
#include "KTEggReaderMonarch.hh"
#include "KTBundle.hh"
#include "KTLogger.hh"
#include "KTRectangularWindow.hh"
#include "KTSlidingWindowFFT.hh"
#include "KTSlidingWindowFFTW.hh"
#include "KTSlidingWindowFSData.hh"
#include "KTSlidingWindowFSDataFFTW.hh"
#include "KTTimeSeriesChannelData.hh"

#ifdef ROOT_FOUND
#include "KTBasicROOTFileWriter.hh"
#endif


using namespace std;
using namespace Katydid;


KTLOGGER(testsw, "TestSlidingWindowFFT");


int main(int argc, char** argv)
{

    if (argc < 2)
    {
        KTERROR(testsw, "No filename supplied");
        return 0;
    }
    string filename(argv[1]);
    bool UseFFTW = false;
    if (argc > 2)
    {
        UseFFTW = string(argv[2]) == "fftw";
    }

    KTINFO(testsw, "Test of hatching egg file <" << filename << ">");

    KTEgg egg;
    unsigned recordSize = 0;
    KTINFO(testsw, "Record size will be " << recordSize << " (if 0, it should be the same as the Monarch record size)");
    KTEggReaderMonarch* reader = new KTEggReaderMonarch();
    reader->SetTimeSeriesSizeRequest(recordSize);
    if (! UseFFTW)
    {
        reader->SetTimeSeriesType(KTEggReaderMonarch::kRealTimeSeries);
    }
    else
    {
        reader->SetTimeSeriesType(KTEggReaderMonarch::kFFTWTimeSeries);
    }
    egg.SetReader(reader);

    KTINFO(testsw, "Opening file");
    if (egg.BreakEgg(filename))
    {
        KTINFO(testsw, "Egg opened successfully");
    }
    else
    {
        KTERROR(testsw, "Egg file was not opened");
        return -1;
    }

    const KTEggHeader* header = egg.GetHeader();
    if (header == NULL)
    {
        KTERROR(testsw, "No header received");
        egg.CloseEgg();
        return -1;
    }

    KTINFO(testsw, "Hatching bundle");
    // Hatch the bundle
    boost::shared_ptr<KTBundle> bundle = egg.HatchNextBundle();
    if (bundle.get() == NULL)
    {
        KTERROR(testsw, "Bundle did not hatch");
        egg.CloseEgg();
        return -1;
    }

    // Get the time-series data from the bundle.
    // The data is still owned by the bundle.
    KTTimeSeriesData* tsData = bundle->GetData<KTProgenitorTimeSeriesData>("time-series");
    if (tsData == NULL)
    {
        KTWARN(testsw, "No time-series data present in bundle");
        egg.CloseEgg();
        return -1;
    }

    unsigned windowSize = 512;
    if (! UseFFTW)
    {
        // Create the transform, and manually configure it.
        KTINFO(testsw, "Creating and configuring sliding window FFT");
        KTSlidingWindowFFT fft;
        KTBundleWindowFunction* windowFunc = new KTRectangularWindow(tsData);
        windowFunc->SetSize(windowSize);
        fft.SetTransformFlag("ESTIMATE");
        fft.SetWindowFunction(windowFunc);
        fft.SetOverlap((unsigned)0);
        fft.InitializeFFT();

        // Transform the data.
        // The data is not owned by the bundle because TransformData was used, not ProcessBundle.
        KTINFO(testsw, "Transforming data");
        KTSlidingWindowFSData* swData = fft.TransformData(tsData);

        if (swData == NULL)
        {
            KTWARN(testsw, "No data was returned by the FFT: test failed");
        }
        else
        {
#ifdef ROOT_FOUND
            KTBasicROOTFileWriter writer;
            writer.SetFilename("SWFFTTest.root");
            writer.SetFileFlag("recreate");

            KTINFO(testsw, "Writing data to file");
            writer.Publish(swData);
#endif
        }
        delete swData;
    }
    else
    {
        // Create the transform, and manually configure it.
        KTINFO(testsw, "Creating and configuring sliding window FFTW");
        KTSlidingWindowFFTW fft;
        KTBundleWindowFunction* windowFunc = new KTRectangularWindow(tsData);
        windowFunc->SetSize(windowSize);
        fft.SetTransformFlag("ESTIMATE");
        fft.SetWindowFunction(windowFunc);
        fft.SetOverlap((unsigned)0);
        fft.InitializeFFT();

        // Transform the data.
        // The data is not owned by the bundle because TransformData was used, not ProcessBundle.
        KTINFO(testsw, "Transforming data");
        KTSlidingWindowFSDataFFTW* swData = fft.TransformData(tsData);

        if (swData == NULL)
        {
            KTWARN(testsw, "No data was returned by the FFT: test failed");
        }
        else
        {
#ifdef ROOT_FOUND
            KTBasicROOTFileWriter writer;
            writer.SetFilename("SWFFTWTest.root");
            writer.SetFileFlag("recreate");

            KTINFO(testsw, "Writing data to file");
            writer.Publish(swData);
#endif
        }
        delete swData;
    }

    KTINFO(testsw, "Test complete; cleaning up");
    egg.CloseEgg();

    return 0;


}
