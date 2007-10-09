// -*- C++ -*-
// $Id: inv_eigcg2.cc,v 1.4 2007-10-09 05:28:41 edwards Exp $

#include "inv_eigcg2.h"

//#include "octave_debug.h"
#include "lapack_wrapper.h"
#include "containers.h"
#include <vector>

//#define DEBUG
#define DEBUG_FINAL


// NEEDS A LOT OF CLEAN UP
namespace Chroma 
{

  using namespace LinAlg ;
//  using namespace Octave ;

  //typedef LatticeFermion T ; // save sometyping 

  // needs a little clean up... for possible exceptions if the size of H
  // is smaller than hind 
  void SubSpaceMatrix(Matrix<DComplex>& H,
		      const LinearOperator<LatticeFermion>& A,
		      const multi1d<LatticeFermion>& evec,
		      const int Nvecs)
  {
    LatticeFermion Ap ;
    H.N = Nvecs ;
    if(Nvecs>evec.size()){
      H.N = evec.size();
    }
    if(H.N>H.size()){
      QDPIO::cerr<<"OOPS! your matrix can't take this many matrix elements\n";
      exit(1);	
    }
    // fill H  with zeros
    H.mat = 0.0 ;
    
    for(int i(0);i<H.N;i++){
      A(Ap,evec[i],PLUS) ;
      for(int j(i);j<H.N;j++){
	H(j,i) = innerProduct(evec[j], Ap, A.subset()) ;
	//enforce hermiticity
	H(i,j) = conj(H(j,i));
	if(i==j) H(i,j) = real(H(i,j));
      }
    } 
  }
   
  SystemSolverResults_t InvEigCG2(const LinearOperator<LatticeFermion>& A,
				  LatticeFermion& x, 
				  const LatticeFermion& b,
				  multi1d<Double>& eval, 
				  multi1d<LatticeFermion>& evec,
				  const int Neig,
				  const int Nmax,
				  const Real RsdCG, const int MaxCG)
  {
    START_CODE();

    FlopCounter flopcount;
    flopcount.reset();
    StopWatch swatch;
    swatch.reset();
    swatch.start();
    
    SystemSolverResults_t  res;
    
    LatticeFermion p ; 
    LatticeFermion Ap; 
    LatticeFermion r,z ;

    Double rsd_sq = (RsdCG * RsdCG) * Real(norm2(b,A.subset()));
    Double alphaprev, alpha,pAp;
    Real beta ;
    Double r_dot_z, r_dot_z_old ;
    //Complex r_dot_z, r_dot_z_old,beta ;
    //Complex alpha,pAp ;

    int k = 0 ;
    A(Ap,x,PLUS) ;
    r[A.subset()] = b - Ap ;
    Double r_norm2 = norm2(r,A.subset()) ;

#if 1
    QDPIO::cout << "StathoCG: Nevecs(input) = " << evec.size() << endl;
    QDPIO::cout << "StathoCG: k = " << k << "  res^2 = " << r_norm2 << endl;
#endif
    Real inorm(Real(1.0/sqrt(r_norm2)));
    bool FindEvals = (Neig>0);
    int tr; // Don't know what this does...
    bool from_restart;
    Matrix<DComplex> H(Nmax) ; // square matrix containing the tridiagonal
    Vectors<LatticeFermion> vec(Nmax) ; // contains the vectors we use...
    //-------- Eigenvalue eigenvector finding code ------
    vec.AddVectors(evec,A.subset()) ;
    p[A.subset()] = inorm*r ; // this is not needed GramSchmidt will take care of it
    vec.AddOrReplaceVector(p,A.subset());

    SimpleGramSchmidt(vec.vec,vec.N,vec.N+1,A.subset());
    SimpleGramSchmidt(vec.vec,vec.N,vec.N+1,A.subset()); //call twice: need to improve...
    SubSpaceMatrix(H,A,vec.vec,vec.N);
    from_restart = true ;
    tr = H.N - 1; // this is just a flag as far as I can tell  
    //-------

    Double betaprev ;
    beta=0.0;
    alpha = 1.0;
    // Algorithm from page 529 of Golub and Van Loan
    // Modified to match the m-code from A. Stathopoulos
    while(toBool(r_norm2>rsd_sq)){
      /** preconditioning algorithm **/
      z[A.subset()]=r ; //preconditioning can be added here
      /**/
      r_dot_z = innerProductReal(r,z,A.subset());
      k++ ;
      betaprev = beta; // additional line for Eigenvalue eigenvector code
      if(k==1){
	p[A.subset()] = z ;	
	H.N++ ;
      }
      else{
	beta = r_dot_z/r_dot_z_old ;
	p[A.subset()] = z + beta*p ; 
	//-------- Eigenvalue eigenvector finding code ------
	// fist block
	if(FindEvals){
	  if(!((from_restart)&&(H.N == tr+1))){
	    if(!from_restart){
	      H(H.N-2,H.N-2) = 1/alpha + betaprev/alphaprev;
	    } 
	    from_restart = false ; 
	    H(H.N-1,H.N-2) = -sqrt(beta)/alpha;
	    H(H.N-2,H.N-1) = -sqrt(beta)/alpha;
	  }
	  H.N++ ;
	}
	//---------------------------------------------------
      }
      A(Ap,p,PLUS) ;
      pAp = innerProductReal(p,Ap,A.subset());
      
      alphaprev = alpha ;// additional line for Eigenvalue eigenvector code
      alpha = r_dot_z/pAp ;
      x[A.subset()] += alpha*p ;
      r[A.subset()] -= alpha*Ap ;
      r_norm2 =  norm2(r,A.subset()) ;
      r_dot_z_old = r_dot_z ;

      
      //-------- Eigenvalue eigenvector finding code ------
      // second block
      if(FindEvals){
	if (vec.N==Nmax){//we already have stored the maximum number of vectors
	  // The magic begins here....
QDPIO::cout<<"MAGIC BEGINS: H.N ="<<H.N<<endl ;
	  H(Nmax-1,Nmax-1) = 1/alpha + beta/alphaprev;

#ifdef DEBUG
	  {
	    stringstream tag ;
	    tag<<"H"<<k ;
	    OctavePrintOut(H.mat,Nmax,tag.str(),"Hmatrix.m");
	  }
	  {
	    Matrix<DComplex> tmp(Nmax) ; 
	    SubSpaceMatrix(tmp,A,vec.vec,vec.N);
	    stringstream tag ;
	    tag<<"H"<<k<<"ex" ;
	    OctavePrintOut(tmp.mat,Nmax,tag.str(),"Hmatrix.m");
	  }
#endif
	  //exit(1);
	  multi2d<DComplex> Hevecs(H.mat) ;
	  multi1d<Double> Heval ;
	  char V = 'V' ; char U = 'U' ;
	  Lapack::zheev(V,U,Hevecs,Heval);
	  multi2d<DComplex> Hevecs_old(H.mat) ;

#ifdef DEBUG
 {
   multi1d<LatticeFermion> tt_vec(vec.size());
	  for(int i(0);i<Nmax;i++){
	    tt_vec[i][A.subset()] = zero ;
	    for(int j(0);j<Nmax;j++)
	      tt_vec[i][A.subset()] +=Hevecs(i,j)*vec[j] ; // NEED TO CHECK THE INDEXINT
	  }
	  // CHECK IF vec are eigenvectors...
	  
	    LatticeFermion Av ;
	    for(int i(0);i<Nmax;i++){
	      A(Av,tt_vec[i],PLUS) ;
	      DComplex rq = innerProduct(tt_vec[i],Av,A.subset());
	      Av[A.subset()] -= Heval[i]*tt_vec[i] ;
	      Double tt = sqrt(norm2(Av,A.subset()));
	      QDPIO::cout<<"1 error eigenvector["<<i<<"] = "<<tt<<" " ;
	      tt =  sqrt(norm2(tt_vec[i],A.subset()));
	      QDPIO::cout<<" --- eval = "<<Heval[i]<<" ";
	      QDPIO::cout<<" --- rq = "<<real(rq)<<" ";
	      QDPIO::cout<<"--- norm = "<<tt<<endl  ;
	    }
 }
#endif
	  multi1d<Double> Heval_old ;
	  Lapack::zheev(V,U,Nmax-1,Hevecs_old,Heval_old);
	  for(int i(0);i<Nmax;i++)	    
	    Hevecs_old(i,Nmax-1) = Hevecs_old(Nmax-1,i) = 0.0 ;
#ifdef DEBUG
	  {
	    stringstream tag ;
	    tag<<"Hevecs_old"<<k ;
	    OctavePrintOut(Hevecs_old,Nmax,tag.str(),"Hmatrix.m");
	  }
	  
#endif
	  tr = Neig + Neig ; // v_old = Neig optimal choice
	   
	  for(int i(Neig);i<tr;i++)
	    for(int j(0);j<Nmax;j++)	    
	      Hevecs(i,j) = Hevecs_old(i-Neig,j) ;
#ifdef DEBUG
	  {
	    stringstream tag ;
	    tag<<"Hevecs"<<k ;
	    OctavePrintOut(Hevecs,Nmax,tag.str(),"Hmatrix.m");
	  }
#endif

	  // Orthogonalize the last Neig vecs (keep the first Neig fixed)
	  // zgeqrf(Nmax, 2*Neig, Hevecs, Nmax,
	  //    TAU_CMPLX_ofsize_2Neig, Workarr, 2*Neig*Lapackblocksize, info);
	  multi1d<DComplex> TAU ;
	  Lapack::zgeqrf(Nmax,2*Neig,Hevecs,TAU) ;
	  char R = 'R' ; char L = 'L' ; char N ='N' ; char C = 'C' ;
	  multi2d<DComplex> Htmp = H.mat ;
	  Lapack::zunmqr(R,N,Nmax,Nmax,Hevecs,TAU,Htmp);
	  Lapack::zunmqr(L,C,Nmax,2*Neig,Hevecs,TAU,Htmp);
	  // Notice that now H is of size 2Neig x 2Neig, 
	  // but still with LDA = Nmax 
#ifdef DEBUG
	  {
	    stringstream tag ;
	    tag<<"Htmp"<<k ;
	    OctavePrintOut(Htmp,Nmax,tag.str(),"Hmatrix.m");
	  }
#endif
          Lapack::zheev(V,U,2*Neig,Htmp,Heval);
 	  for(int i(Neig); i< 2*Neig;i++ ) 
	     for(int j(2*Neig); j<Nmax; j++)
                Htmp(i,j) =0.0;
#ifdef DEBUG
          {
            stringstream tag ;
            tag<<"evecstmp"<<k ;
            OctavePrintOut(Htmp,Nmax,tag.str(),"Hmatrix.m");
          }
#endif
	  Lapack::zunmqr(L,N,Nmax,2*Neig,Hevecs,TAU,Htmp);
#ifdef DEBUG
          {
            stringstream tag ;
            tag<<"Yevecstmp"<<k ;
            OctavePrintOut(Htmp,Nmax,tag.str(),"Hmatrix.m");
          }
#endif
	  // Here I need the restart_X bit
	  multi1d<LatticeFermion> tt_vec = vec.vec;
	  for(int i(0);i<2*Neig;i++){
	    vec[i][A.subset()] = zero ;
	    for(int j(0);j<Nmax;j++)
	      vec[i][A.subset()] +=Htmp(i,j)*tt_vec[j] ; // NEED TO CHECK THE INDEXING
	  }

#ifdef DEBUG
	  // CHECK IF vec are eigenvectors...
	  {
	    LatticeFermion Av ;
	    for(int i(0);i<2*Neig;i++){
	      A(Av,vec[i],PLUS) ;
	      DComplex rq = innerProduct(vec[i],Av,A.subset());
	      Av[A.subset()] -= Heval[i]*vec[i] ;
	      Double tt = sqrt(norm2(Av,A.subset()));
	      QDPIO::cout<<"error eigenvector["<<i<<"] = "<<tt<<" " ;
	      tt =  sqrt(norm2(vec[i],A.subset()));
	      QDPIO::cout<<"--- rq ="<<real(rq)<<" ";
	      QDPIO::cout<<"--- norm = "<<tt<<endl  ;
	    }

	  }
#endif
	  H.mat = 0.0 ; // zero out H 
	  for (int i=0;i<2*Neig;i++) H(i,i) = Heval[i];
	  
	  LatticeFermion Ar ;
	  // Need to reorganize so that we aboid this extra matvec
	  A(Ar,r,PLUS) ;
	  Ar /= sqrt(r_norm2); 
	  // this has the oposite convension than the subspace matrix
	  // this is the reason the vector reconstruction in here does not
	  // need the conj(H) while in the Rayleigh Ritz refinement 
	  // it does need it. 
	  // In here the only complex matrix elements are these computined
	  // in the next few lines. This is why the inconsistency 
	  //  does not matter anywhere else. 
	  // In the refinement step though all matrix elements are complex
	  // hence things break down.
	  // Need to fix the convension so that we do not
	  // have these inconsistencies.
	  for (int i=0;i<2*Neig;i++){
	    H(2*Neig,i) = innerProduct(vec[i], Ar, A.subset()) ;
	    H(i,2*Neig) = conj(H(2*Neig,i)) ;
	  }
	  H(2*Neig,2*Neig) = innerProduct(r, Ar, A.subset())/sqrt(r_norm2) ;
	  
	  H.N = 2*Neig + 1  ; // why this ?
	  from_restart = true ;
	  vec.N = 2*Neig ; // only keep the lowest Neig. Is this correct???
#ifdef DEBUG
	  {
	    stringstream tag ;
	    tag<<"finalH"<<k ;
	    OctavePrintOut(H.mat,Nmax,tag.str(),"Hmatrix.m");
	  }
#endif
	}
	// Here we add a vector in the list
	Double inorm = 1.0/sqrt(r_norm2) ;
	vec.NormalizeAndAddVector(r,inorm,A.subset()) ;
	// Shouldn't be the z vector when a preconditioner is used?
      }
      //---------------------------------------------------

      if(k>MaxCG){
	res.n_count = k;
	res.resid   = sqrt(r_norm2);
	QDP_error_exit("too many CG iterations: count = %d", res.n_count);
	END_CODE();
	return res;
      }
#if 1
      QDPIO::cout << "StathoCG: k = " << k << "  res^2 = " << r_norm2 ;
      QDPIO::cout << "  r_dot_z = " << r_dot_z << endl;
#endif
    }
    res.n_count = k ;
    res.resid = sqrt(r_norm2);
    if(FindEvals)
    {
      // Evec Code ------ Before we return --------
      // vs is the current number of vectors stored
      // Neig is the number of eigenvector we want to compute
      SimpleGramSchmidt(vec.vec,0,2*Neig,A.subset());
      SimpleGramSchmidt(vec.vec,0,2*Neig,A.subset());
      Matrix<DComplex> Htmp(2*Neig) ;
      SubSpaceMatrix(Htmp,A,vec.vec,2*Neig);

      char V = 'V' ; char U = 'U' ;
      multi1d<Double> tt_eval ;
      Lapack::zheev(V,U,Htmp.mat,tt_eval);
      evec.resize(Neig) ;
      eval.resize(Neig);
      for(int i(0);i<Neig;i++){
	evec[i][A.subset()] = zero ;
	eval[i] = tt_eval[i] ;
	for(int j(0);j<2*Neig;j++)
	  evec[i][A.subset()] += Htmp(i,j)*vec[j] ;
      }
      
      // I will do the checking of eigenvector quality outside this routine
      // -------------------------------------------
#ifdef DEBUG_FINAL
      // CHECK IF vec are eigenvectors...                                   
      {
	LatticeFermion Av ;
	for(int i(0);i<Neig;i++)
	{
	  A(Av,evec[i],PLUS) ;
	  DComplex rq = innerProduct(evec[i],Av,A.subset());
	  Av[A.subset()] -= eval[i]*evec[i] ;
	  Double tt = sqrt(norm2(Av,A.subset()));
	  QDPIO::cout<<"FINAL: error eigenvector["<<i<<"] = "<<tt<<" " ;
	  tt =  sqrt(norm2(evec[i],A.subset()));
	  QDPIO::cout<<"--- rq ="<<real(rq)<<" ";
	  QDPIO::cout<<"--- norm = "<<tt<<endl  ;
	}
	
      }
#endif
    }

    res.n_count = k;
    res.resid   = sqrt(r_norm2);
    swatch.stop();
    QDPIO::cout << "InvEigCG2: k = " << k << endl;
    flopcount.report("InvEigCG2", swatch.getTimeInSeconds());
    END_CODE();
    return res;
  }

  // A  should be Hermitian positive definite
  SystemSolverResults_t vecPrecondCG(const LinearOperator<LatticeFermion>& A, 
				     LatticeFermion& x, 
				     const LatticeFermion& b, 
				     const multi1d<Double>& eval, 
				     const multi1d<LatticeFermion>& evec, 
				     int startV, int endV,
				     const Real RsdCG, const int MaxCG)
  {
    START_CODE();

    FlopCounter flopcount;
    flopcount.reset();
    StopWatch swatch;
    swatch.reset();
    swatch.start();

    if(endV>eval.size()){
      QDP_error_exit("vPrecondCG: not enought evecs eval.size()=%d",eval.size());
    }
 
    SystemSolverResults_t  res;
    
    LatticeFermion p ; 
    LatticeFermion Ap; 
    LatticeFermion r,z ;

    Double rsd_sq = (RsdCG * RsdCG) * Real(norm2(b,A.subset()));
    Double alpha,pAp;
    Real beta ;
    Double r_dot_z, r_dot_z_old ;
    //Complex r_dot_z, r_dot_z_old,beta ;
    //Complex alpha,pAp ;

    int k = 0 ;
    A(Ap,x,PLUS) ;
    r[A.subset()] = b - Ap ;
    Double r_norm2 = norm2(r,A.subset()) ;

#if 1
    QDPIO::cout << "vecPrecondCG: k = " << k << "  res^2 = " << r_norm2 << endl;
#endif

    // Algorithm from page 529 of Golub and Van Loan
    while(toBool(r_norm2>rsd_sq)){
      /** preconditioning algorithm **/
      z[A.subset()]=r ; // not optimal but do it for now...
      /**/
      for(int i(startV);i<endV;i++){
	DComplex d = innerProduct(evec[i],r,A.subset()) ;
	//QDPIO::cout<<"vecPrecondCG: "<< d<<" "<<(1.0/eval[i]-1.0)<<endl;
	z[A.subset()] += (1.0/eval[i]-1.0)*d*evec[i];
      }
      /**/
      /**/
      r_dot_z = innerProductReal(r,z,A.subset());
      k++ ;
      if(k==1){
	p[A.subset()] = z ;	
      }
      else{
	beta = r_dot_z/r_dot_z_old ;
	p[A.subset()] = z + beta*p ; 
      }
      //Cheb.Qsq(Ap,p) ;
      A(Ap,p,PLUS) ;
      pAp = innerProductReal(p,Ap,A.subset());
      
      alpha = r_dot_z/pAp ;
      x[A.subset()] += alpha*p ;
      r[A.subset()] -= alpha*Ap ;
      r_norm2 =  norm2(r,A.subset()) ;
      r_dot_z_old = r_dot_z ;

      if(k>MaxCG){
	res.n_count = k ;
	QDP_error_exit("too many CG iterations: count = %d", res.n_count);
	return res;
      }
#if 1
      QDPIO::cout << "vecPrecondCG: k = " << k << "  res^2 = " << r_norm2 ;
      QDPIO::cout << "  r_dot_z = " << r_dot_z << endl;
#endif
    }
    
    res.n_count = k ;
    res.resid   = sqrt(r_norm2); 
    swatch.stop();
    QDPIO::cout << "vPreconfCG: k = " << k << endl;
    flopcount.report("vPrecondCG", swatch.getTimeInSeconds());
    END_CODE();
    return res ;
  }
  
  void InitGuess(const LinearOperator<LatticeFermion>& A, 
		LatticeFermion& x, 
		const LatticeFermion& b, 
		const multi1d<Double>& eval, 
		const multi1d<LatticeFermion>& evec, 
		int& n_count){

  int N = evec.size();
  InitGuess(A,x,b,eval,evec,N,n_count);
 }

 void InitGuess(const LinearOperator<LatticeFermion>& A, 
		LatticeFermion& x, 
		const LatticeFermion& b, 
		const multi1d<Double>& eval, 
		const multi1d<LatticeFermion>& evec, 
		const int N, // number of vectors to use
		int& n_count)
 {
   LatticeFermion p ; 
   LatticeFermion Ap; 
   LatticeFermion r ;

   StopWatch snoop;
   snoop.reset();
   snoop.start();

   A(Ap,x,PLUS) ;
   r[A.subset()] = b - Ap ;
   // Double r_norm2 = norm2(r,A.subset()) ;
   
   for(int i(0);i<N;i++){
     DComplex d = innerProduct(evec[i],r,A.subset());
     x[A.subset()] += (d/eval[i])*evec[i];
     //QDPIO::cout<<"InitCG: "<<d<<" "<<eval[i]<<endl ;

   }
   
   snoop.stop();
   QDPIO::cout << "InitGuess:  time = "
              << snoop.getTimeInSeconds() 
              << " secs" << endl;

   n_count = 1 ;
 }

  
}// End Namespace Chroma

