// -*- C++ -*-
//
// $Id:  nucleon_pes_2pt.h  Kanwar Dec 2017  Exp $ 
/*! \file
 *  \brief save baryon correlator under various PES reps
 */

#ifndef __nucleonpes2pt_h__
#define __nucleonpes2pt_h__

#include "chromabase.h"

namespace Chroma {

void nucleon_pes_2pt(const multi1d<LatticeColorMatrix>& u,
		     const multi1d<int>& src_coord,
		     const LatticePropagator& q1,
		     XMLWriter& xml_sym,
		     XMLWriter& xml_m1,
		     XMLWriter& xml_m2,
		     XMLWriter& xml_asym,
		     int j_decay,
		     std::string& base_name);

} // end namespace Chroma  

#endif

 
