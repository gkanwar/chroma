// -*- C++ -*-
// Wagman 2017
/*! \file
 * \brief Inline measurement of qpropadd
 *
 * Form-factors
 */

#ifndef __inline_U1Badd_w_h__
#define __inline_U1Badd_w_h__

#include "chromabase.h"
#include "meas/inline/abs_inline_measurement.h"

namespace Chroma 
{ 
  /*! \ingroup inlinehadron */
  namespace InlineU1BAddEnv 
  {
    extern const std::string name;
    bool registerAll();
  }

    //! Parameter structure
    /*! \ingroup inlinehadron */
    struct InlineU1BAddParams 
    {
      InlineU1BAddParams();
      InlineU1BAddParams(XMLReader& xml_in, const std::string& path);
      void write(XMLWriter& xml_out, const std::string& path);

      unsigned long      frequency;
      struct Param_t
      {
        Real theta;
        int j_decay;
        multi1d<int> t_srce;
      } param;

      struct NamedObject_t
      {
	std::string      prop_1;    
	std::string      prop_2;   
	std::string      prop_f; 
	std::string      prop_b; 
      } named_obj;

      std::string xml_file;  // Alternate XML file pattern
    };


    //! Inline measurement of to add two props
    /*! \ingroup inlinehadron */
  class InlineU1BAdd : public AbsInlineMeasurement 
  {
  public:
    ~InlineU1BAdd() {}
    InlineU1BAdd(const InlineU1BAddParams& p) : params(p) {}
    InlineU1BAdd(const InlineU1BAdd& p) : params(p.params) {}

      unsigned long getFrequency(void) const {return params.frequency;}

      //! Do the measurement
      void operator()(const unsigned long update_no,
		      XMLWriter& xml_out); 

    protected:
    //! Do the measurement
    void func(const unsigned long update_no,
	      XMLWriter& xml_out); 
    private:
      InlineU1BAddParams params;
    };


}

#endif
