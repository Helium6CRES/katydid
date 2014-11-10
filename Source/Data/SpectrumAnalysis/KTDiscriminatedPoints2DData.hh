/*
 * KTDiscriminatedPoints2DData.hh
 *
 *  Created on: Dec 12, 2012
 *      Author: nsoblath
 */

#ifndef KTDISCRIMINATEDPOINTS2DDATA_HH_
#define KTDISCRIMINATEDPOINTS2DDATA_HH_

#include "KTData.hh"

#include <map>
#include <utility>
#include <vector>

namespace Katydid
{
    struct KTPairCompare
    {
        bool operator() (const std::pair< unsigned, unsigned >& lhs, const std::pair< unsigned, unsigned >& rhs) const
        {
            return lhs.first < rhs.first || (lhs.first == rhs.first && lhs.second < rhs.second);
        }
    };

    class KTDiscriminatedPoints2DData : public KTExtensibleData< KTDiscriminatedPoints2DData >
    {
        public:
            typedef std::map< std::pair< unsigned, unsigned >, double, KTPairCompare > SetOfPoints;

        protected:
            struct PerComponentData
            {
                SetOfPoints fPoints;
                double fThreshold;
            };

        public:
            KTDiscriminatedPoints2DData();
            virtual ~KTDiscriminatedPoints2DData();

            const SetOfPoints& GetSetOfPoints(unsigned component = 0) const;
            double GetThreshold(unsigned component = 0) const;

            unsigned GetNComponents() const;

            void AddPoint(unsigned pointX, unsigned pointY, double value, unsigned component = 0);
            void SetThreshold(double threshold, unsigned component = 0);

            KTDiscriminatedPoints2DData& SetNComponents(unsigned channels);

            unsigned GetNBinsX() const;
            unsigned GetNBinsY() const;
            double GetBinWidthX() const;
            double GetBinWidthY() const;

            void SetNBinsX(unsigned nBins);
            void SetNBinsY(unsigned nBins);
            void SetBinWidthX(double binWidth);
            void SetBinWidthY(double binWidth);

        private:
            std::vector< PerComponentData > fComponentData;

            unsigned fNBinsX;
            unsigned fNBinsY;
            double fBinWidthX;
            double fBinWidthY;

        public:
            static const std::string sName;
    };

    inline const KTDiscriminatedPoints2DData::SetOfPoints& KTDiscriminatedPoints2DData::GetSetOfPoints(unsigned component) const
    {
        return fComponentData[component].fPoints;
    }

    inline double KTDiscriminatedPoints2DData::GetThreshold(unsigned component) const
    {
        return fComponentData[component].fThreshold;
    }

    inline unsigned KTDiscriminatedPoints2DData::GetNComponents() const
    {
        return unsigned(fComponentData.size());
    }

    inline void KTDiscriminatedPoints2DData::AddPoint(unsigned pointX, unsigned pointY, double value, unsigned component)
    {
        if (component >= fComponentData.size()) fComponentData.resize(component+1);
        fComponentData[component].fPoints.insert(std::make_pair(std::make_pair(pointX, pointY), value));
    }

    inline void KTDiscriminatedPoints2DData::SetThreshold(double threshold, unsigned component)
    {
        if (component >= fComponentData.size()) fComponentData.resize(component+1);
        fComponentData[component].fThreshold = threshold;
    }

    inline KTDiscriminatedPoints2DData& KTDiscriminatedPoints2DData::SetNComponents(unsigned channels)
    {
        fComponentData.resize(channels);
        return *this;
    }

    inline unsigned KTDiscriminatedPoints2DData::GetNBinsX() const
    {
        return fNBinsX;
    }

    inline unsigned KTDiscriminatedPoints2DData::GetNBinsY() const
    {
        return fNBinsY;
    }

    inline double KTDiscriminatedPoints2DData::GetBinWidthX() const
    {
        return fBinWidthX;
    }

    inline double KTDiscriminatedPoints2DData::GetBinWidthY() const
    {
        return fBinWidthY;
    }

    inline void KTDiscriminatedPoints2DData::SetNBinsX(unsigned nBins)
    {
        fNBinsX = nBins;
        return;
    }

    inline void KTDiscriminatedPoints2DData::SetNBinsY(unsigned nBins)
    {
        fNBinsY = nBins;
        return;
    }

    inline void KTDiscriminatedPoints2DData::SetBinWidthX(double binWidth)
    {
        fBinWidthX = binWidth;
        return;
    }

    inline void KTDiscriminatedPoints2DData::SetBinWidthY(double binWidth)
    {
        fBinWidthY = binWidth;
        return;
    }

} /* namespace Katydid */

#endif /* KTDISCRIMINATEDPOINTS2DDATA_HH_ */