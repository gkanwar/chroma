// $Id:  position_2pt.cc  wagman Sept 2017  Exp $ 
/*! \file
 *  \brief adds arbitrary U1B field 
 */

#include "chromabase.h"

namespace Chroma {

void position2pt(const multi1d<LatticeColorMatrix>& u,
    const multi1d<int>& src_coord,
    const LatticePropagator& q1,
    XMLWriter& xml_up,
    XMLWriter& xml_down,
    XMLWriter& xml_negpar_up,
    XMLWriter& xml_negpar_down,
    std::string& base_name)
{

  if ( Ns!= 4 || Nc != 3){ //Code specific to Ns=4 and Nc=3
    QDPIO::cerr<<"Baryon code specific to Nc=3 and Ns=4\n";
    QDP_abort(111);
  }
  #if QDP_NC == 3
  QDPIO::cerr<<"Into position 2pt successfully\n";

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

  // GAMMA MATRICES
  // Define spin matrices. Identity
  SpinMatrixD g_one = 1.0;
  SpinMatrixD g5 = Gamma(15)*g_one;
  SpinMatrixD g_x = Gamma(1)*g_one;
  SpinMatrixD g_my = Gamma(2)*g_one;
  SpinMatrixD g_z = Gamma(4)*g_one;
  multi1d<SpinMatrixD> g_vec(3) ;
  // Positive parity spin up projector (note Sz is backwards of a reasonable Euclidean basis)
  SpinMatrixD S_proj_up = 0.5*((g_one + Gamma(8)*g_one) - timesI(Gamma(3)*g_one + Gamma(11)*g_one));
  // Positive parity spin down projector
  SpinMatrixD S_proj_down = 0.5*((g_one + Gamma(8)*g_one) + timesI(Gamma(3)*g_one + Gamma(11)*g_one));
  // For unpolarized, spin-averaged baryon correlator
  SpinMatrixD S_proj_unpol = 0.5*((g_one + Gamma(8)*g_one));
  // Negative parity
  SpinMatrixD SN_proj_up = 0.5*((g_one - Gamma(8)*g_one) - timesI(Gamma(3)*g_one - Gamma(11)*g_one));
  SpinMatrixD SN_proj_down = 0.5*((g_one - Gamma(8)*g_one) + timesI(Gamma(3)*g_one - Gamma(11)*g_one));
  SpinMatrixD SN_proj_unpol = 0.5*((g_one - Gamma(8)*g_one));
  // Charge conjugation C = g2 g4 (DeGrand Rossi - this is really minus C for Weyl basis!)
  SpinMatrixD c = g_one*Gamma(10);
  // Cgamma_5 for diquark construction
  SpinMatrixD cg5 = g_one*Gamma(5);

  QDPIO::cout<<"Starting Correlation Functions" << std::endl;

  // Shift origin of propagator to source point
  LatticePropagator q1_cpy = q1;
  LatticePropagator q1_tmp;
  for(int mu=0; mu<Nd; ++mu)
  {
    for(int steps=src_coord[mu]; steps>0; --steps)
      {
        q1_tmp = shift(q1_cpy,FORWARD,mu);
        q1_cpy = q1_tmp;
      }
  }


  // PION CORRELATION FUNCTIONS
  LatticePropagator adj_q1_cpy = adj(q1_cpy);
  //LatticePropagator adj_q2 = adj(q2);
  LatticeComplex pion_den = trace(q1_cpy * adj_q1_cpy);
  //LatticeComplex kaon_den = trace(q1 * adj_q2);

  // A0 CORELATION FUNCTIONS: negative-definite for free particle
  QDPIO::cout<<"Onto A0" << std::endl;
  // Antiparticle correlator
  LatticePropagator q1_cpy_anti = g5 * adj(q1_cpy) * g5;
  // Rho spin states
  LatticeComplex a0_den = trace( q1_cpy_anti * q1_cpy_anti );

  // BARYON CORRElATION FUNCTIONS
  QDPIO::cout<<"Onto Nucleon" << std::endl;
  // Interpolators are standard Dirac upper components operator contributing in NR quark model, 2--Dirac lower components
  LatticePropagator diq11 = quarkContract13(cg5 * q1_cpy, q1_cpy * cg5);
  LatticeComplex nucleon_up_den = trace(S_proj_up * q1_cpy * diq11) + trace(S_proj_up * q1_cpy * traceSpin(diq11));
  LatticeComplex nucleon_down_den = trace(S_proj_down * q1_cpy * diq11) + trace(S_proj_down * q1_cpy * traceSpin(diq11));
  LatticeComplex negpar_nucleon_up_den = trace(SN_proj_up * q1 * diq11) + trace(SN_proj_up * q1 * traceSpin(diq11));
  LatticeComplex negpar_nucleon_down_den = trace(SN_proj_down * q1 * diq11) + trace(SN_proj_down * q1 * traceSpin(diq11));

  // xml nucleon_up zero momentum corr
  XMLArrayWriter xml_upcorr(xml_up);
  push(xml_upcorr,"PositionCorrs");
  push(xml_upcorr,"elem");
  write(xml_upcorr, "Hadron", base_name + "_NUCLEON_UP");
  write(xml_upcorr, "Coords", src_coord);
  write(xml_upcorr, "Corr", nucleon_up_den);
  pop(xml_upcorr); // close elem
  pop(xml_upcorr); // close Position2pt
  // xml nucleon_down zero momentum corr
  XMLArrayWriter xml_downcorr(xml_down);
  push(xml_downcorr,"PositionCorrs");
  push(xml_downcorr,"elem");
  write(xml_downcorr, "Hadron", base_name + "_NUCLEON_DOWN");
  write(xml_downcorr, "Coords", src_coord);
  write(xml_downcorr, "Corr", nucleon_down_den);
  pop(xml_downcorr); // close elem
  pop(xml_downcorr); // close Position2pt
  // xml nucleon_up zero momentum corr
  XMLArrayWriter xml_negparupcorr(xml_negpar_up);
  push(xml_negparupcorr,"PositionCorrs");
  push(xml_negparupcorr,"elem");
  write(xml_negparupcorr, "Hadron", base_name + "NEGPAR_NUCLEON_UP");
  write(xml_negparupcorr, "Coords", src_coord);
  write(xml_negparupcorr, "Corr", negpar_nucleon_up_den);
  pop(xml_negparupcorr); // close elem
  pop(xml_negparupcorr); // close Position2pt
  // xml nucleon_down zero momentum corr
  XMLArrayWriter xml_negpardowncorr(xml_negpar_down);
  push(xml_negpardowncorr,"PositionCorrs");
  push(xml_negpardowncorr,"elem");
  write(xml_negpardowncorr, "Hadron", base_name + "NEGPAR_NUCLEON_DOWN");
  write(xml_negpardowncorr, "Coords", src_coord);
  write(xml_negpardowncorr, "Corr", negpar_nucleon_down_den);
  pop(xml_negpardowncorr); // close elem
  pop(xml_negpardowncorr); // close Position2pt


  #endif

}
} //end namespace chroma
