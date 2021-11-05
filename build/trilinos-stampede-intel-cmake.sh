#!/usr/bin/env bash

module load cmake python2 boost phdf5 netcdf 
export F77='which mpif90 || /bin/true'
export COPTFLAGS="-O1"
export CC='which mpicc || /bin/true'
export CXX='which mpicxx || /bin/true'

export INSTALL_LOCATION=$HOME'/projects/trilinos/'

cmake -VV \
  -D BUILD_SHARED_LIBS:BOOL=ON \
  -D Trilinos_VERBOSE_CONFIGURE=OFF \
  -D CMAKE_VERBOSE_MAKEFILE=ON \
  -D Trilinos_ENABLE_ALL_PACKAGES:BOOL=OFF \
  -D Trilinos_ENABLE_TESTS:BOOL=OFF \
  -D Trilinos_ENABLE_EXAMPLES:BOOL=OFF \
  \
  -D CMAKE_INSTALL_PREFIX:PATH=${INSTALL_LOCATION} \
  -D CMAKE_BUILD_TYPE:STRING=RELEASE \
  -D CMAKE_C_FLAGS:STRING="${COPTFLAGS} -mkl" \
  -D CMAKE_CXX_FLAGS:STRING="${COPTFLAGS} -mkl -DMPICH_SKIP_MPICXX" \
  \
  -D BLAS_INCLUDE_DIRS:PATH="${TACC_MKL_INC}" \
  -D BLAS_LIBRARY_DIRS:PATH="${TACC_MKL_LIB}" \
  -D BLAS_LIBRARY_NAMES:STRING="mkl_intel_lp64;mkl_sequential;mkl_core;pthread" \
  -D LAPACK_INCLUDE_DIRS:PATH="${TACC_MKL_INC}" \
  -D LAPACK_LIBRARY_DIRS:PATH="${TACC_MKL_LIB}" \
  -D LAPACK_LIBRARY_NAMES:STRING="mkl_intel_lp64;mkl_sequential;mkl_core;pthread" \
  \
  -D TPL_ENABLE_MPI:BOOL=ON \
  -D MPI_EXEC:FILEPATH="/usr/local/bin/ibrun" \
  -D TPL_ENABLE_GLM=OFF \
  -D TPL_ENABLE_Matio=OFF \
  \
  -D TPL_ENABLE_Boost:BOOL=ON \
  -D Boost_INCLUDE_DIRS:PATH=$TACC_BOOST_INC      \
  -D Boost_LIBRARY_DIRS:PATH=$TACC_BOOST_LIB      \
  -D TPL_ENABLE_BoostLib:BOOL=ON \
  -D BoostLib_INCLUDE_DIRS:PATH=$TACC_BOOST_INC      \
  -D BoostLib_LIBRARY_DIRS:PATH=$TACC_BOOST_LIB      \
  \
  -D TPL_ENABLE_HDF5:BOOL=ON \
  -D HDF5_INCLUDE_DIRS:PATH=$TACC_HDF5_INC    \
  -D HDF5_LIBRARY_DIRS:PATH=$TACC_HDF5_LIB    \
  -D TPL_ENABLE_Netcdf:BOOL=ON \
  -D Netcdf_INCLUDE_DIRS:PATH=$TACC_NETCDF_INC    \
  -D Netcdf_LIBRARY_DIRS:PATH=$TACC_NETCDF_LIB    \
  \
  -D Trilinos_ENABLE_ALL_OPTIONAL_PACKAGES:BOOL=ON \
  -D Trilinos_ENABLE_ALL_FORWARD_DEP_PACKAGES:BOOL=ON    \
  -D Trilinos_ENABLE_AztecOO:BOOL=ON \
  -D Trilinos_ENABLE_Belos:BOOL=ON \
  -D Trilinos_ENABLE_Epetra:BOOL=ON \
  -D Trilinos_ENABLE_EpetraExt:BOOL=ON \
  -D Trilinos_ENABLE_FEI:BOOL=OFF \
  -D Trilinos_ENABLE_Ifpack:BOOL=ON \
  -D Trilinos_ENABLE_Intrepid:BOOL=ON \
  -D Trilinos_ENABLE_Isorropia:BOOL=ON \
  -D Trilinos_ENABLE_MueLu:BOOL=ON \
  -D Trilinos_ENABLE_NOX:BOOL=ON \
  -D Trilinos_ENABLE_Pamgen:BOOL=ON \
  -D Trilinos_ENABLE_Panzer:BOOL=OFF \
  -D Trilinos_ENABLE_Phalanx:BOOL=ON \
  -D Phalanx_EXPLICIT_TEMPLATE_INSTANTIATION:BOOL=ON \
  -D Trilinos_ENABLE_Sacado:BOOL=ON \
  -D Trilinos_ENABLE_SEACAS:BOOL=ON \
  -D Trilinos_ENABLE_SECONDARY_STABLE_CODE:BOOL=ON \
  -D Trilinos_ENABLE_Shards:BOOL=ON \
  -D Trilinos_ENABLE_STK:BOOL=OFF \
  -D Trilinos_ENABLE_Sundance:BOOL=OFF \
  -D Trilinos_ENABLE_Teuchos:BOOL=ON \
  -D Trilinos_ENABLE_Tpetra:BOOL=OFF \
  -D Trilinos_ENABLE_Zoltan:BOOL=ON \
  \
  -D CMAKE_PYTHON_INCLUDE_DIR:PATH="${TACC_PYTHON2_INC}" \
  -D CMAKE_PYTHON_LIBRARIES:STRING="${TACC_PYTHON2_LIB}" \
  -D Trilinos_ENABLE_PyTrilinos:Bool=OFF \
  \
  ..
