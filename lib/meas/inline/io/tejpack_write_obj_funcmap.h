// -*- C++ -*-
/*! \file
 *  \brief Write object function std::map
 */

#ifndef __tejpack_write_obj_funcmap_h__
#define __tejpack_write_obj_funcmap_h__

#include "singleton.h"
#include "funcmap.h"
#include "chromabase.h"

namespace Chroma
{

  //! Write object function std::map
  /*! \ingroup inlineio */
  namespace TejPackWriteObjCallMapEnv
  { 
    struct DumbDisambiguator {};

    //! Write object function std::map
    /*! \ingroup inlineio */
    typedef SingletonHolder< 
      FunctionMap<DumbDisambiguator,
		  void,
		  std::string,
		  TYPELIST_4(const std::string&,
			     const std::string&, 
			     int, int),
		  void (*)(const std::string& buffer_id,
			   const std::string& filename, 
			   int j_decay, int t_slice),
		  StringFunctionMapError> >
    TheTejPackWriteObjFuncMap;

    bool registerAll();
  }

} // end namespace Chroma


#endif
