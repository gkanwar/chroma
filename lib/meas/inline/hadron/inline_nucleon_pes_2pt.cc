// $Id: inline_nucleon_pes_2pt.cc Kanwar 2017 Exp $
/*! \file
 *  \brief measures nucleon correlator for various PES reps
 */

#include "meas/inline/hadron/inline_nucleon_pes_2pt.h"
#include "meas/hadron/nucleon_pes_2pt.h"
#include "meas/inline/abs_inline_measurement_factory.h"
// #include "meas/glue/mesplq.h"
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
  namespace InlineNucleonPES2ptEnv 
  { 
    namespace
    {
      AbsInlineMeasurement* createMeasurement(XMLReader& xml_in, 
					      const std::string& path) 
      {
	return new InlineNucleonPES2pt(InlineNucleonPES2ptParams(xml_in, path));
      }

      //! Local registration flag
      bool registered = false;
    }

    // Name of measurement for the XML file
    const std::string name = "NUCLEON_PES_2PT";

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
  void read(XMLReader& xml, const std::string& path, InlineNucleonPES2ptParams::Param_t& param)
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
  }

  //! Writer for parameters
  void write(XMLWriter& xml, const std::string& path, const InlineNucleonPES2ptParams::Param_t& param)
  {
    push(xml, path);
    int version = 1;
    write(xml, "version", version);
    pop(xml);
  }

  //! Propagator input
  void read(XMLReader& xml, const std::string& path, InlineNucleonPES2ptParams::NamedObject_t::Props_t& input)
  {
    XMLReader inputtop(xml, path);
    read(inputtop, "first_id", input.first_id);
//    read(inputtop, "second_id", input.second_id);
  }

  //! Propagator output
  void write(XMLWriter& xml, const std::string& path, const InlineNucleonPES2ptParams::NamedObject_t::Props_t& input)
  {
    push(xml, path);
    write(xml, "first_id", input.first_id);
//    write(xml, "second_id", input.second_id);
    pop(xml);
  }

  //! Gauge input
  void read(XMLReader& xml, const std::string& path, InlineNucleonPES2ptParams::NamedObject_t& input)
  {
    XMLReader inputtop(xml, path);
    read(inputtop, "gauge_id", input.gauge_id);
    read(inputtop, "sink_pairs", input.sink_pairs);
  }

  //! Gauge output
  void write(XMLWriter& xml, const std::string& path, const InlineNucleonPES2ptParams::NamedObject_t& input)
  {
    push(xml, path);
    write(xml, "gauge_id", input.gauge_id);
    write(xml, "sink_pairs", input.sink_pairs);
    pop(xml);
  }

  // Param stuff
  InlineNucleonPES2ptParams::InlineNucleonPES2ptParams()
  { 
    frequency = 0; 
  }

  InlineNucleonPES2ptParams::InlineNucleonPES2ptParams(XMLReader& xml_in, const std::string& path) 
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
      if (paramtop.count("xml_sym_file") != 0) 
      {
        read(paramtop, "xml_sym_file", xml_sym_file);
        read(paramtop, "xml_m1_file", xml_m1_file);
        read(paramtop, "xml_m2_file", xml_m2_file);
        read(paramtop, "xml_asym_file", xml_asym_file);
      }
    }
    catch(const std::string& e) 
    {
      QDPIO::cerr << __func__ << ": Caught Exception reading XML: " << e << std::endl;
      QDP_abort(1);
    }
  }
  
  void InlineNucleonPES2ptParams::write(XMLWriter& xml_out, const std::string& path) 
  {
    push(xml_out, path);
    Chroma::write(xml_out, "Param", param);
    QDP::write(xml_out, "xml_sym_file", xml_sym_file);
    QDP::write(xml_out, "xml_m1_file", xml_m1_file);
    QDP::write(xml_out, "xml_m2_file", xml_m2_file);
    QDP::write(xml_out, "xml_asym_file", xml_asym_file);
    pop(xml_out);
  }

  // Anonymous namespace
  namespace 
  {
    //! Useful structure holding sink props
    struct SinkPropContainer_t
    {
      ForwardProp_t prop_header;
      std::string quark_propagator_id;
      Real Mass;
      multi1d<int> bc; 
      std::string source_type;
      std::string source_disp_type;
      std::string sink_type;
      std::string sink_disp_type;
    };

    //! Holds light and strange propagators
    struct AllSinkProps_t
    {
      SinkPropContainer_t  sink_prop_1;
//      SinkPropContainer_t  sink_prop_2;
    };

    //! Read a sink prop
    void readSinkProp(SinkPropContainer_t& s, const std::string& id)
    {
      try
      {
	// Try a cast to see if it succeeds
	const LatticePropagator& foo = 
	  TheNamedObjMap::Instance().getData<LatticePropagator>(id);

	// Snarf the data into a copy
	s.quark_propagator_id = id;
	
	// Snarf the prop info. This is will throw if the prop_id is not there
	XMLReader prop_file_xml, prop_record_xml;
	TheNamedObjMap::Instance().get(id).getFileXML(prop_file_xml);
	TheNamedObjMap::Instance().get(id).getRecordXML(prop_record_xml);
   
	// Try to invert this record XML into a ChromaProp struct
	// Also pull out the id of this source
	{
          std::string xpath;
	  read(prop_record_xml, "/SinkSmear", s.prop_header);
	  
	  read(prop_record_xml, "/SinkSmear/PropSource/Source/SourceType", s.source_type);
	  xpath = "/SinkSmear/PropSource/Source/Displacement/DisplacementType";
	  if (prop_record_xml.count(xpath) != 0)
	    read(prop_record_xml, xpath, s.source_disp_type);
	  else
	    s.source_disp_type = NoQuarkDisplacementEnv::getName();

	  read(prop_record_xml, "/SinkSmear/PropSink/Sink/SinkType", s.sink_type);
	  xpath = "/SinkSmear/PropSink/Sink/Displacement/DisplacementType";
	  if (prop_record_xml.count(xpath) != 0)
	    read(prop_record_xml, xpath, s.sink_disp_type);
	  else
	    s.sink_disp_type = NoQuarkDisplacementEnv::getName();
	}
      }
      catch( std::bad_cast ) 
      {
	QDPIO::cerr << InlineNucleonPES2ptEnv::name << ": caught dynamic cast error"  << std::endl;
	QDP_abort(1);
      }
      catch (const std::string& e) 
      {
	QDPIO::cerr << InlineNucleonPES2ptEnv::name << ": error message: " << e 
		    << std::endl;
	QDP_abort(1);
      }
      // Derived from input prop
      // Hunt around to find the mass
      // NOTE: this may be problematic in the future if actions are used with no
      // clear def. of a Mass
      QDPIO::cout << "Try action and mass" << std::endl;
      s.Mass = getMass(s.prop_header.prop_header.fermact);
      s.bc = getFermActBoundary(s.prop_header.prop_header.fermact);
      QDPIO::cout << "FermAct = " << s.prop_header.prop_header.fermact.id << std::endl;
      QDPIO::cout << "Mass = " << s.Mass << std::endl;
    }


    //! Read all sinks
    void readAllSinks(AllSinkProps_t& s, InlineNucleonPES2ptParams::NamedObject_t::Props_t sink_pair)
    {
      QDPIO::cout << "Attempt to parse forward propagator = " << sink_pair.first_id << std::endl;
      readSinkProp(s.sink_prop_1, sink_pair.first_id);
      QDPIO::cout << "Forward propagator successfully parsed" << std::endl;

//      QDPIO::cout << "Attempt to parse forward propagator = " << sink_pair.second_id << std::endl;
//      readSinkProp(s.sink_prop_2, sink_pair.second_id);
//      QDPIO::cout << "Forward propagator successfully parsed" << std::endl;
    }

  } // namespace anonymous

  // Function call
  void 
  InlineNucleonPES2pt::operator()(unsigned long update_no,
			    XMLWriter& xml_out) 
  {
    // If xml file not empty, then use alternate
    if (params.xml_sym_file != "")
    {
      std::string xml_sym_file = makeXMLFileName(params.xml_sym_file, update_no);
      std::string xml_m1_file = makeXMLFileName(params.xml_m1_file, update_no);
      std::string xml_m2_file = makeXMLFileName(params.xml_m2_file, update_no);
      std::string xml_asym_file = makeXMLFileName(params.xml_asym_file, update_no);
      push(xml_out, "NucleonPES2pt");
      write(xml_out, "update_no", update_no);
      write(xml_out, "xml_sym_file", xml_sym_file);
      write(xml_out, "xml_m1_file", xml_m1_file);
      write(xml_out, "xml_m2_file", xml_m2_file);
      write(xml_out, "xml_asym_file", xml_asym_file);
      pop(xml_out); 
      XMLFileWriter xml_sym(xml_sym_file);
      XMLFileWriter xml_m1(xml_m1_file);
      XMLFileWriter xml_m2(xml_m2_file);
      XMLFileWriter xml_asym(xml_asym_file);
      func(update_no, xml_sym, xml_m1, xml_m2, xml_asym);
    }
    else
    {
      func(update_no, xml_out, xml_out, xml_out, xml_out);
    }
  }


  // Real work done here
  void 
  InlineNucleonPES2pt::func(unsigned long update_no,
		      XMLWriter& xml_sym_out, XMLWriter& xml_m1_out, XMLWriter& xml_m2_out, XMLWriter& xml_asym_out) 
  {
    // Test and grab a reference to the gauge field
    XMLBufferWriter gauge_xml;
    try
    {
      QDPIO::cout << "Entering try" << std::endl;
      TheNamedObjMap::Instance().getData< multi1d<LatticeColorMatrix> >(params.named_obj.gauge_id);
      QDPIO::cout << "getData success" << std::endl;
      TheNamedObjMap::Instance().get(params.named_obj.gauge_id).getRecordXML(gauge_xml);
      QDPIO::cout << "get success" << std::endl;
    }
    catch( std::bad_cast ) 
    {
      QDPIO::cerr << InlineNucleonPES2ptEnv::name << ": caught dynamic cast error" << std::endl;
      QDP_abort(1);
    }
    catch (const std::string& e) 
    {
      QDPIO::cerr << InlineNucleonPES2ptEnv::name << ": map call failed: " << e << std::endl;
      QDP_abort(1);
    }
    const multi1d<LatticeColorMatrix>& u = 
      TheNamedObjMap::Instance().getData< multi1d<LatticeColorMatrix> >(params.named_obj.gauge_id);

    push(xml_sym_out, "NucleonPES2pt");
    write(xml_sym_out, "update_no", update_no);
    push(xml_m1_out, "NucleonPES2pt");
    write(xml_m1_out, "update_no", update_no);
    push(xml_m2_out, "NucleonPES2pt");
    write(xml_m2_out, "update_no", update_no);
    push(xml_asym_out, "NucleonPES2pt");
    write(xml_asym_out, "update_no", update_no);
    QDPIO::cout << "Write mom-projected nucleon " << std::endl ;
    QDPIO::cout << "     Gauge group: SU(" << Nc << ")" << std::endl;
    QDPIO::cout << "     volume: " << Layout::lattSize()[0];
    for (int i=1; i<Nd; ++i) {
      QDPIO::cout << " x " << Layout::lattSize()[i];
    }
    QDPIO::cout << std::endl;
    proginfo(xml_sym_out);    // Print out basic program info
    params.write(xml_sym_out, "Input");
    write(xml_sym_out, "Config_info", gauge_xml);
    proginfo(xml_m1_out);    // Print out basic program info
    params.write(xml_m1_out, "Input");
    write(xml_m1_out, "Config_info", gauge_xml);
    proginfo(xml_m2_out);    // Print out basic program info
    params.write(xml_m2_out, "Input");
    write(xml_m2_out, "Config_info", gauge_xml);
    proginfo(xml_asym_out);    // Print out basic program info
    params.write(xml_asym_out, "Input");
    write(xml_asym_out, "Config_info", gauge_xml);


    // Now loop over the various fermion pairs
    for(int lpair=0; lpair < params.named_obj.sink_pairs.size(); ++lpair)
    {
      const InlineNucleonPES2ptParams::NamedObject_t::Props_t named_obj = params.named_obj.sink_pairs[lpair];
      AllSinkProps_t all_sinks;
      readAllSinks(all_sinks, named_obj);
      // Derived from input prop
      multi1d<int> t_src1
                  = all_sinks.sink_prop_1.prop_header.source_header.getTSrce();
//      multi1d<int> t_src2
//                  = all_sinks.sink_prop_2.prop_header.source_header.getTSrce();

      int j_decay = all_sinks.sink_prop_1.prop_header.source_header.j_decay;
      int t0      = all_sinks.sink_prop_1.prop_header.source_header.t_source;
      int bc_spec = all_sinks.sink_prop_1.bc[j_decay] ;
      // Sanity checks
//      {
//	if (all_sinks.sink_prop_2.prop_header.source_header.j_decay != j_decay)
//	{
//	  QDPIO::cerr << "Error!! j_decay must be the same for all propagators " << std::endl;
//	  QDP_abort(1);
//	}
//	if (all_sinks.sink_prop_2.bc[j_decay] != bc_spec)
//	{
//	  QDPIO::cerr << "Error!! bc must be the same for all propagators " << std::endl;
//	  QDP_abort(1);
//	}
//	if (all_sinks.sink_prop_2.prop_header.source_header.t_source != 
//	    all_sinks.sink_prop_1.prop_header.source_header.t_source)
//	{
//	  QDPIO::cerr << "Error!! t_source must be the same for all propagators " << std::endl;
//	  QDP_abort(1);
//	}
//	if (all_sinks.sink_prop_1.source_type != all_sinks.sink_prop_2.source_type)
//	{
//	  QDPIO::cerr << "Error!! source_type must be the same in a pair " << std::endl;
//	  QDP_abort(1);
//	}
//	if (all_sinks.sink_prop_1.sink_type != all_sinks.sink_prop_2.sink_type)
//	{
//	  QDPIO::cerr << "Error!! source_type must be the same in a pair " << std::endl;
//	  QDP_abort(1);
//	}
//      }
      // Initialize the slow Fourier transform phases with .ini-file specified  momenta
//      SftMom phases(0, true, j_decay);
      // References for use later
      const LatticePropagator& sink_prop_1 = 
	TheNamedObjMap::Instance().getData<LatticePropagator>(all_sinks.sink_prop_1.quark_propagator_id);
//      const LatticePropagator& sink_prop_2 = 
//	TheNamedObjMap::Instance().getData<LatticePropagator>(all_sinks.sink_prop_2.quark_propagator_id);
      // Construct group name for output
      std::string src_type;
      if (all_sinks.sink_prop_1.source_type == "POINT_SOURCE")
	src_type = "Point";
      else if (all_sinks.sink_prop_1.source_type == "SHELL_SOURCE")
	src_type = "Shell";
      else if (all_sinks.sink_prop_1.source_type == "WALL_SOURCE")
	src_type = "Wall";
      else
      {
	QDPIO::cerr << "Unsupported source type = " << all_sinks.sink_prop_1.source_type << std::endl;
	QDP_abort(1);
      }
      std::string snk_type;
      if (all_sinks.sink_prop_1.sink_type == "POINT_SINK")
	snk_type = "Point";
      else if (all_sinks.sink_prop_1.sink_type == "SHELL_SINK")
	snk_type = "Shell";
      else if (all_sinks.sink_prop_1.sink_type == "WALL_SINK")
	snk_type = "Wall";
      else
      {
	QDPIO::cerr << "Unsupported sink type = " << all_sinks.sink_prop_1.sink_type << std::endl;
	QDP_abort(1);
      }
      std::string source_sink_type = src_type + "_" + snk_type;
      QDPIO::cout << "Source type = " << src_type << std::endl;
      QDPIO::cout << "Sink type = "   << snk_type << std::endl;

      // Run Measurement
      push(xml_sym_out,"elem");
      push(xml_m1_out,"elem");
      push(xml_m2_out,"elem");
      push(xml_asym_out,"elem");
      nucleon_pes_2pt(u, t_src1, sink_prop_1, xml_sym_out, xml_m1_out, xml_m2_out, xml_asym_out, j_decay, source_sink_type);
      pop(xml_sym_out);  // close elem
      pop(xml_m1_out);  // close elem
      pop(xml_m2_out);  // close elem
      pop(xml_asym_out);  // close elem
    }
    pop(xml_sym_out);  // close NucleonPES2pt
    pop(xml_m1_out);  // close NucleonPES2pt
    pop(xml_m2_out);  // close NucleonPES2pt
    pop(xml_asym_out);  // close NucleonPES2pt

    QDPIO::cout << InlineNucleonPES2ptEnv::name << ": ran successfully" << std::endl;
  } 
};
