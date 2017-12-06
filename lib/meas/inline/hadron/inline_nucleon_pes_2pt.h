// -*- C++ -*-
// $Id: inline_nucleon_pes_2pt.h  Kanwar 2017  Exp $
/*! \file
 *  \brief nucleon two-point correlator for various PES reps  */

#ifndef __inline_nucleon_pes_2pt_h__
#define __inline_nucleon_pes_2pt_h__

#include "chromabase.h"
#include "meas/inline/abs_inline_measurement.h"

namespace Chroma 
{ 
  /*! \ingroup inlinehadron */
  namespace InlineNucleonPES2ptEnv 
  {
    extern const std::string name;
    bool registerAll();
  }

  //! Parameter structure
  /*! \ingroup inlinehadron */
  struct InlineNucleonPES2ptParams 
  {
    InlineNucleonPES2ptParams();
    InlineNucleonPES2ptParams(XMLReader& xml_in, const std::string& path);
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

    std::string xml_sym_file;  // Alternate XML file pattern
    std::string xml_m1_file;  // Alternate XML file pattern
    std::string xml_m2_file;  // Alternate XML file pattern
    std::string xml_asym_file;  // Alternate XML file pattern
  };


  //! Inline visualization
  /*! \ingroup inlinehadron */
  class InlineNucleonPES2pt : public AbsInlineMeasurement 
  {
  public:
    ~InlineNucleonPES2pt() {}
    InlineNucleonPES2pt(const InlineNucleonPES2ptParams& p) : params(p) {}
    InlineNucleonPES2pt(const InlineNucleonPES2pt& p) : params(p.params) {}

    unsigned long getFrequency(void) const {return params.frequency;}

    //! Do the measurement
    void operator()(const unsigned long update_no,
		    XMLWriter& xml_out); 

  protected:
    //! Do the measurement
    void func(const unsigned long update_no,
	      XMLWriter& xml_sym_out, XMLWriter& xml_m1_out,
	      XMLWriter& xml_m2_out, XMLWriter& xml_asym_out); 

  private:
    InlineNucleonPES2ptParams params;
  };

};

#endif
