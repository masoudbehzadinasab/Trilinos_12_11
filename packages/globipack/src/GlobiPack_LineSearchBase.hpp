/*
// @HEADER
// ***********************************************************************
// 
//    GlobiPack: Collection of Scalar 1D globalizaton utilities
//                 Copyright (2009) Sandia Corporation
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
// Questions? Contact Roscoe A. Bartlett (rabartl@sandia.gov) 
// 
// ***********************************************************************
// @HEADER
*/

#ifndef GLOBIPACK_LINE_SEARCH_BASE_HPP
#define GLOBIPACK_LINE_SEARCH_BASE_HPP


#include "GlobiPack_MeritFunc1DBase.hpp"
#include "Teuchos_Describable.hpp"
#include "Teuchos_VerboseObject.hpp"
#include "Teuchos_ParameterListAcceptor.hpp"


namespace GlobiPack {


/** \brief Base class for 1D linearsearch algorithms.
 *
 * ToDo: Finish Documentation!
 */
template<typename Scalar>
class LineSearchBase
  : virtual public Teuchos::Describable,
    virtual public Teuchos::VerboseObject<LineSearchBase<Scalar> >,
    virtual public Teuchos::ParameterListAcceptor
{
public:

  /** \brief Determines if the linesearch algorithm requires the base
   * derivative at <tt>Dphi(0)</tt> or not.
   */
  virtual bool requiresBaseDeriv() const = 0;

  /** \brief Determines if the linesearch algorithm requires that
   * <tt>Dphi(alpha)</tt> can be computed or not.
   */
  virtual bool requiresDerivEvals() const = 0;

  /** \brief Called to perform a linesearch.
   *
   * \param phi [in] The merit function object that will compute the merit
   * function value <tt>phi(alpha)</tt> and/or derivative <tt>Dphi(alpha) at
   * different points <tt>alpha</tt>.  The last call to <tt>phi.eval(...)</tt>
   * will always be at the value of <tt>point_kp1->alpha</tt> returned.
   *
   * \param point_k [in] The evaluation of the merit function and optionally
   * its derivative at <tt>alpha=0.0</tt>.
   *
   * \param point_kp1 [in/out] On input, <tt>point_kp1->alpha</tt> is the
   * initial value to try out (usually 1.0 for most Newton-based algorithms).
   * Also, <tt>point_kp1->phi</tt> must be computed at this value for alpha as
   * well as <tt>point_kp1->Dphi</tt> if required.  On output,
   * <tt>point_kp1->alpha</tt> is the accepted value for a successful line
   * search, or it will be the <tt>alpha</tt> for the minimum
   * <tt>phi(alpha)</tt> found during a failed line search algorithm.
   *
   * <b>Preconditions:</b><ul>
   *
   * <tt>point_k.alpha == 0.0</tt>
   *
   * <tt>point_k.phi != PointEval1D<Scalar>::valNotGiven()</tt>
   *
   * <li> [<tt>this->requiresBaseDeriv()==true</tt>]
   * <tt>point_k.Dphi != PointEval1D<Scalar>::valNotGiven()</tt>
   *
   * <li> [<tt>this->requiresBaseDeriv()==true</tt>]
   * <tt>point_k.Dphi < 0.0</tt> (throw <tt>Exceptions::NotDescentDirection</tt>)
   *
   * <li> [<tt>this->requiresDerivEvals()==true</tt>]
   * <tt>phi.supportsDerivEvals()==true</tt>
   *
   * <li> <tt>!is_null(point_kp1)</tt>
   *
   * <tt>point_kp1->phi != PointEval1D<Scalar>::valNotGiven()</tt>
   *
   * <li> [<tt>this->requiresDerivEvals()==true</tt>]
   * <tt>point_kp1->Dphi != PointEval1D<Scalar>::valNotGiven()</tt>
   *
   * </ul>
   *
   * \returns <tt>true</tt> for successful line search or <tt>false</tt> for a
   * line search failure.
   *
   * This function computes the approximate minimum to 1D merit function
   * <tt>phi(alpha)</tt>.  More specifically the following problem is
   * approximately solved:

   \verbatim
     min  phi(alpha)  s.t. alpha = [0, alpha_upper]<br>
   \endverbatim

   * For many lineserach algorithms, if the initial <tt>point_kp1->alpha</tt>
   * satisfies the internally defined descent requirement, then it will
   * typically be choosen over smaller values of <tt>point_kp1->alpha</tt>
   * that may result in a greater reduction in the given merit function.
   * Other linesearch implementions will actually seek an approximate
   * minimizer.
   *
   * If the maximum number of iterations is exceeded without finding an
   * acceptable point, then the subclass object will return <tt>false</tt> and
   * will return values of <tt>point_kp1->alpha</tt> and
   * <tt>point_kp1->phi</tt> will be for the lowest value of <tt>phi_kp1 =
   * phi(alpha_k)</tt> found.  In this case, the last call to
   * <tt>phi(alpha_k)</tt> will be this best value of <tt>phi_kp1</tt>.
   */
  virtual bool doLineSearch(
    const MeritFunc1DBase<Scalar> &phi,
    const PointEval1D<Scalar> &point_k,
    const Ptr<PointEval1D<Scalar> > &point_kp1,
    const Ptr<int> &numIters
    ) const = 0;

};


} // namespace GlobiPack


#endif // GLOBIPACK_LINE_SEARCH_BASE_HPP
