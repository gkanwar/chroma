// $Id:  shift_origin.h  wagman 2017  Exp $ 
/*! \file
 *  \brief shifts gauge field origin 
 */


#ifndef __ShiftOrigin_h__
#define __ShiftOrigin_h__

#include "chromabase.h"

namespace Chroma {


void ShiftOrigin(multi1d<LatticeColorMatrix>& u,
    multi1d<LatticeColorMatrix>& u_shifted,
    const multi1d<int>& t_srce,
    XMLWriter& xml);

} // end namespace Chroma  

#endif

