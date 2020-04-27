/*
 * KTProcessorTemplate.hh
 *
 *  Created on: Oct 13, 2016
 *      Author: ezayas
 */

#ifndef KTPROCESSORTEMPLATE_HH_
#define KTPROCESSORTEMPLATE_HH_

#include "KTProcessor.hh"

#include "KTMemberVariable.hh"
// #include "KTData.hh"
#include "KTSlot.hh"

#include <cmath>
//Why do we need the scarab namespace here: class param_node? 

namespace scarab
{
    class param_node;
}


namespace Katydid
{
    
    KTLOGGER(avlog_hh, "KTProcessorTemplate.hh"); // Why does he drop the logger in his .hh?



    // Why do we need to declare these classes here? Both the data and processors that can be input...
    class KTFrequencySpectrumDataFFTW;
    class KTFrequencySpectrumDataPolar;
    class KTFrequencySpectrumFFTW;
    class KTFrequencySpectrumPolar;
    class KTPowerSpectrum;
    class KTPowerSpectrumData;

    /*
     @class KTProcessorTemplate
     @author E. Zayas

     @brief An example processor to show the structure of these classes

     @details
     This is a "template" processor which has member variables, signals, and slots which act on fictional data objects. Its purpose
     is to show the general structure of a processor class, and to be used as a starting point when creating a new processor.

     Configuration name: "processor-template"

     Available configuration values:
     - "member-variable-1": double -- An example member variable
     - "member-variable-2": double -- An example member variable
     - "member-variable-3": double -- An example member variable

     Slots:
     - "dummy-slot-1": void (Nymph::KTDataPtr) -- Does something with dummy data object #1; Requires KTDummyDataObject1; Adds something perhaps (in this case it will add nothing)
     - "dummy-slot-2": void (Nymph::KTDataPtr) -- Does something with dummy data object #2; Requires KTDummyDataObject2; Adds something perhaps (in this case it will add nothing)
     - "dummy-slot-3": void (Nymph::KTDataPtr) -- Does something with dummy data object #3; Requires KTDummyDataObject3; Adds something perhaps (in this case it will add nothing)

     Signals:
     - "dummy-signal-1": void (Nymph::KTDataPtr) -- Emitted upon successful processing of dummy data object #1; Guarantees KTDummyDataObject1
     - "dummy-signal-2": void (Nymph::KTDataPtr) -- Emitted upon successful processing of dummy data object #2; Guarantees KTDummyDataObject2
     - "dummy-signal-3": void (Nymph::KTDataPtr) -- Emitted upon successful processing of dummy data object #3; Guarantees KTDummyDataObject3
    */

    class KTLPF : public Nymph::KTProcessor
    {
        public:
            KTLPF(const std::string& name = "lp-filter");
            virtual ~KTLPF();  //Why is the destructor virtual?

            bool Configure(const scarab::param_node* node); // This lets the processor be seen by the config files.


            MEMBERVARIABLE(double, TimeConst); // This is a getter and setter macro for the membervariables. 

        private:
            double fMemberVariable1;
            double fMemberVariable2;
            double fMemberVariable3;

        public:
            // Below we declare the filter functions
            // And are we also declaring the datapointers in the second 3 lines? 

            bool Filter(KTFrequencySpectrumDataPolar& fsData);
            bool Filter(KTFrequencySpectrumDataFFTW& fsData);
            bool Filter(KTPowerSpectrumData& psData);

            // Why is the name frequencyspectrum used and not fsData? What is happening below?

            KTFrequencySpectrumPolar* Filter(const KTFrequencySpectrumPolar* frequencySpectrum) const;
            KTFrequencySpectrumFFTW* Filter(const KTFrequencySpectrumFFTW* frequencySpectrum) const;
            KTPowerSpectrum* Filter(const KTPowerSpectrum* powerSpectrum) const;



            //***************
            // Signals
            //***************

        private:
            Nymph::KTSignalData fFSPolarSignal;
            Nymph::KTSignalData fFSFFTWSignal;
            Nymph::KTSignalData fPSSignal;

            //***************
            // Slots: Note: Here we have to declare the slot datatype as well as give the slots names. 
            //***************

        private:
            Nymph::KTSlotDataOneType< KTFrequencySpectrumDataPolar > fFSPolarSlot;
            Nymph::KTSlotDataOneType< KTFrequencySpectrumDataFFTW > fFSFFTWSlot;
            Nymph::KTSlotDataOneType< KTPowerSpectrumData > fPSSlot;
    };

// There could be memeber variable getter and setter functions that would go here.
    
}

#endif /* KTPROCESSORTEMPLATE_HH_ */
