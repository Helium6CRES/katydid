/*
 * KTHammingWindow.cc
 *
 *  Created on: Sep 18, 2011
 *      Author: nsoblath
 */

#include "KTHammingWindow.hh"

#include "KTNOFactory.hh"
#include "KTLogger.hh"
#include "KTMath.hh"
#include "KTParam.hh"

#include <cmath>

using std::string;

namespace Katydid
{
    KTLOGGER(windowlog, "KTHammingWindow");

    static KTNORegistrar< KTWindowFunction, KTHammingWindow > sWFHammRegistrar("hamming");

    KTHammingWindow::KTHammingWindow(const string& name) :
            KTWindowFunction(name)
    {
    }

    KTHammingWindow::~KTHammingWindow()
    {
    }

    bool KTHammingWindow::ConfigureWFSubclass(const KTParamNode*)
    {
        KTDEBUG(windowlog, "Hamming WF configured");
        return true;
    }

    double KTHammingWindow::GetWeight(double time) const
    {
        return GetWeight(KTMath::Nint(time / fBinWidth));
    }

    void KTHammingWindow::RebuildWindowFunction()
    {
        fWindowFunction.resize(fSize);
        double twoPiOverNBinsMinus1 = KTMath::TwoPi() / (double)(fSize - 1);
        for (unsigned iBin=0; iBin<fSize; iBin++)
        {
            fWindowFunction[iBin] = 0.54 - 0.46 * cos((double)iBin * twoPiOverNBinsMinus1);
        }
        return;
    }

} /* namespace Katydid */
