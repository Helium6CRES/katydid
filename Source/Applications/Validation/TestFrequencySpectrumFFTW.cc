/*
 * TestFrequencySpectrumFFTW.cc
 *
 *  Created on: Nov 8, 2012
 *      Author: nsoblath
 */


#include "KTFrequencySpectrumFFTW.hh"
#include "KTLogger.hh"

#include <sstream>

using namespace Katydid;
using namespace std;

KTLOGGER(testlog, "applications.validation");

int main()
{
    KTINFO(testlog, "Creating a KTPhysicalVectorFFTW and a KTFrequencySpectrumFFTW of length 10, and filling with integers from 0 to 9.");
    KTINFO(testlog, "Access to the arrays will be via operator().");

    unsigned size = 10;
    double min = 0.;
    double max = 10.;

    KTPhysicalArray<1, fftw_complex> physArr(size, min, max);
    KTFrequencySpectrumFFTW freqSpect(size, min, max);

    for (unsigned i=0; i<size; i++)
    {
        physArr(i)[0] = double(i);
        physArr(i)[1] = 0.;
        freqSpect(i)[0] = double(i);
        freqSpect(i)[1] = 0.;
    }

    KTINFO(testlog, "Printing contents of the two arrays by accessing the data storage directly.");
    const KTPhysicalArray< 1, fftw_complex >::array_type dataForPhysArr = physArr.GetData();
    const KTFrequencySpectrumFFTW::array_type dataForFreqSpect = freqSpect.GetData();

    stringstream printer;
    printer << '\n';
    printer << "\tphysArr\tfreqSpect\n" << "\t-------\t---------\n";
    for (unsigned i=0; i<size; i++)
    {
        printer << '\t' << dataForPhysArr[i] << '\t' << dataForFreqSpect[i] << '\n';
    }
    KTWARN(testlog, printer.str());

    KTINFO(testlog, "Test complete.");

    return 0;
}

