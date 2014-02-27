/*
 * KTHannWindow.cc
 *
 *  Created on: Sep 18, 2011
 *      Author: nsoblath
 */

#include "KTHannWindow.hh"

#include "KTNOFactory.hh"
#include "KTLogger.hh"
#include "KTMath.hh"
#include "KTParam.hh"

#include <cmath>

using std::string;

namespace Katydid
{
    static KTNORegistrar< KTWindowFunction, KTHannWindow > sWFHannRegistrar("hann");

    KTLOGGER(windowlog, "KTHannWindow");

    KTHannWindow::KTHannWindow(const string& name) :
            KTWindowFunction(name)
    {
    }

    KTHannWindow::~KTHannWindow()
    {
    }

    bool KTHannWindow::ConfigureWFSubclass(const KTParamNode*)
    {
        KTDEBUG(windowlog, "Hann WF configured");
        return true;
    }

    double KTHannWindow::GetWeight(double time) const
    {
        return GetWeight(KTMath::Nint(time / fBinWidth));
    }

    void KTHannWindow::RebuildWindowFunction()
    {
        fWindowFunction.resize(fSize);
        double twoPiOverNBinsMinus1 = KTMath::TwoPi() / (double)(fSize - 1);
        for (unsigned iBin=0; iBin<fSize; iBin++)
        {
            fWindowFunction[iBin] = 0.5 * (1. - cos((double)iBin * twoPiOverNBinsMinus1));
        }
        return;
    }

} /* namespace Katydid */
