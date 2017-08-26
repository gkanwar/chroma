/*! \file
 *  \brief Write out a configuration in TejPack format sliced.
 */

#include "chromabase.h"
#include "chroma_config.h"
#include "io/writetejpack.h"
#include "qdp_util.h"

#include <string>
using std::string;

namespace Chroma {

//! Write a sliced TejPack configuration file
/*!
 * \ingroup io
 *
 * \param u          gauge configuration ( Read )
 * \param j_decay    direction which will be sliced ( Read )
 * \param t_slice    slice in j_decay direction ( Read )
 * \param cfg_file   path ( Read )
 */

void writeTejPackSlice(const multi1d<LatticeColorMatrix>& u,
                       int j_decay, int t_slice,
                       const std::string& cfg_file)
{
  START_CODE();

  // Fetch lattice size config
  multi1d<int> nrow = Layout::lattSize();
  
  // The object where data is written
  BinaryFileWriter cfg_out(cfg_file);
  if (j_decay < 0 || j_decay >= Nd)
    QDP_error_exit("writeTejPackSlice: invalid direction for slice, j_decay=%d", j_decay);
  if (t_slice < 0 || t_slice >= nrow[j_decay])
    QDP_error_exit("writeTejPackSlice: invalid t_slice=%d", t_slice);
  if (Nd != 4)
    QDP_error_exit("writeTejPackSlice: invalid Nd=%d", Nd);

  int vol_space = 1;
  multi1d<int> coord(Nd);
  for (int j = 0; j < Nd; ++j) {
    if (j == j_decay) {
      coord[j] = t_slice;
    }
    else {
      vol_space *= nrow[j];
      coord[j] = 0;
    }
  }

  for (int i = 0; i < vol_space; ++i)  {
    if (i != 0) {
      // Increment coord with carry -- farthest right runs fastest
      int d = Nd-1;
      while (true) {
        if (d == j_decay) d--;
        if (d < 0)
          QDP_error_exit("writeTejPackSlice: increment with carry broke!");

        coord[d]++;
        if (coord[d] == nrow[d]) {
          coord[d] = 0;
          d--;
        }
        else {
          break;
        }
      }
    }
    
    // Put time direction first, to match the convention of my code
    write(cfg_out, u[j_decay], coord);
    for (int j = 0; j < Nd; ++j) {
      if (j == j_decay) continue;
      write(cfg_out, u[j], coord);   // Write out an SU(3) matrix
    }
  }

  cfg_out.close();

  END_CODE();
}

}
