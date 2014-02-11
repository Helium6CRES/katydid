/**
 @file KTEggProcessor.hh
 @brief Contains KTEggProcessor
 @details Iterates over slices in an Egg file.
 @author: N. S. Oblath
 @date: Jan 5, 2012
 */

#ifndef KTEGGPROCESSOR_HH_
#define KTEGGPROCESSOR_HH_

#include "KTPrimaryProcessor.hh"

#include "KTData.hh"
#include "KTEggReader.hh"
#include "KTSlot.hh"


namespace Katydid
{
    class KTDAC;
    class KTProcSummary;
    class KTPStoreNode;
    class KTTimeSeriesData;

    /*!
     @class KTEggProcessor
     @author N. S. Oblath

     @brief Iterates over the slices in an Egg file.

     @details
     Iterates over slices in an egg file; slices are extracted until fNSlices is reached.

     Configuration name: "egg-processor"

     Available configuration options:
     - "number-of-slices": unsigned -- Number of slices to process
     - "progress-report-interval": unsigned -- Interval (# of slices) between reporting progress (mainly relevant for RELEASE builds); turn off by setting to 0
     - "filename": string -- Egg filename to use
     - "egg-reader": string -- Egg reader to use (options: monarch [default], 2011)
     - "slice-size": unsigned -- Specify the size of the time series (required)
     - "stride": unsigned -- Specify how many bins to advance between slices (leave unset to make stride == slice-size; i.e. no overlap or skipping between slices)
     - "normalize-voltages": bool -- Flag to toggle the normalization of ADC values from the egg file (default: true)
     - "dac": object -- configure the DAC

     Command-line options defined
     - -n (n-slices): Number of slices to process
     - -e (egg-file): Egg filename to use
     - -z (--use-2011-egg-reader): Use the 2011 egg reader

     Signals:
     - "header": void (const KTEggHeader*) -- emitted when the file header is parsed.
     - "raw-ts" void (KTDataPtr) -- emitted when a new raw time series is produced; guarantees KTRawTimeSeriesData
     - "ts": void (KTDataPtr) -- emitted when the new calibrated time series is produced; Guarantees KTTimeSeriesData
     - "egg-done": void () --  emitted when a file is finished.
     - "summary": void (const KTProcSummary*) -- emitted when a file is finished (after "egg-done")
    */
    class KTEggProcessor : public KTPrimaryProcessor
    {
        public:
            enum EggReaderType
            {
                k2011EggReader,
                kMonarchEggReader
            };

        public:
            KTEggProcessor(const std::string& name = "egg-processor");
            virtual ~KTEggProcessor();

            bool Configure(const KTPStoreNode* node);

            unsigned GetNSlices() const;
            void SetNSlices(unsigned nSlices);

            unsigned GetProgressReportInterval() const;
            void SetProgressReportInterval(unsigned nSlices);

            const std::string& GetFilename() const;
            void SetFilename(const std::string& filename);

            EggReaderType GetEggReaderType() const;
            void SetEggReaderType(EggReaderType type);

            unsigned GetSliceSize() const;
            void SetSliceSize(unsigned size);

            unsigned GetStride() const;
            void SetStride(unsigned stride);

            bool GetNormalizeVoltages() const;
            void SetNormalizeVoltages(bool flag);

        private:
            unsigned fNSlices;
            unsigned fProgressReportInterval;

            std::string fFilename;

            EggReaderType fEggReaderType;

            unsigned fSliceSize;
            unsigned fStride;

            KTDAC* fDAC;

            bool fNormalizeVoltages;

        public:
            bool Run();

            bool ProcessEgg();

            bool HatchNextSlice(KTEggReader* reader, KTDataPtr& data) const;
            void NormalizeData(KTDataPtr& data);

        private:
            void UnlimitedLoop(KTEggReader* reader);
            void LimitedLoop(KTEggReader* reader);


            //***************
            // Signals
            //***************

        private:
            KTSignalOneArg< const KTEggHeader* > fHeaderSignal;
            KTSignalData fRawDataSignal;
            KTSignalData fDataSignal;
            KTSignalOneArg< void > fEggDoneSignal;
            KTSignalOneArg< const KTProcSummary* > fSummarySignal;

    };

    inline bool KTEggProcessor::Run()
    {
        return ProcessEgg();
    }

    inline bool KTEggProcessor::HatchNextSlice(KTEggReader* reader, KTDataPtr& data) const
    {
        data = reader->HatchNextSlice();
        if (data) return true;
        return false;
    }

    inline unsigned KTEggProcessor::GetNSlices() const
    {
        return fNSlices;
    }

    inline unsigned KTEggProcessor::GetProgressReportInterval() const
    {
        return fProgressReportInterval;
    }

    inline void KTEggProcessor::SetNSlices(unsigned nSlices)
    {
        fNSlices = nSlices;
        return;
    }

    inline void KTEggProcessor::SetProgressReportInterval(unsigned nSlices)
    {
        fProgressReportInterval = nSlices;
        return;
    }

    inline const std::string& KTEggProcessor::GetFilename() const
    {
        return fFilename;
    }

    inline void KTEggProcessor::SetFilename(const std::string& filename)
    {
        fFilename = filename;
        return;
    }

    inline KTEggProcessor::EggReaderType KTEggProcessor::GetEggReaderType() const
    {
        return fEggReaderType;
    }

    inline void KTEggProcessor::SetEggReaderType(EggReaderType type)
    {
        fEggReaderType = type;
        return;
    }

    inline unsigned KTEggProcessor::GetSliceSize() const
    {
        return fSliceSize;
    }

    inline void KTEggProcessor::SetSliceSize(unsigned size)
    {
        fSliceSize = size;
        return;
    }

    inline unsigned KTEggProcessor::GetStride() const
    {
        return fStride;
    }

    inline void KTEggProcessor::SetStride(unsigned stride)
    {
        fStride = stride;
        return;
    }

    inline bool KTEggProcessor::GetNormalizeVoltages() const
    {
        return fNormalizeVoltages;
    }

    inline void KTEggProcessor::SetNormalizeVoltages(bool flag)
    {
        fNormalizeVoltages = flag;
        return;
    }

} /* namespace Katydid */

/*!
 \mainpage Katydid

 <br>
 \section Resource Resources at your disposal
 <hr>
 \li <a href="https://github.com/project8/katydid/wiki">User's Manual</a> -- on Github
 \li Reference Manual -- this Doxygen-based source documentation

 <br>
 \section Help Getting Help
 <hr>
 \li Send your question by email to Noah Oblath: nsoblath-at-mit.edu
 \li For installation problems see below.

 <br>
 \section Requirements System Requirements
 <hr>
 Linux/MacOS with a reasonably new C++ compiler:
 \li The minimum supported gcc version is 4.2.
 \li LLVM (minimum version unknown)

 Dependencies:
 \li <a href="https://github.com/project8/monarch">Monarch</a> included as a submodule of Katyid
 \li <a href="http://www.cmake.org">CMake</a> version 2.6 or higher
 \li <a href="https://code.google.com/p/protobuf/">Protobuf</a>
 \li <a href="http://www.boost.org">Boost</a>
 \li <a href="http://www.fftw.org">FFTW</a> version 3.3 or higher
 \li <a href="http://eigen.tuxfamily.org">Eigen</a> (optional, though some functionality wil be disabled without it)
 \li <a href="http://root.cern.ch/drupal">ROOT</a> version 5.24 or higher (optional, though some functionality will be disabled without it)
 \li <a href="http://logging.apache.org/log4cxx">log4cxx</a> (optional)

 <br>
 \section GettingKT Getting Katydid
 <hr>
 You have two options for downloading any distribution of Katydid:

 <ul>
 <li><b>Distribution Tarball</b><br>
 This option is not yet available
 </li>

 <li> <b>Github</b><br>
 Katydid source can be found on the <a href="https://github.com/project8/katydid">Project 8 Github repository</a>.
 You will need to have CMake installed on your computer (version 2.6 or higher). It's also useful to have ccmake or cmake-gui to make the configuration step easier.

 The master branch of the source code can be cloned from the Github repository in the standard way (assuming you ahve Git installed and configured on your computer):
 \code
 >  git clone https://github.com/project8/katydid.git
 >  cd katydid
 \endcode
 </li>

 <br>
 \section Installing Installing
 <hr>
 The simplest way to configure Katydid is to use the <tt>cmake</tt> command:
 \code
 > cmake .
 \endcode

 Alternatively you can use <tt>ccmake</tt> or <tt>cmake-gui</tt> to interactively select your configuration:
 \code
 > ccmake .
 \endcode
 or
 \code
 > cmake-gui .
 \endcode

 The following two commands will, respectively, compile and install Katydid:
 \code
 > make
 > make install
 \endcode

 If everything compiles and installs correctly, Katydid is ready to use.  If there were problems with the installation process, please see the Installation Problems section below.

 <br>
 \section Problems Installation Problems
 <hr>
 \li There's a bug in Boost that's a problem when compiling with LLVM (including using the default compiler in Mac OS X 10.7).  This was fixed in Boost 1.49.00.  If you don't have access to that version you can fix the problem by making the following change:  on line 565 of boost/signals2/detail/auto_buffer.hpp, change the word "size" to "size_arg" (not including the quotes).  Boost need not be recompiled.

 <br>
 \section ExternalCode External Packages and Imported Code
 <hr>
 Two external packages are distributed with Katydid:
 \li <a href="http://rapidxml.sourceforge.net">RapidXml</a> is used for parsing the header in the 2011-type Egg files.
 \li <a href="https://code.google.com/p/rapidjson/">RapidJSON</a> is used for reading and writing JSON files.

 Code has also been imported with permission from the Kassiopeia package developed by the KATRIN collaboration.  The imported code resides in the Utility and Core libraries and is restricted to infrastructure-related activities.

 <!--The source of this documentation can be found in: Katydid/Egg/KTEggProcessor.hh-->

 */

#endif /* KTEGGPROCESSOR_HH_ */
