/*
 * KTProcessor.cc
 *
 *  Created on: Jan 5, 2012
 *      Author: nsoblath
 */

#include "KTProcessor.hh"

#include <boost/foreach.hpp>

namespace Katydid
{
    ProcessorException::ProcessorException (std::string const& why)
      : std::logic_error(why)
    {}


    KTProcessor::KTProcessor() :
            fSignalMap(),
            fSlotMap()
    {
    }

    KTProcessor::~KTProcessor()
    {
        for (SlotMapIt iter = fSlotMap.begin(); iter != fSlotMap.end(); iter++)
        {
            iter->second->Disconnect();
            delete iter->second;
        }
        for (SigMapIt iter = fSignalMap.begin(); iter != fSignalMap.end(); iter++)
        {
            delete iter->second;
        }
    }

    void KTProcessor::ConnectASlot(const std::string& signalName, KTProcessor* processor, const std::string& slotName, int groupNum)
    {
        KTSignalWrapper* signal = GetSignal(signalName);
        KTSlotWrapper* slot = processor->GetSlot(slotName);

        try
        {
            ConnectSignalToSlot(signal, slot, groupNum);
        }
        catch (std::exception& e)
        {
            throw e;
        }
        return;
    }

    void KTProcessor::ConnectASignal(KTProcessor* processor, const std::string& signalName, const std::string& slotName, int groupNum)
    {
        KTSignalWrapper* signal = processor->GetSignal(signalName);
        KTSlotWrapper* slot = GetSlot(slotName);

        try
        {
            ConnectSignalToSlot(signal, slot, groupNum);
        }
        catch (std::exception& e)
        {
            throw e;
        }
        return;
    }

    void KTProcessor::ConnectSignalToSlot(KTSignalWrapper* signal, KTSlotWrapper* slot, int groupNum)
    {
        if (signal == NULL)
        {
            throw ProcessorException("Signal pointer was NULL");
        }
        if (slot == NULL)
        {
            throw ProcessorException("Slot pointer was NULL");
        }

        try
        {
            slot->SetConnection(signal, groupNum);
        }
        catch (std::exception& e)
        {
            throw e;
        }

        return;
    }

    KTSignalWrapper* KTProcessor::GetSignal(const std::string& name)
    {
        SigMapIt iter = fSignalMap.find(name);
        if (iter == fSignalMap.end())
        {
            return NULL;
        }
        return iter->second;
    }

    KTSlotWrapper* KTProcessor::GetSlot(const std::string& name)
    {
        SlotMapIt iter = fSlotMap.find(name);
        if (iter == fSlotMap.end())
        {
            return NULL;
        }
        return iter->second;
    }



} /* namespace Katydid */