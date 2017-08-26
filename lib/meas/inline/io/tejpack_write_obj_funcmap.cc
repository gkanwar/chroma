/*! \file
 *  \brief Write object function std::map
 */

#include "named_obj.h"
#include "meas/inline/io/named_objmap.h"
#include "meas/inline/io/tejpack_write_obj_funcmap.h"
#include "io/tejpack_io.h"
#include "io/writetejpack.h"
#include "io/writetejpackqprop_w.h"

namespace Chroma
{
 
  //! IO function std::map environment
  /*! \ingroup inlineio */
  namespace TejPackWriteObjCallMapEnv
  { 
    // Anonymous namespace
    namespace
    {
      //! Write a propagator
      void TejPackWriteLatProp(const std::string& buffer_id,
                               const std::string& file, 
                               int j_decay, int t_slice)
      {
	LatticePropagator obj = TheNamedObjMap::Instance().getData<LatticePropagator>(buffer_id);
    	writeTejPackQpropSlice(obj, file, j_decay, t_slice);
      }

      //! Write a gauge field in floating precision
      void TejPackWriteArrayLatColMat(const std::string& buffer_id,
                                      const std::string& file, 
                                      int j_decay, int t_slice)
      {
	multi1d<LatticeColorMatrix> obj = 
	  TheNamedObjMap::Instance().getData< multi1d<LatticeColorMatrix> >(buffer_id);

	writeTejPackSlice(obj, j_decay, t_slice, file);
      }

      //! Local registration flag
      bool registered = false;

    }  // end namespace



    //! Register all the factories
    bool registerAll() 
    {
      bool success = true; 
      if (! registered)
      {
	success &= TheTejPackWriteObjFuncMap::Instance().registerFunction(std::string("LatticePropagator"), 
                                                                          TejPackWriteLatProp);
	success &= TheTejPackWriteObjFuncMap::Instance().registerFunction(std::string("Multi1dLatticeColorMatrix"), 
                                                                          TejPackWriteArrayLatColMat);

	registered = true;
      }
      return success;
    }
  }

}
