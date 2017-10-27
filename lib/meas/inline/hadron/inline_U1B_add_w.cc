/*! \file
 * Wagman 2017
 * \brief Inline measurement of qpropadd
 *
 * Addition of props
 */

#include "meas/inline/hadron/inline_U1B_add_w.h"
#include "meas/inline/abs_inline_measurement_factory.h"
#include "meas/inline/make_xml_file.h"

#include "meas/inline/io/named_objmap.h"

namespace Chroma 
{ 
  namespace InlineU1BAddEnv 
  { 
    namespace
    {
      AbsInlineMeasurement* createMeasurement(XMLReader& xml_in, 
					      const std::string& path) 
      {
	return new InlineU1BAdd(InlineU1BAddParams(xml_in, path));
      }

      //! Local registration flag
      bool registered = false;
    }

    const std::string name = "U1BADD";

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


  //! Propagator parameters
  void read(XMLReader& xml, const std::string& path, InlineU1BAddParams::NamedObject_t& input)
  {
    XMLReader inputtop(xml, path);

    read(inputtop, "prop_1", input.prop_1);
    read(inputtop, "prop_2", input.prop_2);
    read(inputtop, "prop_f", input.prop_f);
    read(inputtop, "prop_b", input.prop_b);
  }

  //! Propagator parameters
  void write(XMLWriter& xml, const std::string& path, const InlineU1BAddParams::NamedObject_t& input)
  {
    push(xml, path);

    write(xml, "prop_1", input.prop_1);
    write(xml, "prop_2", input.prop_2);
    write(xml, "prop_f", input.prop_f);
    write(xml, "prop_b", input.prop_b);

    pop(xml);
  }

  //! Reader for parameters
  void read(XMLReader& xml, const std::string& path, InlineU1BAddParams::Param_t& param)
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

    read(paramtop, "theta", param.theta);
    read(paramtop, "j_decay", param.j_decay);
    read(paramtop, "t_srce", param.t_srce);
  }

  //! Writer for parameters
  void write(XMLWriter& xml, const std::string& path, const InlineU1BAddParams::Param_t& param)
  {
    push(xml, path);
    int version = 1;
    write(xml, "version", version);
    write(xml, "theta", param.theta);
    write(xml, "j_decay", param.j_decay);
    write(xml, "t_srce", param.t_srce);
    pop(xml);
  }


  // Param stuff
  InlineU1BAddParams::InlineU1BAddParams()
  { 
    frequency = 0; 
  }

  InlineU1BAddParams::InlineU1BAddParams(XMLReader& xml_in, const std::string& path) 
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
  
  void InlineU1BAddParams::write(XMLWriter& xml_out, const std::string& path) 
  {
    push(xml_out, path);
    Chroma::write(xml_out, "Param", param);
    QDP::write(xml_out, "xml_file", xml_file);
    pop(xml_out);
  }

  // Function call
  void 
  InlineU1BAdd::operator()(unsigned long update_no,
			    XMLWriter& xml_out) 
  {
    // If xml file not empty, then use alternate
    if (params.xml_file != "")
    {
      std::string xml_file = makeXMLFileName(params.xml_file, update_no);
      push(xml_out, "U1BAdd");
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
  InlineU1BAdd::func(unsigned long update_no,
		      XMLWriter& xml_out) 
  {
      START_CODE();

      push(xml_out, "U1Badd");
      write(xml_out, "update_no", update_no);

      QDPIO::cout << "QPROPADD: propagator transformation utility" << std::endl;

      // Write out the input
      params.write(xml_out, "Input");

      //
      // Read in the source along with relevant information.
      // 
      XMLReader prop1_file_xml, prop1_record_xml;
    
      LatticePropagator prop1 ;
      LatticePropagator prop2 ;
      QDPIO::cout << "Snarf the props from a named buffer" << std::endl;
      try
      {
	// Try the cast to see if this is a valid source
	prop1 = TheNamedObjMap::Instance().getData<LatticePropagator>(params.named_obj.prop_1);

	TheNamedObjMap::Instance().get(params.named_obj.prop_1).getFileXML(prop1_file_xml);
	TheNamedObjMap::Instance().get(params.named_obj.prop_1).getRecordXML(prop1_record_xml);

	prop2 = TheNamedObjMap::Instance().getData<LatticePropagator>(params.named_obj.prop_2);
      }    
      catch (std::bad_cast)
      {
	QDPIO::cerr << InlineU1BAddEnv::name << ": caught dynamic cast error" 
		    << std::endl;
	QDP_abort(1);
      }
      catch (const std::string& e) 
      {
	QDPIO::cerr << InlineU1BAddEnv::name << ": error extracting source_header: " << e << std::endl;
	QDP_abort(1);
      }


      // Add the props with weights
      // Forward propagator 3
      int n_FT = 2;
      QDPIO::cout<<"Discrete FT with " << n_FT <<" partitions of unity" <<std::endl;
      //LatticeReal lrone = Real(1.0);
      Complex prop_phase, back_phase;
      LatticePropagator propf = zero ;
      LatticePropagator propb = zero ;
      LatticePropagator temp1 = zero ;
      LatticePropagator temp2 = zero ;
      LatticePropagator temp2b = zero ;
      LatticePropagator tempf = zero ;
      LatticePropagator tempb = zero ;
      Real theta = params.param.theta;
      int j_decay = params.param.j_decay;
      multi1d<int> src_coord = params.param.t_srce;
      const multi1d<int>& latt_size = Layout::lattSize();
      int Nt = latt_size[j_decay];
      // For building phases
      LatticeBoolean timeslice;
      LatticeInteger t_coord = Layout::latticeCoordinate(j_decay);
      // Loop through propagators, calculating rolling average of sums
      QDPIO::cout<<"Adding U(1)_B FT phases "<<std::endl;
      QDPIO::cout<<"Nt " << Nt <<std::endl;
      theta /= Nt ;
      // Multiplies propagator by appropriate U(1)_B phase
      for (int t=0; t<Nt; t++) {
        // 1 on timeslice t, 0 everywhere else
        timeslice = (t_coord == t);
        // time relative to source point
        int t_eff = (t - src_coord[Nd-1] + Nt) % Nt;
        // phase for this timeslice
        prop_phase = cmplx(cos(theta*t_eff), -sin(theta*t_eff));
        back_phase = cmplx(cos(theta*Nt), sin(theta*Nt));
        temp1 = prop1/2.0;
        temp2 = prop_phase * prop2/2.0;
        temp2b = back_phase * prop_phase * prop2/2.0;
        float ftheta = toFloat(theta);
        if (ftheta == 0.0) {
          tempf = temp1 + temp2;
          tempb = temp1 - temp2;
        }
        else {
          tempf = temp1 + temp2;
          tempb = temp1 + temp2b;
        }
        // propf += (1/n_FT) * timeslice * prop1 ;
//        copymask(temp1,timeslice,prop1);
//        copymask(temp2,timeslice,prop2);
        copymask(propf,timeslice,tempf);
        copymask(propb,timeslice,tempb);
//        propf += (1/n_FT) * temp1 ;
//        propf += (1/n_FT) * prop_phase * temp2 ;
//        propb += (1/n_FT) * temp1 ;
//        propb += (1/n_FT) * back_phase * prop_phase * temp2 ;
      
      // Stores current position vector
      multi1d<int> npos(4) ;
      npos[0] = 0;
      npos[1] = 0;
      npos[2] = 0;
      //npos[3] = Nt-1;  // time
      npos[3] = t;  // time
      QDPIO::cout<<"Built twist averaged propagators" <<std::endl;
      QDPIO::cout<< t <<std::endl;
      LatticeSpinMatrixD sp1 = peekColor(prop1,0,0);
      LatticeComplex lc1 = peekSpin(sp1,0,0);
      Complex c1 = peekSite(lc1,npos);
      LatticeSpinMatrixD sp2 = peekColor(prop2,0,0);
      LatticeComplex lc2 = peekSpin(sp2,0,0);
      Complex c2 = peekSite(lc2,npos);
      LatticeSpinMatrixD fsp1 = peekColor(temp1,0,0);
      LatticeComplex flc1 = peekSpin(fsp1,0,0);
      Complex fc1 = peekSite(flc1,npos);
      LatticeSpinMatrixD fsp2 = peekColor(temp2,0,0);
      LatticeComplex flc2 = peekSpin(fsp2,0,0);
      Complex fc2 = peekSite(flc2,npos);
      LatticeSpinMatrixD ffsp = peekColor(propf,0,0);
      LatticeComplex fflc = peekSpin(ffsp,0,0);
      Complex ffc = peekSite(fflc,npos);
      LatticeSpinMatrixD bfsp = peekColor(propb,0,0);
      LatticeComplex bflc = peekSpin(bfsp,0,0);
      Complex bfc = peekSite(bflc,npos);
      //      Real prop1test = real(peekSite(peekSpin(peekColor(prop1,0,0),0,0), npos)) ;
      QDPIO::cout << "prop_1 is " << c1 << "\n";
      QDPIO::cout << "prop_2 is " << c2 << "\n";
      QDPIO::cout << "phase is  " << prop_phase << "\n";
      QDPIO::cout << "fac_1 is  " << fc1 << "\n";
      QDPIO::cout << "fac_2 is  " << fc2 << "\n";
      QDPIO::cout << "prop_f is " << ffc << "\n";
      QDPIO::cout << "prop_b is " << bfc << "\n";
      }
//      QDPIO::cout << "prop_2 is " << real(peekSite(peekSpin(peekColor(prop2,0,0),0,0), npos)) << "\n";
//      QDPIO::cout << "timeslice is " << peekSite( timeslice, npos) << "\n";
//      QDPIO::cout << "fac1 is " << real(peekSite((1/n_FT) * timeslice, npos)) << "\n";
//      QDPIO::cout << "fac2 is " << real(peekSite((1/n_FT) * prop_phase * timeslice, npos)) << "\n";
 //     QDPIO::cout << "facprop_1 is " << peekSite(peekSpin(peekColor((1/n_FT) * timeslice*prop1,0,0),0,0), npos) << "\n";
//      QDPIO::cout << "facprop_2 is " << peekSite(peekSpin(peekColor((1/n_FT) * prop_phase * timeslice*prop2,0,0),0,0), npos) << "\n";
//      QDPIO::cout << "prop_f is " << real(peekSite(peekSpin(peekColor(propf,0,0),0,0), npos)) << "\n";
      //propApB = params.named_obj.factorA*propA  +  params.named_obj.factorB*propB;


      /*
       *  Write the a source out to a named buffer
       */
      try
      {
	QDPIO::cout << "Attempt to store sequential source" << std::endl;


	// Store the seqsource
	TheNamedObjMap::Instance().create<LatticePropagator>(params.named_obj.prop_f);
	TheNamedObjMap::Instance().getData<LatticePropagator>(params.named_obj.prop_f) = propf ;
	TheNamedObjMap::Instance().get(params.named_obj.prop_f).setFileXML(prop1_file_xml);
	TheNamedObjMap::Instance().get(params.named_obj.prop_f).setRecordXML(prop1_record_xml);
	TheNamedObjMap::Instance().create<LatticePropagator>(params.named_obj.prop_b);
	TheNamedObjMap::Instance().getData<LatticePropagator>(params.named_obj.prop_b) = propb ;
	TheNamedObjMap::Instance().get(params.named_obj.prop_b).setFileXML(prop1_file_xml);
	TheNamedObjMap::Instance().get(params.named_obj.prop_b).setRecordXML(prop1_record_xml);

	QDPIO::cout << "Propagator sum successfully stored"  << std::endl;
      }
      catch (std::bad_cast)
      {
	QDPIO::cerr << InlineU1BAddEnv::name << ": dynamic cast error" 
		    << std::endl;
	QDP_abort(1);
      }
      catch (const std::string& e) 
      {
	QDPIO::cerr << InlineU1BAddEnv::name << ": error storing seqsource: " << e << std::endl;
	QDP_abort(1);
      }


      pop(xml_out);   // qpropadd
        
      QDPIO::cout << "QpropAdd ran successfully" << std::endl;

      END_CODE();
    }


}  
