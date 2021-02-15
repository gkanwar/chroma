#!/bin/bash

# aclocal && automake && autoconf
. setup.sh

# NOTE(gkanwar): Try to run the same config command as /opt/usqcd-new/...
# to ensure we use the same vectorization, MPI, etc. settings as the other
# libs in that directory.
# /home/gurtej/chroma/configure --prefix=/home/gurtej/chroma --with-qdpc=/opt/usqcd-new/install/platypus/qdp --with-qmp=/opt/usqcd-new/install/platypus/qmp --with-qla=/opt/usqcd-new/install/platypus/qla --with-qio=/opt/usqcd-new/install/platypus/qio --with-qopqdp=/opt/usqcd-new/install/platypus/qopqdp --with-qdp=/opt/usqcd-new/install/platypus/qdp++ --enable-qop-mg --enable-cpp-wilson-dslash --enable-sse-scalarsite-bicgstab-kernels CXX=mpiicpc CC=mpiicc CFLAGS="-Wall -O2 -xavx -funroll-loops  -fomit-frame-pointer -std=gnu99" CXXFLAGS="-Wall -O2 -xavx -funroll-loops  -fomit-frame-pointer -std=c++0x"

# QUDA
#/home/gurtej/chroma/configure --prefix=/home/gurtej/ --with-qmp=/home/gurtej/ --with-qdp=/home/gurtej/qdpxx/build/ --with-qla=/home/gurtej/ --with-quda=/home/gurtej/quda/build_noqio --with-cuda=/usr/ --enable-sse-scalarsite-bicgstab-kernels CXX=mpicxx CC=mpicc CXXFLAGS="-fopenmp -Ofast" LDFLAGS="-lcuda -fopenmp"

# NO GPU
/home/gurtej/chroma/configure --prefix=/home/gurtej/chroma/build_single_nogpu/ --with-qmp=/home/gurtej/ --with-qdp=/home/gurtej/qdpxx/install_single_sse/ --with-qla=/home/gurtej/ --enable-sse-scalarsite-bicgstab-kernels --enable-sse2 CXX=mpicxx CC=mpicc CXXFLAGS="-fopenmp -Ofast" LDFLAGS="-fopenmp"

# QUDA and QDP-JIT, single
#/home/gurtej/chroma/configure --prefix=/home/gurtej/chroma/install_single_qdpjit/ --with-qmp=/home/gurtej/ --with-qdp=/home/gurtej/qdpxx/install_single_nosse/ --with-qla=/home/gurtej/ --with-quda=/home/gurtej/quda/build_qdpjit --with-cuda=/usr/ CXX=mpicxx CC=mpicc CXXFLAGS="-fopenmp -Ofast" LDFLAGS="-lcuda -fopenmp -L/home/gurtej/qdp-jit/install_single_gpu/lib/ -lqdp"

# QUDA and QDP-JIT, double
#/home/gurtej/chroma/configure --prefix=/home/gurtej/chroma/install_double_qdpjit/ --with-qmp=/home/gurtej/ --with-qdp=/home/gurtej/qdpxx/installl_double_nosse/ --with-qla=/home/gurtej/ --with-quda=/home/gurtej/quda/build_qdpjit --with-cuda=/usr/ CXX=mpicxx CC=mpicc CXXFLAGS="-fopenmp -Ofast -DQDP_IS_QDPJIT" LDFLAGS="-lcuda -fopenmp"
