#include "actions/ferm/qprop/avp_ssed_solver.h"
    
#include <dwf-ssed.h>


using namespace QDP;
namespace Chroma 
{ 
  namespace AVPSolver 
  { 

    MIT_ssed_DWF_Fermion* SSEDWFSolverD::loadFermionRHS(const void* OuterFermion) const {
      return MIT_ssed_DWF_load_fermion(OuterFermion, NULL, &AVPSolverFunctions::fermionReaderRHS);
    }

    MIT_ssed_DWF_Fermion* SSEDWFSolverD::loadFermionGuess(const void *OuterFermion) const {
	return MIT_ssed_DWF_load_fermion(OuterFermion, NULL, &AVPSolverFunctions::fermionReaderGuess);
    }

    MIT_ssed_DWF_Fermion* SSEDWFSolverD::allocateFermion(void) const {
      return MIT_ssed_DWF_allocate_fermion();
    }

    void SSEDWFSolverD::saveFermionSolver(void *OuterFermion, 
					  MIT_ssed_DWF_Fermion* CGFermion) const {
      MIT_ssed_DWF_save_fermion(OuterFermion, NULL, &AVPSolverFunctions::fermionWriterSolver, CGFermion);
    }

    void SSEDWFSolverD::saveFermionOperator(void *OuterFermion, 
					    MIT_ssed_DWF_Fermion* CGFermion) const {
      MIT_ssed_DWF_save_fermion(OuterFermion, NULL, &AVPSolverFunctions::fermionWriterOperator, CGFermion);
    }
    
    void SSEDWFSolverD::deleteFermion(MIT_ssed_DWF_Fermion* ptr) const {
      MIT_ssed_DWF_delete_fermion(ptr);
    }

    
    int SSEDWFSolverD::cgInternal(MIT_ssed_DWF_Fermion       *psi,
				  double        *out_eps,
				  int           *out_iter,
				  double        M,
				  double        m_f,
				  const MIT_ssed_DWF_Fermion *x0,
				  const MIT_ssed_DWF_Fermion *eta,
				  double        eps,
				  int           min_iter,
				  int           max_iter)  const 
    {
      QDPIO::cout << "Entering MIT_ssed_DWF_cg_solver" << std::endl;
      return MIT_ssed_DWF_cg_solver(psi, out_eps, out_iter, g, M, m_f,
				    x0, eta, eps, min_iter, max_iter);
    }
    
    void SSEDWFSolverD::loadGauge(const void *u,
				  const void *v) { 
      g=MIT_ssed_DWF_load_gauge(u, v, NULL, &AVPSolverFunctions::gaugeReader);
    }
     
    void SSEDWFSolverD::deleteGauge(void) {
      MIT_ssed_DWF_delete_gauge(g);
    }

     
    // Init the system -- Constructor call?
    int SSEDWFSolverD::init(const int lattice[5],
			    void *(*allocator)(size_t size),
			    void (*deallocator)(void *)) {
      return MIT_ssed_DWF_init(lattice, allocator, deallocator);
    }
     
    // Finalize - destructor call
    void SSEDWFSolverD::fini(void) {
      MIT_ssed_DWF_fini();
    }
  };

};
