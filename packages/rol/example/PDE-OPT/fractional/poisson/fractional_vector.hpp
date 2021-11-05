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

#ifndef ROL_FRACTIONALVECTOR_H
#define ROL_FRACTIONALVECTOR_H

#include "ROL_TpetraMultiVector.hpp"
#include "../../TOOLS/assembler.hpp"
#include <math.h>

template <class Real>
class FractionalVector {
private:
  const Teuchos::RCP<PDE<Real> > pde_local_;
  const Teuchos::RCP<PDE<Real> > pde_cylinder_;
  Teuchos::RCP<Assembler<Real> > assembler_local_;
  Teuchos::RCP<Assembler<Real> > assembler_cylinder_;

  Teuchos::RCP<Tpetra::MultiVector<> > Flocal_;
  Teuchos::RCP<Tpetra::CrsMatrix<> > Klocal_;
  Teuchos::RCP<Tpetra::CrsMatrix<> > Mcylinder_;

  Teuchos::RCP<Tpetra::MultiVector<> > Frcp_;
  Teuchos::RCP<ROL::Vector<Real> > F_;

public:
  FractionalVector(const Teuchos::RCP<PDE<Real> > &pde_local,
                   const Teuchos::RCP<MeshManager<Real> > &mesh_local,
                   const Teuchos::RCP<PDE<Real> > &pde_cylinder,
                   const Teuchos::RCP<MeshManager<Real> > &mesh_cylinder,
                   const Teuchos::RCP<const Teuchos::Comm<int> > &comm,
                   Teuchos::ParameterList &parlist,
                   std::ostream &outStream = std::cout)
    : pde_local_(pde_local), pde_cylinder_(pde_cylinder) {
    Teuchos::RCP<Tpetra::MultiVector<> > uvec;
    Teuchos::RCP<Tpetra::MultiVector<> > zvec;
    // Assemble local components
    assembler_local_ = Teuchos::rcp(new Assembler<Real>(pde_local_->getFields(),mesh_local,comm,parlist,outStream));
    assembler_local_->setCellNodes(*pde_local_);
    uvec = assembler_local_->createStateVector();   uvec->putScalar(static_cast<Real>(0));
    zvec = assembler_local_->createControlVector(); zvec->putScalar(static_cast<Real>(0));
    assembler_local_->assemblePDEJacobian1(Klocal_,pde_local_,uvec,zvec);
    assembler_local_->assemblePDEResidual(Flocal_,pde_local_,uvec,zvec);
    // Assemble cylinder components
    assembler_cylinder_ = Teuchos::rcp(new Assembler<Real>(pde_cylinder_->getFields(),mesh_cylinder,comm,parlist,outStream));
    assembler_cylinder_->setCellNodes(*pde_cylinder_);
    assembler_cylinder_->assemblePDERieszMap1(Mcylinder_,pde_cylinder_);
    // Build fractional vector
    Real s     = parlist.sublist("Problem").get("Fractional Power",0.5);
    Real alpha = static_cast<Real>(1) - static_cast<Real>(2)*s;
    Real ds    = std::pow(static_cast<Real>(2), alpha) * tgamma(static_cast<Real>(1)-s)/tgamma(s);
    Frcp_ = Teuchos::rcp(new Tpetra::MultiVector<>(Klocal_->getRowMap(),Mcylinder_->getGlobalNumCols()));
    Frcp_->getVectorNonConst(0)->scale(-ds,*Flocal_);
    F_ = Teuchos::rcp(new ROL::TpetraMultiVector<Real>(Frcp_));
  }

  FractionalVector(const Teuchos::RCP<const Tpetra::MultiVector<> > &F,
                   const Teuchos::RCP<const Tpetra::Map<> > &map,
                   const int numCylinder,
                   const Real s) {
    // Build fractional vector
    Real alpha = static_cast<Real>(1) - static_cast<Real>(2)*s;
    Real ds    = std::pow(static_cast<Real>(2), alpha) * tgamma(static_cast<Real>(1)-s)/tgamma(s);
    Frcp_ = Teuchos::rcp(new Tpetra::MultiVector<>(map,numCylinder));
    Frcp_->getVectorNonConst(0)->scale(-ds,*F);
    F_ = Teuchos::rcp(new ROL::TpetraMultiVector<Real>(Frcp_));
  }

  const Teuchos::RCP<const ROL::Vector<Real> > get(void) const {
    return F_;
  }
};

#endif
