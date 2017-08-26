/*!
 * @file
 * @brief  Write a raw packed quark propagator
 */

#include "chromabase.h"
#include "io/writetejpackqprop_w.h"

#include "qdp_util.h"   // from QDP++

namespace Chroma {

//! Write a SZIN propagator file. This is a simple memory dump writer.
/*!
 * \ingroup io
 *
 * \param q          propagator ( Read )
 * \param file       path ( Read )
 * \param kappa      kappa value (Read)
 */    

void writeTejPackQpropSlice(const LatticePropagator& q,
                         const std::string& file,
                         int j_decay, int t_slice)
{
  BinaryFileWriter cfg_out(file);
  // Fetch lattice size config
  multi1d<int> nrow = Layout::lattSize();

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

    write(cfg_out, q, coord);
  }

  cfg_out.close();
}

}  // end namespace Chroma
