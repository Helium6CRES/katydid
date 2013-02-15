/*
 * KTROOTTreeWriter.hh
 *
 *  Created on: Jan 23, 2012
 *      Author: nsoblath
 */

#ifndef KTROOTTREEWRITER_HH_
#define KTROOTTREEWRITER_HH_

#include "KTWriter.hh"

#include <set>

class TFile;
class TTree;

namespace Katydid
{
    class KTROOTTreeWriter;

    typedef KTDerivedTypeWriter< KTROOTTreeWriter > KTROOTTreeTypeWriter;



    class KTROOTTreeWriter : public KTWriterWithTypists< KTROOTTreeWriter >//public KTWriter
    {
        public:
            KTROOTTreeWriter();
            virtual ~KTROOTTreeWriter();

            Bool_t Configure(const KTPStoreNode* node);

        public:
            TFile* OpenFile(const std::string& filename, const std::string& flag);
            void CloseFile();

            const std::string& GetFilename() const;
            void SetFilename(const std::string& filename);

            const std::string& GetFileFlag() const;
            void SetFileFlag(const std::string& flag);

            TFile* GetFile();

            Bool_t OpenAndVerifyFile();
            void AddTree(TTree* newTree);

            void WriteTrees();

        protected:
            std::string fFilename;
            std::string fFileFlag;

            TFile* fFile;

            std::set< TTree* > fTrees; // Trees are not owned by this writer; they're owned by their respective TypeWriters.

    };

    inline const std::string& KTROOTTreeWriter::GetFilename() const
    {
        return fFilename;
    }
    inline void KTROOTTreeWriter::SetFilename(const std::string& filename)
    {
        fFilename = filename;
        return;
    }

    inline const std::string& KTROOTTreeWriter::GetFileFlag() const
    {
        return fFileFlag;
    }
    inline void KTROOTTreeWriter::SetFileFlag(const std::string& flag)
    {
        fFileFlag = flag;
        return;
    }

    inline TFile* KTROOTTreeWriter::GetFile()
    {
        return fFile;
    }


} /* namespace Katydid */
#endif /* KTROOTTREEWRITER_HH_ */
