// $Id:  inline_shift_origin.cc  wagman 2017  Exp $ 
/*! \file
 *  \brief shifts gauge field origin 
 */

#ifndef __inline_shift_origin_h__
#define __inline_shift_origin_h__

#include "chromabase.h"
#include "meas/inline/abs_inline_measurement.h"

namespace Chroma 
{ 
  /*! \ingroup inlinehadron */
  namespace InlineShiftOriginEnv 
  {
    extern const std::string name;
    bool registerAll();
  }

  //! Parameter structure
  /*! \ingroup inlinehadron */
  struct InlineShiftOriginParams 
  {
    InlineShiftOriginParams();
    InlineShiftOriginParams(XMLReader& xml_in, const std::string& path);
    void write(XMLWriter& xml_out, const std::string& path);

    unsigned long frequency;

    struct Param_t
    {
       multi1d<int> t_srce;
     } param;

    struct NamedObject_t
    {
      std::string  gauge_id;           /*!< Input gauge field */
      std::string  gauge_shifted_id;           
    } named_obj;

    std::string xml_file;  // Alternate XML file pattern
  };


  //! Inline visualization
  /*! \ingroup inlinehadron */
  class InlineShiftOrigin : public AbsInlineMeasurement 
  {
  public:
    ~InlineShiftOrigin() {}
    InlineShiftOrigin(const InlineShiftOriginParams& p) : params(p) {}
    InlineShiftOrigin(const InlineShiftOrigin& p) : params(p.params) {}

    unsigned long getFrequency(void) const {return params.frequency;}

    //! Do the measurement
    void operator()(const unsigned long update_no,
		    XMLWriter& xml_out); 

  protected:
    //! Do the measurement
    void func(const unsigned long update_no,
	      XMLWriter& xml_out); 

  private:
    InlineShiftOriginParams params;
  };

};

#endif
