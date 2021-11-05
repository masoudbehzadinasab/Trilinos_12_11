//@HEADER
//************************************************************************
//
//              Isorropia: Partitioning and Load Balancing Package
//                Copyright (2006) Sandia Corporation
//
//Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
//license for use of this work by or on behalf of the U.S. Government.
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
//************************************************************************
//@HEADER

#ifndef _Isorropia_Tpetra_hpp_
#define _Isorropia_Tpetra_hpp_

#include <Isorropia_ConfigDefs.hpp>
#include <Teuchos_RCP.hpp>
#include <Teuchos_ParameterList.hpp>

#include <Isorropia_TpetraRedistributor.hpp>

#ifdef HAVE_ISORROPIA_TPETRA
#include <Tpetra_MultiVector_decl.hpp>
#include <Tpetra_CrsGraph_decl.hpp>
#include <Tpetra_CrsMatrix_decl.hpp>
#include <Kokkos_DefaultNode.hpp>
#endif

namespace Isorropia {

namespace Tpetra {


#ifdef HAVE_ISORROPIA_TPETRA

/** createBalancedCopy() creates a copy with a more balanced map.
    The caller should free the copy after use.
*/

template <typename Node>
::Tpetra::MultiVector<double,int,int,Node> *
createBalancedCopy(const ::Tpetra::MultiVector<double,int,int,Node>& input_vector);

/** createBalancedCopy() creates a copy with a more balanced map.
    The caller should free the copy after use.
*/
template <typename Node>
::Tpetra::MultiVector<double,int,int,Node> *
createBalancedCopy(const ::Tpetra::MultiVector<double,int,int,Node>& input_vector,
                     const Teuchos::ParameterList& paramlist);

/** createBalancedCopy() creates a copy with a more balanced map.
    The caller should free the copy after use.
*/
template <typename Node>
::Tpetra::CrsGraph<int,int,Node>  *
createBalancedCopy(const ::Tpetra::CrsGraph<int,int,Node> & input_graph);

/** createBalancedCopy() creates a copy with a more balanced map.
    The caller should free the copy after use.
*/
template <typename Node>
::Tpetra::CrsGraph<int,int,Node>  *
createBalancedCopy(const ::Tpetra::CrsGraph<int,int,Node> & input_graph,
                     const Teuchos::ParameterList& paramlist);

/** createBalancedCopy() creates a copy with a more balanced map.
    The caller should free the copy after use.
*/
template <typename Node>
::Tpetra::CrsMatrix<double,int,int,Node> *
createBalancedCopy(const ::Tpetra::CrsMatrix<double,int,int,Node>& input_matrix);

/** createBalancedCopy() creates a copy with a more balanced map.
    The caller should free the copy after use.
*/
template <typename Node>
::Tpetra::CrsMatrix<double,int,int,Node> *
createBalancedCopy(const ::Tpetra::CrsMatrix<double,int,int,Node>& input_matrix,
                     const Teuchos::ParameterList& paramlist);

/** redistribute_rows() is an internal Isorropia function, not part
    of the API.

  Return a new Tpetra::CrsMatrix object constructed with target_rowmap,
  and with the contents of input_matrix imported into it.

  The caller is responsible for deleting the returned object.

  param input_matrix Source/input object.

  param target_rowmap Target rowmap, required to be compatible with
     input_matrix.RowMap() in terms of number-of-elements, etc.

  param importer Optional argument. If importer is supplied, it will be
     used to perform the import operation. Otherwise, a temporary importer
     will be created and used.
*/
template <typename Node>
Teuchos::RCP< ::Tpetra::CrsMatrix<double,int,int,Node> >
  redistribute_rows(const ::Tpetra::CrsMatrix<double,int,int,Node>& input_matrix,
                    const ::Tpetra::Map<int,int,Node>& target_rowmap,
                    ::Tpetra::Import<int,int,Node> * importer=0);

/** redistribute_rows() is an internal Isorropia function, not part
    of the API.

    Return a new Tpetra::CrsMatrix object constructed with target_rowmap,
  and with the contents of input_matrix imported into it.

  The caller is responsible for deleting the returned object.

  param input_matrix Source/input object.

  param target_rowmap Target rowmap, required to be compatible with
     input_matrix.RowMatrixRowMap() in terms of number-of-elements, etc.

  param importer Optional argument. If importer is supplied, it will be
     used to perform the import operation. Otherwise, a temporary importer
     will be created and used.
*/
template <typename Node>
Teuchos::RCP< ::Tpetra::CrsMatrix<double,int,int,Node> >
  redistribute_rows(const ::Tpetra::RowMatrix<int,int,Node> & input_matrix,
                    const ::Tpetra::Map<int,int,Node>& target_rowmap,
                    ::Tpetra::Import<int,int,Node> * importer=0);

/** Return a new Tpetra::CrsGraph object constructed with target_rowmap,
  and with the contents of input_graph imported into it.

  param input_graph Source/input object.

  param target_rowmap Target rowmap, required to be compatible with
     input_graph.RowMap() in terms of number-of-elements, etc.

  param importer Optional argument. If importer is supplied, it will be
     used to perform the import operation. Otherwise, a temporary importer
     will be created and used.
*/
template <typename Node>
Teuchos::RCP< ::Tpetra::CrsGraph<int,int,Node> >
  redistribute_rows(const ::Tpetra::CrsGraph<int,int,Node> & input_graph,
                    const ::Tpetra::Map<int,int,Node>& target_rowmap,
                    ::Tpetra::Import<int,int,Node> * importer=0);

/** Return a new Tpetra::MultiVector object constructed with target_map,
  and with the contents of 'input' imported into it.

  param input Source/input object.

  param target_map Target map, required to be compatible with
     input.Map() in terms of number-of-elements, etc.

  param importer Optional argument. If importer is supplied, it will be
     used to perform the import operation. Otherwise, a temporary importer
     will be created and used.
*/
template <typename Node>
Teuchos::RCP< ::Tpetra::MultiVector<double,int,int,Node> >
  redistribute(const ::Tpetra::MultiVector<double,int,int,Node>& input,
               const ::Tpetra::Map<int,int,Node> & target_map,
               ::Tpetra::Import<int,int,Node> * importer=0);

/** Return a new Tpetra::Vector object constructed with target_map,
  and with the contents of 'input' imported into it.

  param input Source/input object.

  param target_map Target map, required to be compatible with
     input.RowMap() in terms of number-of-elements, etc.

  param importer Optional argument. If importer is supplied, it will be
     used to perform the import operation. Otherwise, a temporary importer
     will be created and used.
*/
template <typename Node>
Teuchos::RCP< ::Tpetra::Vector<double,int,int,Node> >
     redistribute(const ::Tpetra::Vector<double,int,int,Node>& input,
                  const ::Tpetra::Map<int,int,Node>& target_map,
                  ::Tpetra::Import<int,int,Node> * importer=0);

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/** Return a vector containing weights that are equal to the number of
  nonzeros per row in the input_matrix. The returned vector will have
  the same size and distribution as input_matrix's row-map.
*/
template <typename Node>
::Tpetra::MultiVector<double,int,int,Node>* create_row_weights_nnz(const ::Tpetra::RowMatrix<int,int,Node> & input_matrix);

/** Return a vector containing weights that are equal to the number of
  nonzeros per row in the input_graph. The returned vector will have
  the same size and distribution as input_graph's row-map.
*/
template <typename Node>
::Tpetra::MultiVector<double,int,int,Node>* create_row_weights_nnz(const ::Tpetra::CrsGraph<int,int,Node> & input_graph);

template <typename Node>
::Tpetra::MultiVector<double,int,int,Node>* create_unit_weights(const ::Tpetra::MultiVector<double,int,int,Node>& input_coords);


/** Calculate a new partitioning, and fill output containers with new
    elements for the local partition, as well as export and import lists.
    This is a simple linear partitioning that does not use Zoltan.

    \param[in] input_map Input map describing the existing or 'old' partitioning.

    \param[in] weights Input vector giving a weight for each element in input_map.
    weights.Map() is required to be the same size and layout as input_map.

    \param[out] newPartitions contains the new partition for each element,
                    in input_map local ID order.  Partition numbers go from
                    0 to numProcs - 1
         
    \param[out] exports the number of exports, that is, the number of
                  elements in newPartitions that are not equal to my
                  process rank

    \param[out] imports the list of global IDs of the elements I will
                   import under the new partitioning

    \return Error-code, 0 if successful. This probably should be a void
    function, since a serious error will result in an exception-throw
    rather than an integer-code error-return.
*/
template <typename Node>
int repartition(const ::Tpetra::Map<int,int,Node> & input_map,
	    const ::Tpetra::MultiVector<double,int,int,Node>& weights,
	    std::vector<int>& myNewElements,
	    int& exportsSize,
	    std::vector<int>& imports);

/**  gather_all_proc_global_offsets() is an internal Isorropia function, not
     part of the API.

    Given an Tpetra::Map object, fill a vector of length numprocs+1
  with each processor's starting offset into the Tpetra::Map's global
  set of elements (the last position will contain num-global-elements).
  Gather the vector of offsets onto all processors.
*/
template <typename Node>
void gather_all_proc_global_offsets(const ::Tpetra::Map<int,int,Node> & blkmap,
                                    std::vector<int>& all_proc_offsets);


/**  compute_imbalance() is an internal Isorropia function, not
     part of the API.

     This function is used by Isorropia to compute the global imbalance
     of an initial partitioning and a new partitioning, to ensure the
     new computed partitioning is better.
*/

double compute_imbalance(int nprocs, std::vector<int> &offsets, 
                         double *wgts, double target);

#endif //DOXYGEN_SHOULD_SKIP_THIS
#endif //HAVE_ISORROPIA_TPETRA

}//namespace Tpetra
}//namespace Isorropia

#endif

