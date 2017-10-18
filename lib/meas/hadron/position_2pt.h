// -*- C++ -*-
//
// $Id:  position_2pt.h  Wagman Sept 2017  Exp $ 
/*! \file
 *  \brief save position-space baryon correlator
 */

#ifndef __position2pt_h__
#define __position2pt_h__

#include "chromabase.h"

namespace Chroma {

void position2pt(const multi1d<LatticeColorMatrix>& u,
    const multi1d<int>& src_coord,
    const LatticePropagator& q1,
    XMLWriter& xml_up,
    XMLWriter& xml_down,
    std::string& base_name);

} // end namespace Chroma  

#endif

 
