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

#include <Isorropia_TpetraCostDescriber.hpp>
#include <Isorropia_Exception.hpp>

#ifdef HAVE_ISORROPIA_TPETRA


namespace Isorropia {

namespace Tpetra {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
CostDescriber<Node>::CostDescriber()
  : vertex_weights_(),
    graph_edge_weights_(),
    paramlist_(),
    hg_edge_gids_(NULL),
    hg_edge_weights_(NULL),
    num_hg_edge_weights_(0),
    numGlobalVertexWeights_(0),
    numGlobalGraphEdgeWeights_(0),
    numGlobalHypergraphEdgeWeights_(0)
{
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
CostDescriber<Node>::~CostDescriber()
{
  free_hg_edge_weights_();
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
void CostDescriber<Node>::setParameters(const Teuchos::ParameterList& paramlist)
{
  paramlist_ = paramlist;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/** Supply a vector of vertex (row) weights.  If rows are distributed, then
    each process must supply a weight for each of its rows.  (Alternatively
    the application can supply no vertex weights at all.)  The weights should
    be in the same order as the rows in the Tpetra object being partitioned.
*/
////////////////////////////////////////////////////////////////////////////////
template<class Node>
void CostDescriber<Node>::setVertexWeights(Teuchos::RCP<const ::Tpetra::Vector<double,int,int,Node> > vwts)
{
  if (vertex_weights_.get() != 0){
    vertex_weights_.release();
  }
  vertex_weights_ = vwts;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
void CostDescriber<Node>::setGraphEdgeWeights(Teuchos::RCP<const ::Tpetra::CrsMatrix<double,int,int,Node> > gewts)
{
  if (graph_edge_weights_.get() != 0){
    graph_edge_weights_.release();
    graph_self_edges_.clear();
  }
  graph_edge_weights_ = gewts;

  if (gewts->NumMyDiagonals() > 0){

    // Save list of self edges - we omit them in the Zoltan query functions

    const ::Tpetra::Map<int,int,Node> &rowmap = gewts->RowMap();

    ::Tpetra::Vector<double,int,int,Node> diag(rowmap);
    
    gewts->ExtractDiagonalCopy(diag);

    int nvals = gewts->NumMyRows();
    double *entry;
    diag.ExtractView(&entry);
    for (int i=0; i<nvals; i++){
      if (entry[i] != 0){
        graph_self_edges_.insert( rowmap.GID(i));
      }
    }
  }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
void CostDescriber<Node>::setHypergraphEdgeWeights(Teuchos::RCP<const ::Tpetra::Vector<double,int,int,Node> > hgewts)
{
  free_hg_edge_weights_();
  const ::Tpetra::Map<int,int,Node>& map = hgewts->Map();

  int numWeights = map.NumMyElements();

  if (numWeights > 0)
  {
    allocate_hg_edge_weights_(numWeights);
    map.MyGlobalElements(hg_edge_gids_);
    double *v;
    int stride;
    hgewts->ExtractView(&v, &stride);
    for (int i=0; i<numWeights; i++){
      hg_edge_weights_[i] = (float)v[i];
    }
  }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
void CostDescriber<Node>::setHypergraphEdgeWeights(int numHGedges, const int *hgGIDs, const float *hgEwgts)
{
  free_hg_edge_weights_();
  if (numHGedges > 0){
    allocate_hg_edge_weights_(numHGedges);
    for (int i=0; i<numHGedges; i++){
      hg_edge_weights_[i] = hgEwgts[i];
      hg_edge_gids_[i] = hgGIDs[i];
    }
  }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
void CostDescriber<Node>::setHypergraphEdgeWeights(int numHGedges, const int *hgGIDs, const double *hgEwgts)
{
  free_hg_edge_weights_();
  if (numHGedges > 0){
    allocate_hg_edge_weights_(numHGedges);
    for (int i=0; i<numHGedges; i++){
      hg_edge_weights_[i] = (float)hgEwgts[i];
      hg_edge_gids_[i] = hgGIDs[i];
    }
  }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
bool CostDescriber<Node>::haveVertexWeights() const
{
  const int n = getNumVertices();
  return( n > 0);
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
int CostDescriber<Node>::getNumVertices() const
{
  return( vertex_weights_.get()==0 ? 0 : vertex_weights_->MyLength() );
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
int CostDescriber<Node>::getVertexWeights(std::map<int, float> &wgtMap) const
{
  double *wgts;

  const ::Tpetra::Map<int,int,Node>& map = vertex_weights_->Map();
  const int length = map.NumMyElements();

  if (length < 1) return 0;

  int *global_ids = map.MyGlobalElements();
  vertex_weights_->ExtractView(&wgts);

  for(int i=0; i<length; ++i) {
    wgtMap[global_ids[i]] = (float)wgts[i];
  }
  return length;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
void CostDescriber<Node>::getVertexWeights(int numVertices,
                                     int* global_ids,
                                     float* weights) const
{
  if (getNumVertices() == 0){
    return;
  }

  const ::Tpetra::Map<int,int,Node>& map = vertex_weights_->Map();

  if (numVertices != map.NumMyElements()) {
    throw Isorropia::Exception("CostDescriber::getVertexWeights: wrong numVertices");
  }

  map.MyGlobalElements(global_ids);

  double* vals = vertex_weights_->Values();
  for(int i=0; i<numVertices; ++i) {
    weights[i] = vals[i];
  }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
bool CostDescriber<Node>::haveGraphEdgeWeights() const
{
  int n = 0;
  if (graph_edge_weights_.get()){
    n = graph_edge_weights_->NumMyNonzeros();
  }
  return( n > 0);
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
int CostDescriber<Node>::getNumGraphEdges(int vertex_global_id) const
{
  int n = 0;
  if (graph_edge_weights_.get() != 0) {
    int lrid = graph_edge_weights_->LRID(vertex_global_id);
    if (lrid >= 0){   
      n = graph_edge_weights_->NumMyEntries(lrid);

      if (graph_self_edges_.size() > 0){
        std::set<int>::const_iterator it = graph_self_edges_.find(vertex_global_id);
        if (it != graph_self_edges_.end()){
          n--;     // don't count self edges
        }
      }

    }
  }

  return n;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
int CostDescriber<Node>::getEdges(int vertexGID, int len, int *nborGID, float *weights) const
{
  const ::Tpetra::Map<int,int,Node> &colmap = graph_edge_weights_->ColMap();
  const ::Tpetra::Map<int,int,Node> &rowmap = graph_edge_weights_->RowMap();

  int vertexLID = rowmap.LID(vertexGID);
  int numRealEdges = getNumGraphEdges(vertexGID);  //excluding self edges

  if (numRealEdges < 1){
    return 0;
  }

  if (len < numRealEdges ){
    throw Isorropia::Exception("CostDescriber::getEdges: length of allocated arrays");
  }

  int self_edge = 0;
  std::set<int>::const_iterator it = graph_self_edges_.find(vertexGID);
  if (it != graph_self_edges_.end()){
    self_edge = 1;
  }

  int *viewIds;
  double *viewWgts;
  int numedges;         // including self edges

  int rc = graph_edge_weights_->ExtractMyRowView(vertexLID, numedges,
                                                 viewWgts, viewIds);

  if (rc){
    throw Isorropia::Exception("CostDescriber::getEdges: Extract matrix row view");
  }

  if (numedges != (numRealEdges + self_edge)){
    throw Isorropia::Exception("CostDescriber::getEdges: Extract matrix count");
  }

  int nextID = 0;

  for (int j=0; j < numedges; j++){
    int gid = colmap.GID(viewIds[j]);
    if (gid == vertexGID) continue;   // skip the self edges

    nborGID[nextID] = gid;
    weights[nextID] = (float)viewWgts[j];

    nextID++;
  }

  return nextID;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
int CostDescriber<Node>::getGraphEdgeVertices(std::set<int> &gids) const
{
  gids.clear();
  int ngids = 0;

  if (haveGraphEdgeWeights()){
    const ::Tpetra::Map<int,int,Node> &rowmap = graph_edge_weights_->RowMap();
    ngids = rowmap.NumMyElements();
    for (int i=0; i<ngids; i++){
      gids.insert(rowmap.GID(i));
    }
  }
  return ngids;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
int CostDescriber<Node>::getGraphEdgeWeights(int vertex_global_id, std::map<int, float> &wgtMap) const
{
  int rowlen = getNumGraphEdges(vertex_global_id);

  if (rowlen < 1){
    return 0;
  }

  float *wgt = new float [rowlen];
  int *nborGID = new int [rowlen];

  int numEdges = getEdges(vertex_global_id, rowlen, nborGID, wgt);

  for (int i=0; i<numEdges; i++){
    wgtMap[nborGID[i]] = wgt[i];
  }

  if (rowlen > 0){
    delete [] nborGID;
    delete [] wgt;
  }
  return numEdges;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
void CostDescriber<Node>::getGraphEdgeWeights(int vertex_global_id,
                                       int num_neighbors,
                                       int* neighbor_global_ids,
                                       float* weights) const
{
  int rowlen = getNumGraphEdges(vertex_global_id);

  if (rowlen < 1){
    return;
  }

  if (rowlen > num_neighbors) {
    throw Isorropia::Exception("CostDescriber::getGraphEdgeWeights: wrong num_neighbors");
  }

  getEdges(vertex_global_id, num_neighbors, neighbor_global_ids, weights);
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
bool CostDescriber<Node>::haveHypergraphEdgeWeights() const
{
  return(num_hg_edge_weights_ > 0);
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
int CostDescriber<Node>::getNumHypergraphEdgeWeights() const
{
  return num_hg_edge_weights_;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
void CostDescriber<Node>::getHypergraphEdgeWeights(int numEdges,
					     int* global_ids,
					     float* weights) const
{
  if (numEdges != num_hg_edge_weights_) {
    throw Isorropia::Exception("CostDescriber::getHypergraphEdgeWeights: wrong numEdges");
  }

  for(int i=0; i<numEdges; ++i) {
    weights[i] = hg_edge_weights_[i];
    global_ids[i] = hg_edge_gids_[i];
  }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
int CostDescriber<Node>::getHypergraphEdgeWeights(std::map<int, float> &wgtMap) const
{
  int nEdges = num_hg_edge_weights_;
  if (nEdges < 1) return 0;

  for(int i=0; i<nEdges; ++i) {
    wgtMap[hg_edge_gids_[i]] = hg_edge_weights_[i];
  }
  return nEdges;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
void CostDescriber<Node>::getCosts(std::map<int, float> &vertexWeights,
                           std::map<int, std::map<int, float > > &graphEdgeWeights, 
                           std::map<int, float> &hypergraphEdgeWeights) const
{
  if (haveVertexWeights()){
    getVertexWeights(vertexWeights);
  }

  if (haveHypergraphEdgeWeights()){
    getHypergraphEdgeWeights(hypergraphEdgeWeights);
  }

  if (haveGraphEdgeWeights()){
    std::set<int> vgids;
//     int ngids = getGraphEdgeVertices(vgids);
    std::set<int>::iterator curr;
    std::set<int>::iterator end = vgids.end();
    curr = vgids.begin();
    while (curr != end){
      std::map<int, float> nborMap;
      getGraphEdgeWeights(*curr, nborMap);
      graphEdgeWeights[*curr] = nborMap;
      curr++;
    }
  }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
bool CostDescriber<Node>::haveGlobalVertexWeights() const
{
  return (numGlobalVertexWeights_ > 0);
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
void CostDescriber<Node>::setNumGlobalVertexWeights(int num)
{
  numGlobalVertexWeights_ = num;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
bool CostDescriber<Node>::haveGlobalGraphEdgeWeights() const
{
  return (numGlobalGraphEdgeWeights_ > 0);
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
void CostDescriber<Node>::setNumGlobalGraphEdgeWeights(int num)
{
  numGlobalGraphEdgeWeights_ = num;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
bool CostDescriber<Node>::haveGlobalHypergraphEdgeWeights() const
{
  return (numGlobalHypergraphEdgeWeights_ > 0);
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
void CostDescriber<Node>::setNumGlobalHypergraphEdgeWeights(int num)
{
  numGlobalHypergraphEdgeWeights_ = num;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
void CostDescriber<Node>::allocate_hg_edge_weights_(int n)
{
  free_hg_edge_weights_();
  if (n > 0){
    hg_edge_gids_ = new int [n];
    hg_edge_weights_ = new float [n];
    num_hg_edge_weights_ = n;
  }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
void CostDescriber<Node>::free_hg_edge_weights_()
{
  if (hg_edge_gids_){
    delete [] hg_edge_gids_;
    delete [] hg_edge_weights_;
    hg_edge_gids_ = NULL;
    hg_edge_weights_ = NULL;
    num_hg_edge_weights_ = 0;
  }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class Node>
void CostDescriber<Node>::show_cd(std::ostream &os) const
{
  int nv = getNumVertices();
  int nhge = getNumHypergraphEdgeWeights();

  int *gids = NULL;
  if (nv){
    os << "Vertices and weights" << std::endl << "  ";
    gids = new int [nv];
    float *w = new float [nv];

    getVertexWeights(nv, gids, w);
    for (int j=0; j<nv; j++){
      os << gids[j] << " (" << w[j] << ") ";
    }
    os << std::endl;
    delete [] w;
  }
  else{
    os << "No vertex weights" << std::endl;
  }
  if (gids && haveGraphEdgeWeights()){
    os << "Graph edge (non zero) weights for each vertex (row)" << std::endl;
    for (int i=0; i < nv; i++){
      int vid = gids[i];
      std::map<int, float> wgts;

      getGraphEdgeWeights(vid, wgts);

      os << "  Vertex (row) GID " << vid << std::endl << "    ";
      std::map<int, float>::iterator curr;

      for(curr = wgts.begin(); curr != wgts.end(); curr++){
        os << curr->first << " (" << curr->second << ") ";
      } 
      os << std::endl;
    }
  }
  else{
    os << "No graph edge weights" << std::endl;
  }
  if (nhge){
    int *colgids = new int [nhge];
    float *wgts = new float [nhge];

    getHypergraphEdgeWeights(nhge, colgids, wgts);

    os << "Hypergraph Edge (column) weights" << std::endl << "  ";

    for (int j=0; j < nhge; j++){
      os << colgids[j] << " (" << wgts[j] << ") ";
    }
    os << std::endl;

    delete [] colgids;
    delete [] wgts;
  }
  else{
    os << "No hypergraph edge weights" << std::endl;
  }
  
  if (gids) delete [] gids;

  nv = numGlobalVertexWeights_;
  int nge = numGlobalGraphEdgeWeights_;
  nhge = numGlobalHypergraphEdgeWeights_;

  if (paramlist_.begin() == paramlist_.end()){
    os << "No parameters set" << std::endl;
  }
  else{
    os << "Have some parameters set" << std::endl;
  }

  if (haveGlobalVertexWeights()){
    os << "Number of global vertices " << nv << std::endl;
  }
  else{
    os << "Don't know number of global vertices " << std::endl;
  }

  if (haveGlobalGraphEdgeWeights()){
    os << "Number of global graph edge weights " << nge << std::endl;
  }
  else{
    os << "Don't know number of global graph edge weights " << std::endl;
  }

  if (haveGlobalHypergraphEdgeWeights()){
    os << "Number of global hypergraph edge weights " << nhge << std::endl;
  }
  else{
    os << "Don't know number of global hypergraph edge weights " << std::endl;
  }
}
////////////////////////////////////////////////////////////////////////////////

}//namespace Tpetra
}//namespace Isorropia

//std::ostream& operator <<(std::ostream& os, const Isorropia::Tpetra::CostDescriber &cd)
//{
//  cd.show_cd(os);
//  return os;
//}


#endif

