// $Id:  nucleon_pes_2pt.cc  Kanwar Dec 2017  Exp $ 
/*! \file
 *  \brief baryon correlator under various PES reps
 */

#include "chromabase.h"
#include "util/ft/sftmom.h"

namespace Chroma {
void nucleon_pes_2pt(const multi1d<LatticeColorMatrix>& u,
		     const multi1d<int>& src_coord,
		     const LatticePropagator& q1,
		     XMLWriter& xml_sym,
		     XMLWriter& xml_m1,
		     XMLWriter& xml_m2,
		     XMLWriter& xml_asym,
		     int j_decay,
		     std::string& base_name) {
  if (Ns != 4 || Nc != 3) {
    QDPIO::cerr << "Nucleon PES code needs Ns=4, Nc=3" << std::endl;
    QDP_abort(123);
  }
  #if QDP_NC == 3
  QDPIO::cerr << "Into nucleon PES 2pt" << std::endl;

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

  QDPIO::cout << "Starting Correlation Functions" << std::endl;

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

  // BARYON CORRElATION FUNCTIONS
  QDPIO::cout << "Onto Nucleon" << std::endl;
  // Interpolators are standard Dirac upper components operator contributing in NR quark model, 2--Dirac lower components
  LatticePropagator diq11 = quarkContract13(cg5 * q1_cpy, q1_cpy * cg5);
  LatticeComplex nucleon_sym =
    trace(S_proj_up * q1_cpy * diq11) +
    trace(S_proj_up * q1_cpy * traceSpin(diq11));
  LatticeComplex nucleon_m1 =
    trace(S_proj_down * q1_cpy * diq11) +
    trace(S_proj_down * q1_cpy * traceSpin(diq11));
  LatticeComplex nucleon_m2 =
    trace(SN_proj_up * q1 * diq11) +
    trace(SN_proj_up * q1 * traceSpin(diq11));
  LatticeComplex nucleon_asym =
    trace(SN_proj_down * q1 * diq11) +
    trace(SN_proj_down * q1 * traceSpin(diq11));


  /// Zero-momentum projection
  SftMom phases(0, true, j_decay);
  multi1d<DComplex> momproj_nucleon_sym(phases.numSubsets());
  multi1d<DComplex> momproj_nucleon_m1(phases.numSubsets());
  multi1d<DComplex> momproj_nucleon_m2(phases.numSubsets());
  multi1d<DComplex> momproj_nucleon_asym(phases.numSubsets());
  momproj_nucleon_sym = sumMulti(nucleon_sym, phases.getSet());
  momproj_nucleon_m1 = sumMulti(nucleon_m1, phases.getSet());
  momproj_nucleon_m2 = sumMulti(nucleon_m2, phases.getSet());
  momproj_nucleon_asym = sumMulti(nucleon_asym, phases.getSet());

  write(xml_sym, "momproj_corr", momproj_nucleon_sym);
  write(xml_m1, "momproj_corr", momproj_nucleon_m1);
  write(xml_m2, "momproj_corr", momproj_nucleon_m2);
  write(xml_asym, "momproj_corr", momproj_nucleon_asym);

  #endif // QDP_NC == 3
}


}
