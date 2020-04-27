/**
 @file KTLPFData.hh
 @brief Contains LPF data types
 @details 
 @author: Drew Byron
 @date: March 31, 2020
 */

#ifndef KTLPFDATA_HH_
#define KTLPFDATA_HH_

#include "KTFrequencySpectrumDataFFTW.hh"
#include "KTFrequencySpectrumDataPolar.hh"
#include "KTPowerSpectrumData.hh"
namespace Katydid
{


    class KTFrequencySpectrumDataFFTW_LPF : public KTFrequencySpectrumDataFFTWCore, public Nymph::KTExtensibleData< KTFrequencySpectrumDataFFTW >
    {
        public:
            KTFrequencySpectrumDataFFTW_LPF();
            virtual ~KTFrequencySpectrumDataFFTW_LPF();

            virtual KTFrequencySpectrumDataFFTW_LPF& SetNComponents(unsigned components);

        public:
            static const std::string sName;

    };

    class KTFrequencySpectrumDataPolar_LPF : public KTFrequencySpectrumDataPolarCore, public Nymph::KTExtensibleData< KTFrequencySpectrumDataPolar >
    {
        public:
            KTFrequencySpectrumDataPolar_LPF();
            virtual ~KTFrequencySpectrumDataPolar_LPF();

            KTFrequencySpectrumDataPolar_LPF& SetNComponents(unsigned component);

        public:
            static const std::string sName;

    };

    class KTPowerSpectrumData_LPF : public KTPowerSpectrumDataCore, public Nymph::KTExtensibleData< KTPowerSpectrumData >
    {
        public:
            KTPowerSpectrumData_LPF();
            virtual ~KTPowerSpectrumData_LPF();

            KTPowerSpectrumData_LPF& SetNComponents(unsigned channels);

        public:
            static const std::string sName;

    };
} /* namespace Katydid */
#endif /* KTLPFDATA_HH_ */
