// $Id:  inline_shift_origin.cc  wagman 2017  Exp $ 
/*! \file
 *  \brief shifts gauge field origin 
 */

#include "meas/inline/glue/inline_shift_origin.h"
#include "meas/inline/abs_inline_measurement_factory.h"
#include "meas/glue/mesplq.h"
#include "meas/glue/shift_origin.h"
#include "util/ft/sftmom.h"
#include "util/info/proginfo.h"
// #include "io/param_io.h"
#include "io/qprop_io.h"
// #include "meas/hadron/curcor2_w.h"
#include "meas/inline/make_xml_file.h"
#include "meas/inline/io/named_objmap.h"
#include "meas/smear/no_quark_displacement.h"

namespace Chroma 
{ 
  namespace InlineShiftOriginEnv 
  { 
    namespace
    {
      AbsInlineMeasurement* createMeasurement(XMLReader& xml_in, 
					      const std::string& path) 
      {
	return new InlineShiftOrigin(InlineShiftOriginParams(xml_in, path));
      }

      //! Local registration flag
      bool registered = false;
    }

    // Name of measurement for the XML file
    const std::string name = "SHIFT_ORIGIN";

    //! Register all the factories
    bool registerAll() 
    {
      bool success = true; 
      if (! registered)
      {
	success &= TheInlineMeasurementFactory::Instance().registerObject(name, createMeasurement);
	registered = true;
      }
      return success;
    }
  }



  //! Reader for parameters
  void read(XMLReader& xml, const std::string& path, InlineShiftOriginParams::Param_t& param)
  {
    XMLReader paramtop(xml, path);

    int version;
    read(paramtop, "version", version);

    switch (version) 
    {
    case 1:
      break;

    default:
      QDPIO::cerr << "Input parameter version " << version << " unsupported." << std::endl;
      QDP_abort(1);
    }

    read(paramtop, "t_srce", param.t_srce);
  }

  //! Writer for parameters
  void write(XMLWriter& xml, const std::string& path, const InlineShiftOriginParams::Param_t& param)
  {
    push(xml, path);
    int version = 1;
    write(xml, "version", version);
    write(xml, "t_srce", param.t_srce);
    pop(xml);
  }

  //! Gauge input
  void read(XMLReader& xml, const std::string& path, InlineShiftOriginParams::NamedObject_t& input)
  {
    XMLReader inputtop(xml, path);
    read(inputtop, "gauge_id", input.gauge_id);
    read(inputtop, "gauge_shifted_id", input.gauge_shifted_id);
  }

  //! Gauge output
  void write(XMLWriter& xml, const std::string& path, const InlineShiftOriginParams::NamedObject_t& input)
  {
    push(xml, path);
    write(xml, "gauge_id", input.gauge_id);
    write(xml, "gauge_shifted_id", input.gauge_shifted_id);
    pop(xml);
  }

  // Param stuff
  InlineShiftOriginParams::InlineShiftOriginParams()
  { 
    frequency = 0; 
  }

  InlineShiftOriginParams::InlineShiftOriginParams(XMLReader& xml_in, const std::string& path) 
  {
    try 
    {
      XMLReader paramtop(xml_in, path);
      if (paramtop.count("Frequency") == 1)
          read(paramtop, "Frequency", frequency); else frequency = 1;

      // Parameters for source construction
      read(paramtop, "Param", param);
      // Read in gauge field named object
      read(paramtop, "NamedObject", named_obj);
      // Possible alternate XML file pattern
      if (paramtop.count("xml_file") != 0) 
      {
        read(paramtop, "xml_file", xml_file);
      }
    }
    catch(const std::string& e) 
    {
      QDPIO::cerr << __func__ << ": Caught Exception reading XML: " << e << std::endl;
      QDP_abort(1);
    }
  }
  
  void InlineShiftOriginParams::write(XMLWriter& xml_out, const std::string& path) 
  {
    push(xml_out, path);
    Chroma::write(xml_out, "Param", param);
    QDP::write(xml_out, "xml_file", xml_file);
    pop(xml_out);
  }

  // Function call
  void 
  InlineShiftOrigin::operator()(unsigned long update_no,
			    XMLWriter& xml_out) 
  {
    // If xml file not empty, then use alternate
    if (params.xml_file != "")
    {
      std::string xml_file = makeXMLFileName(params.xml_file, update_no);
      push(xml_out, "ShiftOrigin");
      write(xml_out, "update_no", update_no);
      write(xml_out, "xml_file", xml_file);
      pop(xml_out); 
      XMLFileWriter xml(xml_file);
      func(update_no, xml);
    }
    else
    {
      func(update_no, xml_out);
    }
  }


  // Real work done here
  void 
  InlineShiftOrigin::func(unsigned long update_no,
		      XMLWriter& xml_out) 
  {
    // Test and grab a reference to the gauge field
    XMLBufferWriter gauge_xml;
    try
    {
      TheNamedObjMap::Instance().get(params.named_obj.gauge_id).getRecordXML(gauge_xml);
    }
    catch( std::bad_cast ) 
    {
      QDPIO::cerr << InlineShiftOriginEnv::name << ": caught dynamic cast error" << std::endl;
      QDP_abort(1);
    }
    catch (const std::string& e) 
    {
      QDPIO::cerr << InlineShiftOriginEnv::name << ": map call failed: " << e << std::endl;
      QDP_abort(1);
    }
    // Read original gauge field
    multi1d<LatticeColorMatrix>& u = TheNamedObjMap::Instance().getData< multi1d<LatticeColorMatrix> >(params.named_obj.gauge_id);
    XMLReader gauge_file_xml, gauge_record_xml;
    TheNamedObjMap::Instance().get(params.named_obj.gauge_id).getFileXML(gauge_file_xml);
    TheNamedObjMap::Instance().get(params.named_obj.gauge_id).getRecordXML(gauge_record_xml);
    // Create named object for shifted gauge field
    TheNamedObjMap::Instance().create< multi1d<LatticeColorMatrix> >(params.named_obj.gauge_shifted_id);
    TheNamedObjMap::Instance().get(params.named_obj.gauge_shifted_id).setFileXML(gauge_file_xml);
    TheNamedObjMap::Instance().get(params.named_obj.gauge_shifted_id).setRecordXML(gauge_record_xml);
    TheNamedObjMap::Instance().getData< multi1d<LatticeColorMatrix> >(params.named_obj.gauge_shifted_id) = u;
    multi1d<LatticeColorMatrix>& u_shifted = TheNamedObjMap::Instance().getData< multi1d<LatticeColorMatrix> >(params.named_obj.gauge_shifted_id);
    // References for parameters
    multi1d<int> t_srce = params.param.t_srce;

    push(xml_out, "ShiftOrigin");
    write(xml_out, "update_no", update_no);
    QDPIO::cout << "Shift Origin " << std::endl ;
    QDPIO::cout << "     Gauge group: SU(" << Nc << ")" << std::endl;
    QDPIO::cout << "     volume: " << Layout::lattSize()[0];
    for (int i=1; i<Nd; ++i) {
      QDPIO::cout << " x " << Layout::lattSize()[i];
    }
    QDPIO::cout << std::endl;
    proginfo(xml_out);    // Print out basic program info
    // Write out the input
    params.write(xml_out, "Input");
    // Write out the config info
    write(xml_out, "Config_info", gauge_xml);

    // Run Measurement
    push(xml_out,"elem");
    ShiftOrigin(u, u_shifted, t_srce, xml_out);
    pop(xml_out);  // close elem

    pop(xml_out);  // close ShiftOrigin

    QDPIO::cout << InlineShiftOriginEnv::name << ": ran successfully" << std::endl;
  } 
};
