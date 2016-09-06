/*
 * KTLinearDensityProbeFit.cc
 *
 *  Created on: Nov 13, 2015
 *      Author: ezayas
 */

#include "KTLinearDensityProbeFit.hh"
#include "KTDiscriminatedPoints2DData.hh"
#include "KTDiscriminatedPoints1DData.hh"
#include "KTProcessedTrackData.hh"
#include "KTLinearFitResult.hh"
#include "KTSpectrumCollectionData.hh"
#include "KTLogger.hh"
#include "KTTimeSeries.hh"
#include "KTTimeSeriesReal.hh"
#include "KTTimeSeriesData.hh"
#include "KTEggHeader.hh"

#include "KTParam.hh"

#include <cmath>
#include <vector>
#include <algorithm>

#ifdef USE_OPENMP
#include <omp.h>
#endif

using std::string;
using std::vector;


namespace Katydid
{
    KTLOGGER(sdlog, "KTLinearDensityProbeFit");

    KT_REGISTER_PROCESSOR(KTLinearDensityProbeFit, "linear-density-fit");

    KTLinearDensityProbeFit::KTLinearDensityProbeFit(const std::string& name) :
            KTProcessor(name),
            fMinFrequency(0.),
            fMaxFrequency(1.),
            fProbeWidthBig(1e6),
            fProbeWidthSmall(0.02e6),
            fStepSizeBig(0.2e6),
            fStepSizeSmall(0.004e6),
            fLinearDensityFitSignal("fit-result", this),
            fThreshPointsSlot("thresh-points", this, &KTLinearDensityProbeFit::Calculate, &fLinearDensityFitSignal),
            fPreCalcSlot("gv", this, &KTLinearDensityProbeFit::SetPreCalcGainVar)
    {
    }

    KTLinearDensityProbeFit::~KTLinearDensityProbeFit()
    {
    }

    bool KTLinearDensityProbeFit::Configure(const KTParamNode* node)
    {
        if (node == NULL) return false;

        if (node->Has("min-frequency"))
        {
            SetMinFrequency(node->GetValue< double >("min-frequency"));
        }
        if (node->Has("max-frequency"))
        {
            SetMaxFrequency(node->GetValue< double >("max-frequency"));
        }
        if (node->Has("probe-width-big"))
        {
            SetProbeWidthBig(node->GetValue< double >("probe-width-big"));
            SetStepSizeBig(node->GetValue< double >("probe-width-big") / 5);
        }
        if (node->Has("probe-width-small"))
        {
            SetProbeWidthSmall(node->GetValue< double >("probe-width-small"));
            SetStepSizeSmall(node->GetValue< double >("probe-width-small") / 5);
        }

        SetStepSizeBig(node->GetValue< double >("step-size-big", fStepSizeBig));
        SetStepSizeSmall(node->GetValue< double >("step-size-small", fStepSizeSmall));

        return true;
    }

    double Gaus_Eval( double arg, double sigma )
    {
        return exp( -1*pow(arg/sigma, 2)/2 );
    }

    double Significance( vector<double> x, vector<int> omit, int include, std::string metric )
    {
        double noiseAmp = 0;
        double noiseDev = 0;

        int s = x.size();
        for( int i = 0; i < s; i++ )
        {
            if( find( omit.begin(), omit.end(), i ) == omit.end() )
            {
                noiseAmp += x[i] / s;
                noiseDev += pow( x[i], 2 ) / s;
            }
        }
        noiseDev = sqrt( noiseDev - pow( noiseAmp, 2 ) );
        if( metric == "Sigma" )
            return (x[include] - noiseAmp) / noiseDev;
        else if( metric == "SNR" )
            return x[include] / noiseAmp;
        else
            return -1.;
    }

    double KTLinearDensityProbeFit::findIntercept( KTDiscriminatedPoints2DData& pts, double dalpha, double q, double width )
    {
        double alpha = fMinFrequency;
        double bestAlpha = 0, bestError = 0, error = 0;
        while( alpha <= fMaxFrequency )
        {
            error = 0;

            // Calculate the associated error to the current value of alpha
            for( KTDiscriminatedPoints2DData::SetOfPoints::const_iterator it = pts.GetSetOfPoints(0).begin(); it != pts.GetSetOfPoints(0).end(); ++it )
            {
                error -= Gaus_Eval( it->second.fOrdinate - q * it->second.fAbscissa - alpha, width );
            }
            
            if( error < bestError || bestError == 0 )
            {
                bestError = error;
                bestAlpha = alpha;
            }
            
            // Increment alpha
            alpha += dalpha;
        }

        return bestAlpha;
    }

    bool KTLinearDensityProbeFit::SetPreCalcGainVar(KTGainVariationData& gvData)
    {
        fGVData = gvData;
        return true;
    }

    bool KTLinearDensityProbeFit::Calculate(KTProcessedTrackData& data, KTDiscriminatedPoints2DData& pts)
    {
        KTLinearFitResult& newData = data.Of< KTLinearFitResult >();
        KTPSCollectionData& fullSpectrogram = data.Of< KTPSCollectionData >();

        newData.SetNComponents( 2 );
        newData.SetSlope( data.GetSlope(), 0 );
        newData.SetSlope( data.GetSlope(), 1 );
        newData.SetTrackDuration( data.GetEndTimeInRunC() - data.GetStartTimeInRunC(), 0 );
        newData.SetTrackDuration( data.GetEndTimeInRunC() - data.GetStartTimeInRunC(), 1 );

        double intercept1, intercept2;

        PerformTest( pts, newData, fProbeWidthBig, fStepSizeBig, 0 );
        PerformTest( pts, newData, fProbeWidthSmall, fStepSizeSmall, 1 );

        KTDEBUG(sdlog, "Beginning fourier analysis of sideband" );

        intercept1 = newData.GetIntercept( 0 );
        intercept2 = newData.GetIntercept( 1 );

        newData.SetSidebandSeparation( intercept1 - intercept2, 0 );
        newData.SetSidebandSeparation( intercept1 - intercept2, 1 );

        // We will need to calculate the unweighted projection first
        double delta_f;
        double alphaBound_upper = intercept1 + 1e6;
        double alphaBound_lower = intercept1 - 1e6;

        int xBinStart, xBinEnd, xWindow, yBinStart, yBinEnd, yWindow;
        double ps_xmin, ps_xmax, ps_ymin, ps_dx, ps_dy;
        double q_fit = newData.GetSlope( 0 );
        double x, y;
        
        ps_xmin = fullSpectrogram.GetStartTime();
        ps_xmax = fullSpectrogram.GetEndTime();
        ps_ymin = fullSpectrogram.GetSpectra().begin()->second->GetRangeMin();
        ps_dx   = fullSpectrogram.GetDeltaT();
        ps_dy   = fullSpectrogram.GetSpectra().begin()->second->GetFrequencyBinWidth();
        
        // We add +1 for the underflow bin
        xBinStart = floor( (data.GetStartTimeInRunC() - ps_xmin) / ps_dx ) + 1;
        xBinEnd   = floor( (data.GetEndTimeInRunC() - ps_xmin) / ps_dx ) + 1;
        xWindow = xBinEnd - xBinStart + 1;
        KTINFO(sdlog, "Set xBin range to " << xBinStart << ", " << xBinEnd);

        newData.SetFit_width( xWindow, 0 );
        newData.SetFit_width( xWindow, 1 );

        // The y window this time will be floating, but its size will be consistent
        // The number of bins between the alpha bounds
        yWindow = ceil( (alphaBound_upper - alphaBound_lower) / ps_dy );

        //double *unweighted = new double[xWindow];
        vector< double > unweighted, weighted, fourier;

        KTDEBUG(sdlog, "Computing unweighted projection");

        int i = 0;
        // First we compute the unweighted projection
        for( std::map< double, KTPowerSpectrum* >::const_iterator it = fullSpectrogram.GetSpectra().begin(); it != fullSpectrogram.GetSpectra().end(); ++it )
        {
            // Set x value and starting y-bin
            x = ps_xmin + (i - 1) * ps_dx;
            yBinStart = it->second->FindBin( alphaBound_lower + q_fit * x );

            // Unweighted power = sum of raw power spectrum
            unweighted.push_back( 0 );
            for( int j = yBinStart; j < yBinStart + yWindow; j++ )
            {
                y = ps_ymin + ps_dy * (j - 1);

                // We reevaluate the spline rather than deal with the appropriate index of power_minus_bkgd
                unweighted[i] += (*it->second)(j) - fGVData.GetSpline()->Evaluate( y );
            }
            i++;
        }

        KTDEBUG(sdlog, "Computing weighted projection");

        i = 0;

        // Weighted projection
        double cumulative;
        for( std::map< double, KTPowerSpectrum* >::const_iterator it = fullSpectrogram.GetSpectra().begin(); it != fullSpectrogram.GetSpectra().end(); ++it )
        {
            cumulative = 0.;

            x = ps_xmin + (i - 1) * ps_dx;
            yBinStart = it->second->FindBin( alphaBound_lower + q_fit * x );

            for( int j = yBinStart; j < yBinStart + yWindow; j++ )
            {
                y = ps_ymin + ps_dy * (j - 1);

                // Calculate delta-f using the fit values
                delta_f = y - (q_fit * x + newData.GetIntercept(0));
                cumulative += delta_f * ((*it->second)(j) - fGVData.GetSpline()->Evaluate( y )) / unweighted[i];
            }

            weighted.push_back( cumulative );
            i++;
        }

        // Discrete Cosine Transform (real -> real) of type I
        // Explicit, not fast (i.e. n^2 operations)

        double pi = 3.14159265358979;
        for( int k = 0; k < xWindow; k++ )
        {
            cumulative = 0.;
            for( int n = 1; n <= xWindow - 2; n++ )
                cumulative += weighted[n] * cos( n * k * pi / (xWindow - 1) );

            fourier.push_back( pow( 0.5 * (weighted[0] + pow( -1, k ) * weighted[xWindow - 1]) + cumulative, 2 ) );
        }

        // Evaluate SNR and std dev significance of the largest fourier peak

        double avg_fourier = 0.;
        double max_fourier = 0.;
        double freq_step = 1/(2 * (xWindow - 1) * ps_dx);

        for( int k = 0; k < xWindow; k++ )
            avg_fourier += fourier[k] / xWindow;

        for( int k = 0; k < xWindow; k++ )
        {
            if( fourier[k] > max_fourier )
            {
                max_fourier = fourier[k];
                newData.SetFFT_peak( double(k) * freq_step, 0 );
                newData.SetFFT_SNR( max_fourier / avg_fourier, 0 );
                newData.SetFFT_peak( double(k) * freq_step, 1 );
                newData.SetFFT_SNR( max_fourier / avg_fourier, 1 );
            }
        }

        KTINFO(sdlog, "Successfully obtained power modulation. Maximum fourier peak at frequency " << newData.GetFFT_peak( 0 ) << " with SNR " << newData.GetFFT_SNR( 0 ));

        return true;
    }

    bool KTLinearDensityProbeFit::PerformTest(KTDiscriminatedPoints2DData& pts, KTLinearFitResult& newData, double fProbeWidth, double fStepSize, unsigned component)
    {
        double alpha = fMinFrequency;
        double bestAlpha = 0, bestError = 0, error = 0;
        
        KTINFO(sdlog, "Performing density probe test with fProbeWidth = " << fProbeWidth << " and fStepSize = " << fStepSize);
        bestAlpha = findIntercept( pts, fStepSize, newData.GetSlope( component ), fProbeWidth );
        newData.SetIntercept( bestAlpha, component );

        return true;
    }
    

} /* namespace Katydid */