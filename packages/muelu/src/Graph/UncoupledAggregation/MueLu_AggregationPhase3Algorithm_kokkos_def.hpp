// @HEADER
//
// ***********************************************************************
//
//        MueLu: A package for multigrid based preconditioning
//                  Copyright 2012 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
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
// Questions? Contact
//                    Jonathan Hu       (jhu@sandia.gov)
//                    Andrey Prokopenko (aprokop@sandia.gov)
//                    Ray Tuminaro      (rstumin@sandia.gov)
//
// ***********************************************************************
//
// @HEADER
#ifndef MUELU_AGGREGATIONPHASE3ALGORITHM_KOKKOS_DEF_HPP
#define MUELU_AGGREGATIONPHASE3ALGORITHM_KOKKOS_DEF_HPP

#ifdef HAVE_MUELU_KOKKOS_REFACTOR

#include <Teuchos_Comm.hpp>
#include <Teuchos_CommHelpers.hpp>

#include <Xpetra_Vector.hpp>

#include "MueLu_AggregationPhase3Algorithm_kokkos_decl.hpp"

#include "MueLu_Aggregates_kokkos.hpp"
#include "MueLu_Exceptions.hpp"
#include "MueLu_LWGraph_kokkos.hpp"
#include "MueLu_Monitor.hpp"

namespace MueLu {

  // Try to stick unaggregated nodes into a neighboring aggregate if they are
  // not already too big. Otherwise, make a new aggregate
  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  void AggregationPhase3Algorithm_kokkos<LocalOrdinal, GlobalOrdinal, Node>::BuildAggregates(const ParameterList& params, const LWGraph_kokkos& graph, Aggregates_kokkos& aggregates, std::vector<unsigned>& aggStat, LO& numNonAggregatedNodes) const {
    Monitor m(*this, "BuildAggregates");

    bool error_on_isolated = false;
    if(params.isParameter("aggregation: error on nodes with no on-rank neighbors"))
      params.get<bool>("aggregation: error on nodes with no on-rank neighbors");

    const LO  numRows = graph.GetNodeNumVertices();
    const int myRank  = graph.GetComm()->getRank();

    ArrayRCP<LO> vertex2AggId = aggregates.GetVertex2AggId()->getDataNonConst(0);
    ArrayRCP<LO> procWinner   = aggregates.GetProcWinner()  ->getDataNonConst(0);

    LO numLocalAggregates = aggregates.GetNumAggregates();

    for (LO i = 0; i < numRows; i++) {
      if (aggStat[i] == AGGREGATED || aggStat[i] == IGNORED)
        continue;

      typename LWGraph_kokkos::row_type neighOfINode = graph.getNeighborVertices(i);

      // We don't want a singleton. So lets see if there is an unaggregated
      // neighbor that we can also put with this point.
      bool isNewAggregate = false;
      for (int j = 0; j < neighOfINode.size(); j++) {
        LO neigh = neighOfINode(j);

        if (neigh != i && graph.isLocalNeighborVertex(neigh) && aggStat[neigh] == READY) {
          isNewAggregate = true;

          aggStat     [neigh] = AGGREGATED;
          vertex2AggId[neigh] = numLocalAggregates;
          procWinner  [neigh] = myRank;

          numNonAggregatedNodes--;
        }
      }

      if (isNewAggregate) {
        // Create new aggregate (not singleton)
        aggregates.SetIsRoot(i);
        vertex2AggId[i] = numLocalAggregates++;

      } else {
        // We do not want a singleton, but there are no non-aggregated
        // neighbors. Lets see if we can connect to any other aggregates
        // NOTE: This is very similar to phase 2b, but simplier: we stop with
        // the first found aggregate
        int j = 0;
        for (; j < neighOfINode.size(); j++) {
          LO neigh = neighOfINode(j);

          // We don't check (neigh != rootCandidate), as it is covered by checking (aggStat[neigh] == AGGREGATED)
          if (graph.isLocalNeighborVertex(neigh) && aggStat[neigh] == AGGREGATED)
            break;
        }

        if (j < neighOfINode.size()) {
          // Assign to an adjacent aggregate
          vertex2AggId[i] = vertex2AggId[neighOfINode(j)];
	} else if (error_on_isolated) {
	  // Error on this isolated node, as the user has requested
	  std::ostringstream oss;
	  oss<<"MueLu::AggregationPhase3Algorithm::BuildAggregates: MueLu has detected a non-Dirichlet node that has no on-rank neighbors and is terminating (by user request). "<<std::endl;
	  oss<<"If this error is being generated at level 0, this is due to an initial partitioning problem in your matrix."<<std::endl;
	  oss<<"If this error is being generated at any other level, try turning on repartitioning, which may fix this problem."<<std::endl;
	  throw Exceptions::RuntimeError(oss.str());
        } else {
          // Create new aggregate (singleton)
          this->GetOStream(Warnings1) << "Found singleton: " << i << std::endl;

          aggregates.SetIsRoot(i);
          vertex2AggId[i] = numLocalAggregates++;
        }
      }

      // One way or another, the node is aggregated (possibly into a singleton)
      aggStat   [i] = AGGREGATED;
      procWinner[i] = myRank;
      numNonAggregatedNodes--;

    }

    // update aggregate object
    aggregates.SetNumAggregates(numLocalAggregates);
  }

} // end namespace

#endif // HAVE_MUELU_KOKKOS_REFACTOR
#endif // MUELU_AGGREGATIONPHASE3ALGORITHM_KOKKOS_DEF_HPP
