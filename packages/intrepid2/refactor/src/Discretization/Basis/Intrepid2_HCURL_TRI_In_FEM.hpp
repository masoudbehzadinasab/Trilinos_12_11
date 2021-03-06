// @HEADER
// ************************************************************************
//
//                           Intrepid2 Package
//                 Copyright (2007) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Kyungjoo Kim  (kyukim@sandia.gov), or
//                    Mauro Perego  (mperego@sandia.gov)
//
// ************************************************************************
// @HEADER

/** \file   Intrepid_HCURL_TRI_In_FEM.hpp
    \brief  Header file for the Intrepid2::HCURL_TRI_In_FEM class.
    \author Created by R. Kirby and P. Bochev and D. Ridzal.
            Kokkorized by Kyungjoo Kim and Mauro Perego
 */

#ifndef __INTREPID2_HCURL_TRI_IN_FEM_HPP__
#define __INTREPID2_HCURL_TRI_IN_FEM_HPP__

#include "Intrepid2_Basis.hpp"
#include "Intrepid2_HGRAD_TRI_Cn_FEM_ORTH.hpp"

#include "Intrepid2_PointTools.hpp"
#include "Teuchos_LAPACK.hpp"

namespace Intrepid2 {

/** \class  Intrepid2::Basis_HCURL_TRI_In_FEM
    \brief  Implementation of the default H(curl)-compatible Nedelec (first kind) 
            basis of arbitrary degree  on Triangle cell.  The lowest order space
            is indexed with 1 rather than 0.
            Implements nodal basis of degree n (n>=1) on the reference Triangle cell. The basis has
            cardinality n(n+2) and spans an INCOMPLETE polynomial space of degree n. 
            Basis functions are dual to a unisolvent set of degrees-of-freedom (DoF) defined by
            \li The tangential component of the vector field at n points per edge
            \li The x and y components evaluated on a lattice of order n+1 with offset 1 (see PointTools)

            If the pointType argument to the constructor specifies equispaced points, then the edge points
            will be equispaced on each edge and the interior points equispaced also.  If
            the pointType argument specifies warp-blend points, then Gauss-Lobatto points of order n
            are chosen on each edge and the interior of warp-blend lattice of order n+1 is chosen for
            the interior points.


 */

#define CardinalityHCurlTri(order) (order*(order+2))

namespace Impl {

class Basis_HCURL_TRI_In_FEM {
public:

  template<EOperator opType>
  struct Serial {
    template<typename outputValueViewType,
    typename inputPointViewType,
    typename workViewType,
    typename vinvViewType>
    KOKKOS_INLINE_FUNCTION
    static void
    getValues(       outputValueViewType outputValues,
        const inputPointViewType  inputPoints,
        workViewType        work,
        const vinvViewType        vinv );
  };

  template<typename ExecSpaceType, ordinal_type numPtsPerEval,
  typename outputValueValueType, class ...outputValueProperties,
  typename inputPointValueType,  class ...inputPointProperties,
  typename vinvValueType,        class ...vinvProperties>
  static void
  getValues(        Kokkos::DynRankView<outputValueValueType,outputValueProperties...> outputValues,
      const Kokkos::DynRankView<inputPointValueType, inputPointProperties...>  inputPoints,
      const Kokkos::DynRankView<vinvValueType,       vinvProperties...>        vinv,
      const EOperator operatorType);

  template<typename outputValueViewType,
  typename inputPointViewType,
  typename vinvViewType,
  EOperator opType,
  ordinal_type numPtsEval>
  struct Functor {
    outputValueViewType _outputValues;
    const inputPointViewType  _inputPoints;
    const vinvViewType        _coeffs;

    KOKKOS_INLINE_FUNCTION
    Functor(       outputValueViewType outputValues_,
        inputPointViewType  inputPoints_,
        vinvViewType        coeffs_ )
    : _outputValues(outputValues_), _inputPoints(inputPoints_), _coeffs(coeffs_) {}

    KOKKOS_INLINE_FUNCTION
    void operator()(const size_type iter) const {
      const auto ptBegin = Util<ordinal_type>::min(iter*numPtsEval,    _inputPoints.dimension(0));
      const auto ptEnd   = Util<ordinal_type>::min(ptBegin+numPtsEval, _inputPoints.dimension(0));

      const auto ptRange = Kokkos::pair<ordinal_type,ordinal_type>(ptBegin, ptEnd);
      const auto input   = Kokkos::subview( _inputPoints, ptRange, Kokkos::ALL() );

      typedef typename outputValueViewType::value_type outputValueType;
      constexpr ordinal_type spaceDim = 2;
      constexpr ordinal_type bufSize = (opType == OPERATOR_CURL) ?
                                       spaceDim * CardinalityHCurlTri(Parameters::MaxOrder)*numPtsEval :
                                       CardinalityHCurlTri(Parameters::MaxOrder)*numPtsEval;

      outputValueType buf[bufSize];

      Kokkos::DynRankView<outputValueType,
      Kokkos::Impl::ActiveExecutionMemorySpace> work(&buf[0], bufSize);

      switch (opType) {
      case OPERATOR_VALUE : {
        auto output = Kokkos::subview( _outputValues, Kokkos::ALL(), ptRange, Kokkos::ALL()  );
        Serial<opType>::getValues( output, input, work, _coeffs );
        break;
      }
      case OPERATOR_CURL: {
        auto output = Kokkos::subview( _outputValues, Kokkos::ALL(), ptRange);
        Serial<opType>::getValues( output, input, work, _coeffs );
        break;
      }
      default: {
        INTREPID2_TEST_FOR_ABORT( true,
            ">>> ERROR: (Intrepid2::Basis_HCURL_TRI_In_FEM::Functor) operator is not supported");

      }
      }
    }
  };
};
}

template<typename ExecSpaceType = void,
    typename outputValueType = double,
    typename pointValueType = double>
class Basis_HCURL_TRI_In_FEM
    : public Basis<ExecSpaceType,outputValueType,pointValueType> {
    public:
  typedef typename Basis<ExecSpaceType,outputValueType,pointValueType>::ordinal_type_array_1d_host ordinal_type_array_1d_host;
  typedef typename Basis<ExecSpaceType,outputValueType,pointValueType>::ordinal_type_array_2d_host ordinal_type_array_2d_host;
  typedef typename Basis<ExecSpaceType,outputValueType,pointValueType>::ordinal_type_array_3d_host ordinal_type_array_3d_host;

  /** \brief  Constructor.
   */
  Basis_HCURL_TRI_In_FEM(const ordinal_type order,
      const EPointType   pointType = POINTTYPE_EQUISPACED);


  typedef typename Basis<ExecSpaceType,outputValueType,pointValueType>::outputViewType outputViewType;
  typedef typename Basis<ExecSpaceType,outputValueType,pointValueType>::pointViewType  pointViewType;
  typedef typename Basis<ExecSpaceType,outputValueType,pointValueType>::scalarViewType  scalarViewType;
  typedef typename Basis<ExecSpaceType,outputValueType,pointValueType>::scalarType  scalarType;

  using Basis<ExecSpaceType,outputValueType,pointValueType>::getValues;

  virtual
  void
  getValues(       outputViewType outputValues,
      const pointViewType  inputPoints,
      const EOperator operatorType = OPERATOR_VALUE) const {
#ifdef HAVE_INTREPID2_DEBUG
    Intrepid2::getValues_HCURL_Args(outputValues,
        inputPoints,
        operatorType,
        this->getBaseCellTopology(),
        this->getCardinality() );
#endif
    constexpr ordinal_type numPtsPerEval = Parameters::MaxNumPtsPerBasisEval;
    Impl::Basis_HCURL_TRI_In_FEM::
    getValues<ExecSpaceType,numPtsPerEval>( outputValues,
        inputPoints,
        this->coeffs_,
        operatorType);
  }

  virtual
  void
  getDofCoords( scalarViewType dofCoords ) const {
#ifdef HAVE_INTREPID2_DEBUG
    // Verify rank of output array.
    INTREPID2_TEST_FOR_EXCEPTION( dofCoords.rank() != 2, std::invalid_argument,
        ">>> ERROR: (Intrepid2::Basis_HCURL_TRI_In_FEM::getDofCoords) rank = 2 required for dofCoords array");
    // Verify 0th dimension of output array.
    INTREPID2_TEST_FOR_EXCEPTION( static_cast<ordinal_type>(dofCoords.dimension(0)) != this->getCardinality(), std::invalid_argument,
        ">>> ERROR: (Intrepid2::Basis_HCURL_TRI_In_FEM::getDofCoords) mismatch in number of dof and 0th dimension of dofCoords array");
    // Verify 1st dimension of output array.
    INTREPID2_TEST_FOR_EXCEPTION( dofCoords.dimension(1) != this->getBaseCellTopology().getDimension(), std::invalid_argument,
        ">>> ERROR: (Intrepid2::Basis_HCURL_TRI_In_FEM::getDofCoords) incorrect reference cell (1st) dimension in dofCoords array");
#endif
    Kokkos::deep_copy(dofCoords, this->dofCoords_);
  }

  void
  getExpansionCoeffs( scalarViewType coeffs ) const {
    // has to be same rank and dimensions
    Kokkos::deep_copy(coeffs, this->coeffs_);
  }

  virtual
  const char*
  getName() const {
    return "Intrepid2_HCURL_TRI_In_FEM";
  }

  virtual
  bool
  requireOrientation() const {
    return (this->basisDegree_ > 1);
  }

    private:

  /** \brief expansion coefficients of the nodal basis in terms of the
        orthgonal one */
  Kokkos::DynRankView<scalarType,ExecSpaceType> coeffs_;

};

}// namespace Intrepid2

#include "Intrepid2_HCURL_TRI_In_FEMDef.hpp"

#endif
