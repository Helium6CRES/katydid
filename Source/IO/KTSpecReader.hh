/*
 * KTEggReader.hh
 *
 *  Created on: Aug 20, 2012
 *      Author: nsoblath
 */

#ifndef KTSPECREADER_HH_
#define KTSPECREADER_HH_

#include "KTData.hh"

#include "factory.hh"
#include "path.hh"

#include <string>

namespace Katydid
{

    class KTSpecProcessor;

    class KTSpecReader
    {
        public:
            typedef std::vector< scarab::path > path_vec;

        public:
            KTSpecReader();
            virtual ~KTSpecReader();

        public:
            virtual bool Configure(const KTSpecProcessor& specProc) = 0;

    };

} /* namespace Katydid */
#endif /* KTSPECREADER_HH_ */
