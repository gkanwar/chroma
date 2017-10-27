// $Id:  shift_origin.cc  wagman 2017  Exp $ 
/*! \file
 *  \brief shifts gauge field origin 
 */

#include "chromabase.h"

namespace Chroma {

void ShiftOrigin(multi1d<LatticeColorMatrix>& u,
    multi1d<LatticeColorMatrix>& u_shifted,
    const multi1d<int>& t_srce,
    XMLWriter& xml)
{

  QDPIO::cerr<<"Into Shift Origin successfully\n";

  // Get the lattice size
  const multi1d<int>& latt_size = Layout::lattSize();
  int Nx = latt_size[0];
  int Ny = latt_size[1];
  int Nz = latt_size[2];
  int Nt = latt_size[3];
  int Nsites = Nt*Nx*Ny*Nz;

  // Stores current position vector
  multi1d<int> npos(4) ;
  npos[0] = 0;
  npos[1] = 0;
  npos[2] = 0;
  npos[3] = 0;  // time


  // Write transform info
  XMLArrayWriter xml_U1B(xml);
  push(xml_U1B, "TransformInfo");
  push(xml_U1B,"elem");
  write(xml_U1B, "t_src", t_srce);
  pop(xml_U1B);
  pop(xml_U1B);

  // Shift origin of gauge field to source point
  u_shifted = u;
  LatticeColorMatrix u_tmp;

  QDPIO::cerr<<"Starting shift\n";
  for(int mu=0; mu<Nd; ++mu)
  {
    for(int nu=0; nu<Nd; ++nu)
    {
      for(int steps=t_srce[nu]; steps>0; --steps)
      {
        u_tmp = shift(u_shifted[mu],FORWARD,nu);
        u_shifted[mu] = u_tmp;
      }
    }
  }

  // Print info
  QDPIO::cout << "U4(t_src) is " << peekSite(peekColor(u[3],0,0), t_srce) << "\n";
  QDPIO::cout << "Ushifted4(0) is " << peekSite(peekColor(u_shifted[3],0,0), npos) << "\n";



}
} //end namespace chroma
