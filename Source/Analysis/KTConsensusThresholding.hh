/**
 @file KTConsensusThresholding.hh
 @brief Contains KTConsensusThresholding
 @details Creates a KD-Tree
 @author: N. S. Oblath
 @date: Aug 7, 2014
 */

#ifndef KTCONSENSUSTHRESHOLDING_HH_
#define KTCONSENSUSTHRESHOLDING_HH_

#include "KTProcessor.hh"

#include "KTKDTreeData.hh"
#include "KTMemberVariable.hh"
#include "KTSlot.hh"


namespace Katydid
{
    class KTKDTreeData;
    class KTParamNode;

   /*!
     @class KTConsensusThresholding
     @author N. S. Oblath

     @brief Filters sparse-waterfall data with the Consensus Thresholding algorithm

     @details
 
     Configuration name: "consensus-thresholding"

     Available configuration values:
     - "membership-radius": double -- Defines the circle in which nearest neighbors are searched for
     - "min-number-votes": unsigned -- Minimum number of votes to keep a point
     - "remove-noise": bool -- Flag that determines whether noise points are removed (true) or flagged (false; default)

     Slots:
     - "kd-tree-in": void (KTDataPtr) -- Performs the CT algorithm on the data in a k-d tree; Requires KTKDTreeData; existing data is modified

     Signals:
     - "kd-tree-out": void (KTDataPtr) emitted upon completion of the CT algorithm
    */

    class KTConsensusThresholding : public KTProcessor
    {
        public:
            KTConsensusThresholding(const std::string& name = "consensus-thresholding");
            virtual ~KTConsensusThresholding();

            bool Configure(const KTParamNode* node);

            MEMBERVARIABLE(double, MembershipRadius);
            MEMBERVARIABLE(unsigned, MinNumberVotes);
            MEMBERVARIABLE(bool, RemovePointsFlag);

        public:
            bool ConsensusVote(KTKDTreeData& kdTreeData);

            bool ConsensusVoteComponent(const KTTreeIndex< double >* kdTree, const std::vector< KTKDTreeData::Point >& setOfPoints, std::vector< size_t >& noisePoints);

        private:
            //void VoteCore(bool doPositive, size_t pid, double* thisPoint, double* neighborPoint, double slope, double intercept);

            //***************
            // Signals
            //***************

        private:
            KTSignalData fKDTreeSignal;

            //***************
            // Slots
            //***************

        private:
            KTSlotDataOneType< KTKDTreeData > fKDTreeSlot;

    };

} /* namespace Katydid */
#endif /* KTCONSENSUSTHRESHOLDING_HH_ */