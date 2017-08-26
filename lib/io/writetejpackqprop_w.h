
#ifndef __writetejpackqprop_h__
#define __writetejpackqprop_h__

/*! \file
 *  \brief Write out a TejPack propagator
 */

namespace Chroma {


//! Write a raw packed propagator file.
/*!
 * \ingroup io
 *
 * \param q          propagator ( Read )
 * \param file       path ( Read )
 * \param j_decay    time direction ( Read )
 * \param t_slice    time slice ( Read )
 */    

void writeTejPackQpropSlice(const LatticePropagator& q, const std::string& file,
                            int j_decay, int t_slice);

}  // end namespace Chroma

#endif
