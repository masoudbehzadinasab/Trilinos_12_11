// @HEADER
// ************************************************************************
//
//               Rapid Optimization Library (ROL) Package
//                 Copyright (2014) Sandia Corporation
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
// Questions? Contact lead developers:
//              Drew Kouri   (dpkouri@sandia.gov) and
//              Denis Ridzal (dridzal@sandia.gov)
//
// ************************************************************************
// @HEADER

#ifndef ROL_TEUCHOS_EQUALITYCONSTRAINT_H
#define ROL_TEUCHOS_EQUALITYCONSTRAINT_H

#include "ROL_EqualityConstraint.hpp"
#include "ROL_TeuchosVector.hpp"

/** @ingroup func_group
    \class ROL::TeuchosEqualityConstraint
    \brief Defines the equality constraint operator interface for 
           Teuchos::SerialDenseVectors

*/

namespace ROL {

template <class Ordinal, class Real>
class TeuchosEqualityConstraint : public EqualityConstraint<Real> {

  template <typename T> using RCP = Teuchos::RCP<T>;

  typedef Teuchos::SerialDenseVector<Ordinal,Real> SDV;
  typedef TeuchosVector<Ordinal,Real>              TV;

public:

  virtual ~TeuchosEqualityConstraint() {}


  using EqualityConstraint<Real>::update;
  void update( const Vector<Real> &x, bool flag = true, int iter = -1 ) {
    RCP<const SDV> xp = Teuchos::dyn_cast<const TV>(x).getVector();
    update(*xp,flag,true);
  }

  virtual void update( const SDV &x, Real &tol ) {}  


  using EqualityConstraint<Real>::value;
  void value(Vector<Real> &c, const Vector<Real> &x, Real &tol) {
    RCP<SDV> cp = Teuchos::dyn_cast<TV>(c).getVector();
    RCP<const SDV> xp = Teuchos::dyn_cast<const TV>(x).getVector();

    value(*cp,*xp,tol);
  }

  virtual void value( SDV &c, const SDV &x, Real &tol ) = 0;


  using EqualityConstraint<Real>::applyJacobian;
  void applyJacobian(Vector<Real> &jv, const Vector<Real> &v, 
                             const Vector<Real> &x, Real &tol) {

    RCP<SDV> jvp = Teuchos::dyn_cast<TV>(jv).getVector();
    RCP<const SDV> vp = Teuchos::dyn_cast<const TV>(v).getVector();
    RCP<const SDV> xp = Teuchos::dyn_cast<const TV>(x).getVector();
    try {
      applyJacobian(*jvp,*vp,*xp,tol);      
    } 
    catch (std::exception &e ){
      EqualityConstraint<Real>::applyJacobian(jv,v,x,tol);
    }
  }

  virtual void applyJacobian( SDV &jv, const SDV v, 
                              const SDV &x, Real &tol ) {
    TEUCHOS_TEST_FOR_EXCEPTION(true, std::invalid_argument,
      ">>> ERROR (ROL::TeuchosEqualityConstraint): applyJacobian not implemented!");
  }


  using EqualityConstraint<Real>::applyAdjointJacobian;
  void applyAdjointJacobian(Vector<Real> &ajv,     const Vector<Real> &v,
                                    const Vector<Real> &x, Real &tol) {

    RCP<SDV> ajvp = Teuchos::dyn_cast<TV>(ajv).getVector();
    RCP<const SDV> vp = Teuchos::dyn_cast<const TV>(v).getVector();
    RCP<const SDV> xp = Teuchos::dyn_cast<const TV>(x).getVector();

    try {
       applyJacobian(*ajvp,*vp,*xp,tol);      
    } 
    catch (std::exception &e ){
      EqualityConstraint<Real>::applyAdjointJacobian(ajv,v,x,tol);
    }
  }

   virtual void applyAdjointJacobian( SDV &ajv, const SDV v, 
                                      const SDV &x, Real &tol ) {
    TEUCHOS_TEST_FOR_EXCEPTION(true, std::invalid_argument,
      ">>> ERROR (ROL::TeuchosEqualityConstraint): applyAdjointJacobian not implemented!");
  }


  using EqualityConstraint<Real>::applyAdjointHessian;
  void applyAdjointHessian(Vector<Real> &ahuv, const Vector<Real> &u, const Vector<Real> &v,
                           const Vector<Real> &x, Real &tol) {

    RCP<SDV> ahuvp = Teuchos::dyn_cast<TV>(ahuv).getVector();
    RCP<const SDV> up = Teuchos::dyn_cast<const TV>(u).getVector();
    RCP<const SDV> vp = Teuchos::dyn_cast<const TV>(v).getVector();
    RCP<const SDV> xp = Teuchos::dyn_cast<const TV>(x).getVector();

    try {
      applyAdjointHessian( *ahuvp, *up, *vp, *xp, tol );
    }
    catch (std::exception &e) {
      EqualityConstraint<Real>::applyAdjointHessian(ahuv,u,v,x,tol);
    }   

  }

  virtual void applyAdjointHessian( SDV &ahuv, const SDV &u,
                                    const SDV &v, const SDV &x,
                                    Real &tol ) {
    TEUCHOS_TEST_FOR_EXCEPTION(true, std::invalid_argument, 
      ">>> ERROR (ROL::TeuchosEqualityConstraint) : applyAdjointHessian not implemented!");
  }


  using EqualityConstraint<Real>::solveAugmentedSystem;
  SDV solveAugmentedSystem(Vector<Real> &v1, Vector<Real> &v2,
                                         const Vector<Real> &b1, const Vector<Real> &b2,
                                         const Vector<Real> &x, Real &tol) {

    RCP<SDV> v1p = Teuchos::dyn_cast<TV>(v1).getVector();
    RCP<SDV> v2p = Teuchos::dyn_cast<TV>(v2).getVector();
    RCP<const SDV> b1p = Teuchos::dyn_cast<const TV>(b1).getVector();
    RCP<const SDV> b2p = Teuchos::dyn_cast<const TV>(b2).getVector();
    RCP<const SDV> xp = Teuchos::dyn_cast<const TV>(x).getVector();

    try {
      solveAugmentedSystem( *v1p, *v2p, *b1p, *b2p, tol );
    }
    catch (std::exception &e) {
      EqualityConstraint<Real>::solveAugmentedSystem(v1,v2,b1,b2,x,tol); 
    }
  }

  virtual SDV solveAugmentedSystem( SDV &v1, SDV &v2,
                                                  const SDV &b1, const SDV &b2,
                                                  const SDV &x, Real tol ) {
    TEUCHOS_TEST_FOR_EXCEPTION(true, std::invalid_argument, 
      ">>> ERROR (ROL::TeuchosEqualityConstraint) : solveAugmentedSystem not implemented!");
  }


  using EqualityConstraint<Real>::applyPreconditioner;
  void applyPreconditioner(Vector<Real> &Pv, const Vector<Real> &v, const Vector<Real> &x,
                           const Vector<Real> &g, Real &tol) {

    RCP<SDV> Pvp = Teuchos::dyn_cast<TV>(Pv).getVector();
    RCP<const SDV> vp = Teuchos::dyn_cast<const TV>(v).getVector();
    RCP<const SDV> xp = Teuchos::dyn_cast<const TV>(x).getVector();

    try {
      applyPreconditioner( *Pvp, *vp, *xp, *gp, tol );
    }
    catch (std::exception &e) {
      EqualityConstraint<Real>::applyPreconditioner(pv,v,x,g,tol);
    }
  }

  virtual void applyPreconditioner( SDV &pv, const SDV &v,
                                    const SDV &x, const SDV &g, Real &tol ) {
    TEUCHOS_TEST_FOR_EXCEPTION(true, std::invalid_argument, 
      ">>> ERROR (ROL::TeuchosEqualityConstraint) : applyPreconditioner not implemented!");
  }


}; // class TeuchosEqualityConstraint

} // namespace ROL


#endif // ROL_TEUCHOS_EQUALITYCONSTRAINT_H
