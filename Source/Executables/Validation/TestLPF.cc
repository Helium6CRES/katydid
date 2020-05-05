/*
 * TestCorrelator.cc
 *
 *  Created on: Sep 5, 2012
 *      Author: nsoblath
 */

#include "KTLPF.hh"
#include "KTLPFData.hh"

#include "KTFrequencySpectrumDataPolar.hh"
#include "KTFrequencySpectrumDataFFTW.hh"
#include "KTFrequencySpectrumFFTW.hh"
#include "KTFrequencySpectrumPolar.hh"
#include "KTLogger.hh"
#include "KTMath.hh"
#include "KTPowerSpectrum.hh"
#include "KTPowerSpectrumData.hh"

//Should delete below ones once it works
// #include "complexpolar.hh"
// #include "KTCorrelationData.hh"
// #include "KTCorrelator.hh"
// #include "KTFrequencySpectrumDataPolar.hh"
// #include "KTFrequencySpectrumDataFFTW.hh"
// #include "KTLogger.hh"

using namespace Katydid;

KTLOGGER(lpftestlog, "TestCorrelator");

int main()
{

    KTFrequencySpectrumDataFFTW* dataInput = new KTFrequencySpectrumDataFFTW();
    dataInput->SetNComponents(1);

    KTFrequencySpectrumFFTW* spectrum0 = new KTFrequencySpectrumFFTW(19, 0, 1e8); 
    // The second two arguments above are the min and max frequency in Hz. A typical egg file is 100 MHz in bandwidth. 
    (*spectrum0)(0)[0] = 0.; (*spectrum0)(0)[1] = 0.;
    (*spectrum0)(1)[0] = 0.; (*spectrum0)(1)[1] = 0.;
    (*spectrum0)(2)[0] = 1.; (*spectrum0)(2)[1] = 0.;
    (*spectrum0)(3)[0] = 2.; (*spectrum0)(3)[1] = 0.;
    (*spectrum0)(4)[0] = 3.; (*spectrum0)(4)[1] = 0.;
    (*spectrum0)(5)[0] = 4.; (*spectrum0)(5)[1] = 0.;
    (*spectrum0)(6)[0] = 5.; (*spectrum0)(6)[1] = 0.;
    (*spectrum0)(7)[0] = 6.; (*spectrum0)(7)[1] = 0.;
    (*spectrum0)(8)[0] = 0.; (*spectrum0)(8)[1] = 1.;
    (*spectrum0)(9)[0] = 0.; (*spectrum0)(9)[1] = 10.;
    (*spectrum0)(10)[0] = 0.; (*spectrum0)(10)[1] = 1.;
    (*spectrum0)(11)[0] = 6.; (*spectrum0)(11)[1] = 0.;
    (*spectrum0)(12)[0] = 5.; (*spectrum0)(12)[1] = 0.;
    (*spectrum0)(13)[0] = 4.; (*spectrum0)(13)[1] = 0.;
    (*spectrum0)(14)[0] = 3.; (*spectrum0)(14)[1] = 0.;
    (*spectrum0)(15)[0] = 2.; (*spectrum0)(15)[1] = 0.;
    (*spectrum0)(16)[0] = 1.; (*spectrum0)(16)[1] = 0.;
    (*spectrum0)(17)[0] = 0.; (*spectrum0)(17)[1] = 0.;
    (*spectrum0)(18)[0] = 0.; (*spectrum0)(18)[1] = 0.;


    dataInput->SetSpectrum(spectrum0, 0);
   

    KTINFO(lpftestlog, "Spectrum 0");
    spectrum0->Print(0, 19);

    // Execute the Low Pass Filtering
    KTLPF* lpf = new KTLPF();
    // Set the RC time constant of the filter: (Not sure how to do this at the time so going to wait...)

    double RC = 2e-8;
    lpf->SetTimeConst(RC); 
    KTINFO(lpftestlog, "The RC time constant of the low pass filter is: " << lpf->GetTimeConst());

    //Feed the dataInput data object into the LPF processor and identify the dataOutput object with its output. 
    
    if (! lpf->Filter(*dataInput))   //This means if (Filter() doesn't return true). 
    {
        KTERROR(lpftestlog, "Something went wrong during the low pass filtering");
        return -1;
    }
    KTFrequencySpectrumDataFFTW_LPF& dataOutput = dataInput->Of< KTFrequencySpectrumDataFFTW_LPF >();

    KTINFO(lpftestlog, "There are " << dataOutput.GetNComponents() << " output spectra");


    for (unsigned iSpectrum=0; iSpectrum < dataOutput.GetNComponents(); iSpectrum++)
    {
        KTINFO(lpftestlog, "Size of spectrum: " << dataOutput.GetSpectrumFFTW(iSpectrum)->size());
        dataOutput.GetSpectrumFFTW(iSpectrum)->Print(0, dataOutput.GetSpectrumFFTW(iSpectrum)->size());
    }


    // correlator->AddPair(KTCorrelator::UIntPair(0, 0));
    // correlator->AddPair(KTCorrelator::UIntPair(0, 1));
    // KTINFO(lpftestlog, "The correlator has " << correlator->GetPairVector().size() << " correlation pairs");

    // if (! correlator->Correlate(*dataInput))
    // {
    //     KTERROR(lpftestlog, "Something went wrong during the correlation");
    //     return -1;
    // }
    // KTCorrelationData& dataOutput = dataInput->Of< KTCorrelationData >();

    // KTINFO(lpftestlog, "There are " << dataOutput.GetNComponents() << " output spectra");
    // for (unsigned iSpectrum=0; iSpectrum < dataOutput.GetNComponents(); iSpectrum++)
    // {
    //     KTINFO(lpftestlog, "Output Spectrum " << iSpectrum << "; "
    //             "pair (" << dataOutput.GetInputPair().first << ", " <<
    //             dataOutput.GetInputPair().second << ")");
    //     KTINFO(lpftestlog, "Size of spectrum: " << dataOutput.GetSpectrumPolar(iSpectrum)->size());
    //     dataOutput.GetSpectrumPolar(iSpectrum)->Print(0, 10);
    // }

    // Clean up
    delete dataInput;
    delete lpf;

    return 0;
}


