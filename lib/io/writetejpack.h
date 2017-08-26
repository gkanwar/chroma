
/*! \file
 *  \brief Write a TejPack configuration.
 */

#ifndef __writetejpack_h__
#define __writetejpack_h__

namespace Chroma 
{

  //! Write a truncated TejPack configuration file
  /*!
   *   Gauge field layout is (slow to fast)
   *     u(x,y,z,mu,color_row,color_col)
   *   Always written in double precision.
   *
   * \ingroup io
   *
   * \param u          gauge configuration ( Read )
   * \param j_decay    direction which will be slice ( Read )
   * \param t_slice    slice in j_decay direction ( Read )
   * \param cfg_file   path ( Read )
   */    

  void writeTejPackSlice(const multi1d<LatticeColorMatrix>& u, 
                         int j_decay, int t_slice,
                         const std::string& cfg_file);

}  // end namespace Chroma

#endif

