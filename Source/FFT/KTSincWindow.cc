/*
 * KTSincWindow.cc
 *
 *  Created on: Apr 18, 2013
 *      Author: nsoblath
 */

#include "KTSincWindow.hh"

#include "KTNOFactory.hh"
#include "KTLogger.hh"
#include "KTMath.hh"
#include "KTPStoreNode.hh"

#include <cmath>

using std::string;

namespace Katydid
{
    KTLOGGER(windowlog, "katydid.fft");

    static KTDerivedNORegistrar< KTWindowFunction, KTSincWindow > sWFSincRegistrar("sinc");

    KTSincWindow::KTSincWindow(const string& name) :
            KTWindowFunction(name)
    {
    }

    KTSincWindow::~KTSincWindow()
    {
    }

    bool KTSincWindow::ConfigureWFSubclass(const KTPStoreNode*)
    {
        KTDEBUG(windowlog, "Sinc WF configured");
        return true;
    }

    double KTSincWindow::GetWeight(double time) const
    {
        return GetWeight(KTMath::Nint(time / fBinWidth));
    }

    void KTSincWindow::RebuildWindowFunction()
    {
        fWindowFunction.resize(fSize);
        double xVal = 0.;
        int halfSize = int(fSize / 2);
        double twoPiOverNBinsMinus1 = KTMath::TwoPi() / (double)(halfSize - 1);
        for (unsigned iBin=0; iBin < fSize; iBin++)
        {
            xVal = twoPiOverNBinsMinus1 * double(iBin-halfSize);
            fWindowFunction[iBin] = xVal == 0. ? 1. : sin(xVal) / xVal;
        }
        return;
    }


} /* namespace Katydid */
