// $Id: prec_clover_fermact_w.cc,v 1.9 2004-12-24 04:23:20 edwards Exp $
/*! \file
 *  \brief Even-odd preconditioned Clover fermion action
 */

#include "chromabase.h"
#include "actions/ferm/linop/prec_clover_linop_w.h"
#include "actions/ferm/fermacts/prec_clover_fermact_w.h"
#include "actions/ferm/linop/lmdagm.h"


#include "actions/ferm/fermacts/fermact_factory_w.h"
#include "actions/ferm/fermbcs/fermbcs_w.h"

namespace Chroma
{

  //! Hooks to register the class with the fermact factory
  namespace EvenOddPrecCloverFermActEnv
  {
    //! Callback function
    WilsonTypeFermAct<LatticeFermion>* createFermAct(XMLReader& xml_in,
						     const std::string& path)
    {
      return new EvenOddPrecCloverFermAct(WilsonTypeFermBCEnv::reader(xml_in, path), 
					  EvenOddPrecCloverFermActParams(xml_in, path));
    }

    //! Name to be used
    const std::string name = "UNPRECONDITIONED_CLOVER";

    //! Register the Clover fermact
    const bool registered = TheWilsonTypeFermActFactory::Instance().registerObject(name, createFermAct);
  }


  //! Read parameters
  EvenOddPrecCloverFermActParams::EvenOddPrecCloverFermActParams(XMLReader& xml, const string& path)
  {
    XMLReader paramtop(xml, path);

    // Read the stuff for the action
    if (paramtop.count("Mass") != 0) 
    {
      read(paramtop, "Mass", Mass);
      if (paramtop.count("Kappa") != 0) 
      {
	QDPIO::cerr << "Error: found both a Kappa and a Mass tag" << endl;
	QDP_abort(1);
      }
    }
    else if (paramtop.count("Kappa") != 0)
    {
      Real Kappa;
      read(paramtop, "Kappa", Kappa);
      Mass = kappaToMass(Kappa);    // Convert Kappa to Mass
    }
    else
    {
      QDPIO::cerr << "Error: neither Mass or Kappa found" << endl;
      QDP_abort(1);
    }

    read(paramtop, "ClovCoeff", ClovCoeff);
    read(paramtop, "u0", u0);
  }

  //! Read parameters
  void read(XMLReader& xml, const string& path, EvenOddPrecCloverFermActParams& param)
  {
    EvenOddPrecCloverFermActParams tmp(xml, path);
    param = tmp;
  }



  //! Produce a linear operator for this action
  /*!
   * The operator acts on the odd subset
   *
   * \param state	    gauge field     	       (Read)
   */
  const EvenOddPrecLinearOperator< LatticeFermion, multi1d<LatticeColorMatrix> >* 
  EvenOddPrecCloverFermAct::linOp(Handle<const ConnectState> state) const
  {
    return new EvenOddPrecCloverLinOp(state->getLinks(),Mass,ClovCoeff,u0);
  }

  //! Produce a M^dag.M linear operator for this action
  /*!
   * The operator acts on the odd subset
   *
   * \param state	    gauge field     	       (Read)
   */
  const LinearOperator<LatticeFermion>* 
  EvenOddPrecCloverFermAct::lMdagM(Handle<const ConnectState> state) const
  {
    return new lmdagm<LatticeFermion>(linOp(state));
  }
  
};

#endif
