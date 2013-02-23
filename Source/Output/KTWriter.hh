/*
 * KTWriter.hh
 *
 *  Created on: Aug 24, 2012
 *      Author: nsoblath
 */

#ifndef KTWRITER_HH_
#define KTWRITER_HH_

#include "KTProcessor.hh"

#include "KTTIFactory.hh"

#include <map>
#include <typeinfo>

namespace Katydid
{
    class KTWriteableData;




    class KTTypeWriter
    {
        public:
            KTTypeWriter();
            virtual ~KTTypeWriter();

            virtual void RegisterSlots() = 0;
    };


    template< class XWriter >
    class KTDerivedTypeWriter : public KTTypeWriter
    {
        public:
            KTDerivedTypeWriter();
            virtual ~KTDerivedTypeWriter();

            void SetWriter(XWriter* writer);

        protected:
            XWriter* fWriter;
    };


    template< class XWriter >
    KTDerivedTypeWriter< XWriter >::KTDerivedTypeWriter() :
            KTTypeWriter(),
            fWriter(NULL)
    {
    }

    template< class XWriter >
    KTDerivedTypeWriter< XWriter >::~KTDerivedTypeWriter()
    {
    }

    template< class XWriter >
    void KTDerivedTypeWriter< XWriter >::SetWriter(XWriter* writer)
    {
        fWriter = writer;
        return;
    }




    class KTWriter : public KTProcessor
    {
        public:
            KTWriter();
            virtual ~KTWriter();

            virtual void Publish(const KTWriteableData* data) = 0;

            virtual void Write(const KTWriteableData* data) = 0;

    };

    template< class XWriter >
    class KTWriterWithTypists : public KTWriter
    {
        protected:
            typedef std::map< const std::type_info*, KTDerivedTypeWriter< XWriter >* > TypeWriterMap;
        public:
            KTWriterWithTypists();
            virtual ~KTWriterWithTypists();

            template< class XTypeWriter >
            XTypeWriter* GetTypeWriter();

        protected:
            TypeWriterMap fTypeWriters;

    };


    template< class XWriter >
    KTWriterWithTypists< XWriter >::KTWriterWithTypists() :
            KTWriter(),
            fTypeWriters()
    {
        KTTIFactory< KTDerivedTypeWriter< XWriter > >* twFactory = KTTIFactory< KTDerivedTypeWriter< XWriter > >::GetInstance();
        for (typename KTTIFactory< KTDerivedTypeWriter< XWriter > >::FactoryCIt factoryIt = twFactory->GetFactoryMapBegin();
                factoryIt != twFactory->GetFactoryMapEnd();
                factoryIt++)
        {
            KTDerivedTypeWriter< XWriter >* newTypeWriter = twFactory->Create(factoryIt);
            newTypeWriter->SetWriter(static_cast< XWriter* >(this));
            newTypeWriter->RegisterSlots();
            fTypeWriters.insert(typename TypeWriterMap::value_type(factoryIt->first, newTypeWriter));
        }
    }

    template< class XWriter >
    KTWriterWithTypists< XWriter >::~KTWriterWithTypists()
    {
        while (! fTypeWriters.empty())
        {
            delete fTypeWriters.begin()->second;
            fTypeWriters.erase(fTypeWriters.begin());
        }
    }

    template< class XWriter >
    template< class XTypeWriter >
    XTypeWriter* KTWriterWithTypists< XWriter >::GetTypeWriter()
    {
        typename TypeWriterMap::const_iterator it = fTypeWriters.find(&typeid(XTypeWriter));
        if (it == fTypeWriters.end())
        {
            return NULL;
        }
        return static_cast< XTypeWriter* >(it->second);
    }



} /* namespace Katydid */
#endif /* KTWRITER_HH_ */
