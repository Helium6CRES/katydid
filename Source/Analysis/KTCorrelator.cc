/*
 * KTCorrelator.cc
 *
 *  Created on: Aug 20, 2012
 *      Author: nsoblath
 */

#include "KTCorrelator.hh"

#include "KTCorrelationData.hh"
#include "KTNOFactory.hh"
#include "KTFrequencySpectrumPolar.hh"
#include "KTFrequencySpectrumDataPolar.hh"
#include "KTFrequencySpectrumDataFFTW.hh"
#include "KTFrequencySpectrumFFTW.hh"
#include "KTLogger.hh"
#include "KTNormalizedFSData.hh"

using std::string;
using std::vector;



namespace Katydid
{
    KTLOGGER(corrlog, "katydid.analysis");

    static KTNORegistrar< KTProcessor, KTCorrelator > sCorrelatorRegistrar("correlator");

    KTCorrelator::KTCorrelator(const std::string& name) :
            KTProcessor(name),
            fPairs(),
            fCorrSignal("correlation", this),
            fFSPolarSlot("fs-polar", this, &KTCorrelator::Correlate, &fCorrSignal),
            fFSFFTWSlot("fs-fftw", this, &KTCorrelator::Correlate, &fCorrSignal),
            fNormFSPolarSlot("norm-fs-polar", this, &KTCorrelator::Correlate, &fCorrSignal),
            fNormFSFFTWSlot("norm-fs-fftw", this, &KTCorrelator::Correlate, &fCorrSignal)
    {
    }

    KTCorrelator::~KTCorrelator()
    {
    }

    bool KTCorrelator::Configure(const KTPStoreNode* node)
    {
        KTPStoreNode::csi_pair itPair = node->EqualRange("corr-pair");
        for (KTPStoreNode::const_sorted_iterator citer = itPair.first; citer != itPair.second; ++citer)
        {
            UIntPair pair = ParsePairUInt(citer->second.get_value< string >());
            KTINFO(corrlog, "Adding correlation pair " << pair.first << ", " << pair.second);
            this->AddPair(pair);
        }

        return true;
    }

    bool KTCorrelator::Correlate(KTFrequencySpectrumDataPolar& data)
    {
        KTCorrelationData& newData = data.Of< KTCorrelationData >().SetNComponents(fPairs.size());
        return CoreCorrelate(data, newData);
    }

    bool KTCorrelator::Correlate(KTFrequencySpectrumDataFFTW& data)
    {
        KTCorrelationData& newData = data.Of< KTCorrelationData >().SetNComponents(fPairs.size());
        return CoreCorrelate(data, newData);
    }

    bool KTCorrelator::Correlate(KTNormalizedFSDataPolar& data)
    {
        KTCorrelationData& newData = data.Of< KTCorrelationData >().SetNComponents(fPairs.size());
        return CoreCorrelate(data, newData);
    }

    bool KTCorrelator::Correlate(KTNormalizedFSDataFFTW& data)
    {
        KTCorrelationData& newData = data.Of< KTCorrelationData >().SetNComponents(fPairs.size());
        return CoreCorrelate(data, newData);
    }

    bool KTCorrelator::CoreCorrelate(KTFrequencySpectrumDataPolarCore& data, KTCorrelationData& newData)
    {
        unsigned iPair = 0;
        for (PairVector::const_iterator iter = fPairs.begin(); iter != fPairs.end(); ++iter)
        {
            unsigned firstChannel = (*iter).first;
            unsigned secondChannel = (*iter).second;
            KTFrequencySpectrumPolar* result = DoCorrelation(data.GetSpectrumPolar(firstChannel), data.GetSpectrumPolar(secondChannel));
            if (result == NULL)
            {
                KTWARN(corrlog, "Something went wrong with the correlation of channels " << firstChannel << " and " << secondChannel);
            }
            else
            {
                newData.SetSpectrum(result, iPair);
                newData.SetInputPair(firstChannel, secondChannel, iPair);
            }
            iPair++;
        }

        KTINFO(corrlog, "Correlations complete; " << iPair << " channel-pairs correlated.");
        return true;
    }

    bool KTCorrelator::CoreCorrelate(KTFrequencySpectrumDataFFTWCore& data, KTCorrelationData& newData)
    {
        unsigned iPair = 0;
        for (PairVector::const_iterator iter = fPairs.begin(); iter != fPairs.end(); ++iter)
        {
            unsigned firstChannel = (*iter).first;
            unsigned secondChannel = (*iter).second;
            KTFrequencySpectrumPolar* result = DoCorrelation(data.GetSpectrumFFTW(firstChannel), data.GetSpectrumFFTW(secondChannel));
            if (result == NULL)
            {
                KTWARN(corrlog, "Something went wrong with the correlation of channels " << firstChannel << " and " << secondChannel);
            }
            else
            {
                newData.SetSpectrum(result, iPair);
                newData.SetInputPair(firstChannel, secondChannel, iPair);
            }
            iPair++;
        }

        KTINFO(corrlog, "Correlations complete; " << iPair << " channel-pairs correlated.");
        return true;

    }

    KTFrequencySpectrumPolar* KTCorrelator::DoCorrelation(const KTFrequencySpectrumPolar* firstSpectrum, const KTFrequencySpectrumPolar* secondSpectrum)
    {
        // Performs cc(firstSpectrum) * secondSpectrum
        KTFrequencySpectrumPolar* newSpect = new KTFrequencySpectrumPolar(*firstSpectrum);
        newSpect->CConjugate();
        (*newSpect) *= (*secondSpectrum);
        return newSpect;
    }

    KTFrequencySpectrumPolar* KTCorrelator::DoCorrelation(const KTFrequencySpectrumFFTW* firstSpectrum, const KTFrequencySpectrumFFTW* secondSpectrum)
    {
        // Performs cc(firstSpectrum) * secondSpectrum
        KTFrequencySpectrumFFTW newSpectFFTW(*firstSpectrum);
        newSpectFFTW.CConjugate();
        newSpectFFTW *= (*secondSpectrum);

        return newSpectFFTW.CreateFrequencySpectrumPolar();
    }

} /* namespace Katydid */
