// -*- C++ -*-
// $Id: inline_position_2pt.h  wagman 2017  Exp $
/*! \file
 *  \brief source coordinates sampled uniformly across action and topological charge densities  */

#ifndef __inline_position_2pt_h__
#define __inline_position_2pt_h__

#include "chromabase.h"
#include "meas/inline/abs_inline_measurement.h"

namespace Chroma 
{ 
  /*! \ingroup inlinehadron */
  namespace InlinePosition2ptEnv 
  {
    extern const std::string name;
    bool registerAll();
  }

  //! Parameter structure
  /*! \ingroup inlinehadron */
  struct InlinePosition2ptParams 
  {
    InlinePosition2ptParams();
    InlinePosition2ptParams(XMLReader& xml_in, const std::string& path);
    void write(XMLWriter& xml_out, const std::string& path);

    unsigned long frequency;

    struct Param_t
    {
     } param;

    struct NamedObject_t
    {
      std::string  gauge_id;           /*!< Input gauge field */

      struct Props_t
      {
	std::string  first_id;
        //Nf = 2 only!!
//	std::string  second_id;
      };

      multi1d<Props_t> sink_pairs;
    } named_obj;

    std::string xml_up_file;  // Alternate XML file pattern
    std::string xml_down_file;  // Alternate XML file pattern
    std::string xml_negpar_up_file;  // Alternate XML file pattern
    std::string xml_negpar_down_file;  // Alternate XML file pattern
  };


  //! Inline visualization
  /*! \ingroup inlinehadron */
  class InlinePosition2pt : public AbsInlineMeasurement 
  {
  public:
    ~InlinePosition2pt() {}
    InlinePosition2pt(const InlinePosition2ptParams& p) : params(p) {}
    InlinePosition2pt(const InlinePosition2pt& p) : params(p.params) {}

    unsigned long getFrequency(void) const {return params.frequency;}

    //! Do the measurement
    void operator()(const unsigned long update_no,
		    XMLWriter& xml_out); 

  protected:
    //! Do the measurement
    void func(const unsigned long update_no,
	      XMLWriter& xml_up_out, XMLWriter& xml_down_out, XMLWriter& xml_negpar_up_out, XMLWriter& xml_negpar_down_out); 

  private:
    InlinePosition2ptParams params;
  };

};

#endif
