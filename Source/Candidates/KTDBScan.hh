/**
 @file KTDBScan.hh
 @brief Contains KTDBScan
 @details DBScan Clustering Algorithm
 @author: N.S. Oblath
 @date: Jun 24, 2014
 */

#ifndef KTDBSCAN_HH_
#define KTDBSCAN_HH_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <vector>


namespace Katydid
{
    //
    // Euclidean distance
    template < typename VEC_T >
    class Euclidean
    {
        protected:
            typedef VEC_T vector_type;

            // this must be not directly accessible
            // since we want to provide a rich set of distances

            double GetDistance(const VEC_T v1, const VEC_T v2)
            {
                return norm_2(v1-v2);
            };

            double GetDistance(const VEC_T v1, const VEC_T v2, const VEC_T w)
            {
                return norm_2(outer_prod(w,  (v1-v2)));
            };
    };

    /*
    template <typename VEC_T>
    class Cosine
    {
        protected:
            typedef VEC_T vector_type;

            // this must be not directly accessible
            // since we want to provide a rich set of distances

            double GetDistance(const VEC_T v1, const VEC_T v2)
            {
                //std::cout << "dot=" << prec_inner_prod(v1, v2) << " norm_2=" << norm_2(v1) << "norm2=" << norm_2(v2) << std::endl;
                return prec_inner_prod(v1, v2) / (norm_2(v1) * norm_2(v2));
            };
    };
    */

    template <typename Distance_Policy>   // this allows to provide a static mechanism for pseudo-like
                                          // inheritance, which is optimal from a performance point of view.
    class Distance : Distance_Policy
    {
        public:

            // distance function with equal weighting
            double GetDistance(typename Distance_Policy::vector_type x,
                               typename Distance_Policy::vector_type y)
            {
                return Distance_Policy::GetDistance(x, y);
            };

            // distance function with weighting
            double GetDistance(typename Distance_Policy::vector_type x,
                               typename Distance_Policy::vector_type y,
                               typename Distance_Policy::vector_type w)
            {
                return Distance_Policy::GetDistance(x, y, w);
            };
    };


    /*!
     @class KTDBScan
     @author N.S. Oblath

     @brief DBSCAN Clustering Algorithm

     @details
     This implementation was adapted from:
     http://codingplayground.blogspot.com/2009/11/dbscan-clustering-algorithm.html
     Accessed on 6/5/2014.
     Code was provided without a license.
    */

    class KTDBScan
    {
        public:
            // a single point is made up of vector of doubles
            typedef boost::numeric::ublas::vector< double > Point;
            typedef std::vector< Point > Points;

            typedef boost::numeric::ublas::vector< double > Weights;

            typedef unsigned ClusterId;
            typedef unsigned PointId;

            // a cluster is a vector of pointid
            typedef std::vector< PointId > Cluster;
            // a set of Neighbors is a vector of pointid
            typedef std::vector< PointId > Neighbors;

        public:
            KTDBScan(double radius = 1., unsigned minPoints = 1);
            virtual ~KTDBScan();

            double GetRadius() const;
            void SetRadius(double eps);

            unsigned GetMinPoints() const;
            void SetMinPoints(unsigned pts);

        private:
            // eps radiuus
            // Two points are neighbors if the distance
            // between them does not exceed threshold value.
            double fRadius;

            //minimum number of points
            unsigned fMinPoints;

        public:
            template < typename DistanceType >
            bool RunDBScan(const Points& points);

            template < typename DistanceType >
            bool RunDBScan(const Points& points, const Weights& weights);

            void InitializeArrays(size_t nPoints);

            // assign each point to a new cluster
            void UniformPartition();

            // compute distance
            template < typename DistanceType >
            void ComputeDistance(const Points& points);

            // compute distance
            template < typename DistanceType >
            void ComputeDistance(const Points& points, const Weights& weights);

            bool DoClustering();

            const std::vector< Cluster >& GetClusters() const;

        private:
            //
            // findNeighbors(PointId pid, double threshold)
            //
            // this can be implemented with reduced complexity by using R+trees
            //
            Neighbors FindNeighbors(PointId pid, double threshold);

            unsigned fNPoints;

            // noise-point vector
            std::vector< bool > fNoise;

            // visited-point vector
            std::vector< bool > fVisited;

            // mapping point_id -> clusterId
            std::vector< ClusterId > fPointIdToClusterId;

            // the collection of clusters
            std::vector< Cluster > fClusters;

            // distance matrix
            boost::numeric::ublas::matrix< double > fDist;

            friend std::ostream& operator<<(std::ostream& stream, const KTDBScan& cs);
            friend std::ostream& operator<<(std::ostream& stream, const KTDBScan::Cluster& cluster);
            friend std::ostream& operator<<(std::ostream& stream, const KTDBScan::Point& point);
    };

    std::ostream& operator<<(std::ostream& stream, const KTDBScan& cs);
    std::ostream& operator<<(std::ostream& stream, const KTDBScan::Cluster& cluster);
    std::ostream& operator<<(std::ostream& stream, const KTDBScan::Point& point);


    inline double KTDBScan::GetRadius() const
    {
        return fRadius;
    }
    inline void KTDBScan::SetRadius(double eps)
    {
        fRadius = eps;
        return;
    }

    inline unsigned KTDBScan::GetMinPoints() const
    {
        return fMinPoints;
    }
    inline void KTDBScan::SetMinPoints(unsigned pts)
    {
        fMinPoints = pts;
        return;
    }

    template < typename DistanceType >
    bool KTDBScan::RunDBScan(const Points& points)
    {
        InitializeArrays(points.size());
        ComputeDistance< DistanceType >(points);
        return DoClustering();
    }

    template < typename DistanceType >
    bool KTDBScan::RunDBScan(const Points& points, const Weights& weights)
    {
        InitializeArrays(points.size());
        ComputeDistance< DistanceType >(points, weights);
        return DoClustering();
    }


    template < typename DistanceType >
    void KTDBScan::ComputeDistance(const Points& points)
    {
        // calculate the min and max for each dimension
        unsigned nDims = points[0].size();
        double min, max, range;
        for (unsigned dim = 0; dim < nDims; ++dim)
        {
            min = points[0](dim);
            max = points[0](dim);
            for (PointId pid = 0; pid < points.size(); ++pid)
            {
                if (points[pid](dim) < min) min = points[pid](dim);
                else if (points[pid](dim) > max) max = points[pid](dim);
            }
            range = max - min;
            if (range == 0.) range = 1.;

        }

        Distance< DistanceType > dist;
        for (unsigned i=0; i < fNPoints; ++i)
        {
            for (unsigned j=i+1; j < fNPoints; ++j)
            {
                fDist(j, i) = fDist(i, j) = dist.GetDistance(points[i], points[j]);
                //std::cout << "dist(" << i << ", " << j << ") = dist( " << points[i] << ", " << points[j] << " ) = " << fDist(i, j) << std::endl;
            }
        }
    }

    template < typename DistanceType >
    void KTDBScan::ComputeDistance(const Points& points, const Weights& weights)
    {
        Distance< DistanceType > dist;
        for (unsigned i=0; i < fNPoints; ++i)
        {
            for (unsigned j=i+1; j < fNPoints; ++j)
            {
                fDist(j, i) = fDist(i, j) = dist.GetDistance(points[i], points[j], weights);
                //std::cout << "dist(" << i << ", " << j << ") = dist( " << points[i] << ", " << points[j] << " ) = " << fDist(i, j) << std::endl;
            }
        }
    }

    inline const std::vector< KTDBScan::Cluster >& KTDBScan::GetClusters() const
    {
        return fClusters;
    }

} /* namespace Katydid */
#endif /* KTDBSCAN_HH_ */
