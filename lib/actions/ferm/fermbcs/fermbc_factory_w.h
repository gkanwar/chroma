// -*- C++ -*-
// $Id: fermbc_factory_w.h,v 1.1 2004-12-24 04:23:20 edwards Exp $
/*! \file
 *  \brief Fermion Boundary Condition factories
 */

#ifndef __fermbc_factory_w_h__
#define __fermbc_factory_w_h__

#include "singleton.h"
#include "objfactory.h"

#include "fermbc.h"


namespace Chroma
{
  //! FermBC factory
  typedef SingletonHolder< 
    ObjectFactory<FermBC<LatticeFermion>, 
		  std::string,
		  TYPELIST_2(XMLReader&, const std::string&),
		  FermBC<LatticeFermion>* (*)(XMLReader&, const std::string&), 
		  StringFactoryError> >
  TheWilsonTypeFermBCFactory;


  //! FermBC array factory
  typedef SingletonHolder< 
    ObjectFactory<FermBC< multi1d<LatticeFermion> >, 
		  std::string,
		  TYPELIST_2(XMLReader&, const std::string&),
		  FermBC< multi1d<LatticeFermion> >* (*)(XMLReader&, const std::string&), 
		  StringFactoryError> >
  TheWilsonTypeFermBCArrayFactory;

}


#endif
