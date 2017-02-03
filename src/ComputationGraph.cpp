#include<iostream>
#include<vector>

#include "ComputationGraph.hpp"

using namespace std;

//Transpose the matrix
void Transpose_mtx(const vector<vector<int>> &in_mtx, vector<vector<int>> &out_mtx)
{
  for(size_t j=0; j<in_mtx[0].size(); j++){
    vector<int> out_mtx_row;
    out_mtx.push_back(out_mtx_row);
  }

  for(size_t i=0; i<in_mtx.size(); i++){
    for(size_t j=0; j<in_mtx[0].size(); j++){
      out_mtx[j].push_back(in_mtx[i][j]);
    }
  }
}

void VecToMtx_horizontal(vector<int> &in_vec, vector<vector<int>> &out_mtx, int num_row)
{
  for(int i=0; i<num_row; i++){
    out_mtx.push_back(in_vec);
  }
}

void VecToMtx_vertical(vector<int> &in_vec, vector<vector<int>> &out_mtx, int num_col)
{
  for(size_t i=0; i<in_vec.size(); i++){
    vector<int> out_mtx_row;
    for(int j=0; j<num_col; j++){
      out_mtx_row.push_back(in_vec[i]);
    }
    out_mtx.push_back(out_mtx_row);
  }
}

void MtxToVec_vertical(vector<vector<int>> &in_mtx, vector<int> &out_vec)
{
  for(size_t i=0; i<in_mtx.size(); i++){
    out_vec.push_back(in_mtx[i][0]);
  }
}

void MtxToVec_horizontal(vector<vector<int>> &in_mtx, vector<int> &out_vec)
{
  for(size_t j=0; j<in_mtx[0].size(); j++){
    out_vec.push_back(in_mtx[0][j]);
  }
}

void SToVec(int &in_s, vector<int> &out_vec, int n)
{
  for(int i=0; i<n; i++){
    out_vec.push_back(in_s);
  }
}

void ReplaceLastEle(const vector<int> &in_vec, int &s, vector<int> &out_vec)
{
  for(size_t i=0; i<in_vec.size(); i++){
    if(i == in_vec.size()-1){
      out_vec.push_back(s);
    }
    else{
      out_vec.push_back(in_vec[i]);
    }
  }
}

void GetRowUnderDiag(vector<vector<int>> &in_mtx, vector<int> &out_vec, int row_idx){
  for(int j=0; j<row_idx; j++){
    out_vec.push_back(in_mtx[row_idx][j]);
  }
}

void GetDiag(const vector<vector<int>> &in_mtx, vector<int> &out_diag){
  for(size_t i=0; i<in_mtx.size(); i++){
    out_diag.push_back(in_mtx[i][i]);
  }
}

void GetElement(vector<vector<int>> &in_mtx, int i, int j, int &out_s)
{
  out_s = in_mtx[i][j];
}

//get first k columns of mtx
void GetColumns(const vector<vector<int>> &in_mtx, int k, vector<vector<int>>& out_mtx){
  for(size_t i=0; i<in_mtx.size(); i++){
    vector<int> row;
    for(int j=0; j<k; j++){
      row.push_back(in_mtx[i][j]);
    }
    out_mtx.push_back(row);
  }
}


void Mtx_PlaneRotation(int c, int s, int ns, int n, int p, int q, vector<vector<int>> &out_mtx)
{
  //P matrix
  for(int i=0; i<n; i++){
    vector<int> mtx_row;
    for(int j=0; j<n; j++){
      if((i == p) && (j == p)){
	mtx_row.push_back(c);
      }
      else if((i == p) && (j == q)){
	mtx_row.push_back(s);
      }
      else if((i == q) && (j == p)){
	mtx_row.push_back(ns);
      }
      else if((i == q) && (j == q)){
	mtx_row.push_back(c);
      }
      else{
	mtx_row.push_back(-1);
      }
    }
    out_mtx.push_back(mtx_row);
  }
}

void Mtx_extend(const vector<vector<int>> &in_mtx, int n, vector<vector<int>> &out_mtx)
{
  for(int i=0; i<n; i++){
    vector<int> row;
    for(int j=0; j<n; j++){
      if((i<int(in_mtx.size())) && (j<int(in_mtx[0].size()))){
	row.push_back(in_mtx[i][j]);
      }
      else{
	row.push_back(-1);
      }
    }
    out_mtx.push_back(row);
  }
}

void Vec_const(int m, vector<int> &out_vec){
  for(int i=0; i<m; i++){
    out_vec.push_back(-1);
  }
}

void Mtx_const(int m, int n, vector<vector<int>> &out_mtx){
  for(int i=0; i<m; i++){
    vector<int> row;
    Vec_const(n, row);
    out_mtx.push_back(row);
  }
}


//------------------------------------------------------------------------------------
void ClearMtx(vector<vector<int>> &mtx)
{
  for(size_t i=0; i<mtx.size(); i++){
    mtx[i].clear();
  }
  mtx.clear();
}

//-------------------------------------------------------------------------------------

ComputationGraph::~ComputationGraph(){
  for(vector<Pattern*>::iterator p=patterns.begin(); p!=patterns.end(); p++){
    delete *p;
  }
}

void ComputationGraph::PrintOps()
{
  for(vector<Pattern*>::iterator i=patterns.begin(); i!=patterns.end(); i++){
    (*i)->PrintOps(ops);
    cout << endl;
  }
}

void ComputationGraph::PrintTiles()
{
  for(vector<Tile>::iterator t=tiles.begin(); t!=tiles.end(); t++){
    cout << endl << "tile" << t-tiles.begin() << ": " << endl;
    cout << t->pattern_name << endl;
    cout << t->cpsection_name << endl;
    for(vector<int>::iterator op=t->ops.begin(); op!=t->ops.end(); op++){
      cout << "op " << *op << ": ";
      for(vector<int>::iterator inop=ops[*op].in.begin(); inop!=ops[*op].in.end(); inop++){
	cout << *inop << " ";
      }
      cout << ops[*op].fu.name << endl;
    }
    cout << "live-in ops: " << endl;;
    for(map<int, int>::iterator op=t->livein_ops.begin(); op!=t->livein_ops.end(); op++){
      cout << op->first << " read at cycle " << op->second << endl;
    }
    cout << "pre tiles: ";
    for(vector<int>::iterator tile=t->pred_tiles.begin(); tile!=t->pred_tiles.end(); tile++){
      cout << *tile << " ";
    }
    cout << endl;
    for(vector<int>::iterator op=t->liveout_ops.begin(); op!=t->liveout_ops.end(); op++){
      cout << "live-out ops: " << *op << endl;
      cout << "   post tiles: ";
      for(vector<int>::iterator tile=t->post_tiles[*op].begin(); tile!=t->post_tiles[*op].end(); tile++){
	cout << *tile << " ";
      }
      cout << endl;
    }

  }
  cout << endl;
}

void ComputationGraph::SetOpOut()
{
  for(vector<Operation>::iterator op=ops.begin(); op!=ops.end(); op++){
    int op_idx = op-ops.begin();
    for(vector<int>::iterator inop_idx=op->in.begin(); inop_idx!=op->in.end(); inop_idx++){
      if(*inop_idx >= 0){
	if(ops[*inop_idx].out.find(op_idx) == ops[*inop_idx].out.end()){
	  ops[*inop_idx].out.insert(op_idx);
	}
      }
    }
  }
}


void ComputationGraph::TileGenPattern()
{
  cout << "Num of patterns " << patterns.size() << endl;
  for(vector<Pattern*>::iterator p=patterns.begin(); p!=patterns.end(); p++){
    if((*p)->name != "load_matrix"){
      cout << endl << "generate tile for " << (*p)->name << endl; 
      (*p)->TileGen(tiles, ops);
    }
  }
}

void ComputationGraph::BuildTileDep()
{
  //completing tile's information
  for(vector<Tile>::iterator tile=tiles.begin(); tile!=tiles.end(); tile++){
    //livein_ops & pre_tiles
    int livein_idx = 0;
    for(vector<int>::iterator op=tile->ops.begin(); op!=tile->ops.end(); op++){
      for(vector<int>::iterator inop=ops[*op].in.begin(); inop!=ops[*op].in.end(); inop++){
	if(*inop >= 0){
	  //inop is not inside the tile
	  int inop_is_intile = 0;
	  for(vector<int>::iterator op_t=tile->ops.begin(); op_t!=tile->ops.end(); op_t++){
	    if(*op_t == *inop){
	      inop_is_intile = 1;
	      break;
	    }
	  }

	  if(inop_is_intile){
	    continue;
	  }
	  else{
	    if(tile->livein_ops.find(*inop) == tile->livein_ops.end()){
	      tile->livein_ops[*inop] = tile->cb->LiveinReadCycle(livein_idx);
	    }
	    livein_idx++;			

	    int intile = ops[*inop].tile;
	    if(intile != -1){
	      vector<int>::iterator i;
	      for(i=tile->pred_tiles.begin(); i!=tile->pred_tiles.end(); i++){
		if(*i == intile)
		  break;
	      }
	      if(i == tile->pred_tiles.end())
		tile->pred_tiles.push_back(intile);
	    }
	  }
	}
	else{
	  //livein op is a constant
	  //dont actually be read
	  livein_idx++;            
	}
      }

      //liveout_ops & post_tiles
      for(set<int>::iterator outop=ops[*op].out.begin(); outop!=ops[*op].out.end(); outop++){
	int outop_is_intile = 0;
	for(vector<int>::iterator op_t=tile->ops.begin(); op_t!=tile->ops.end(); op_t++){
	  if(*op_t == *outop){
	    outop_is_intile = 1;
	    break;
	  }
	}

	if(outop_is_intile){
	  continue;
	}
	else{
	  vector<int>::iterator i;
	  for(i=tile->liveout_ops.begin(); i!=tile->liveout_ops.end(); i++){
	    if(*i == *op)
	      break;
	  }
	  if(i == tile->liveout_ops.end()){
	    tile->liveout_ops.push_back(*op);
	    vector<int> post_tiles_t;
	    tile->post_tiles[*op] = post_tiles_t;
	  }
	  int outtile = ops[*outop].tile;
	  if(outtile != -1){
	    for(i=tile->post_tiles[*op].begin(); i!=tile->post_tiles[*op].end(); i++){
	      if(*i == outtile)
		break;
	    }
	    if(i == tile->post_tiles[*op].end())
	      tile->post_tiles[*op].push_back(outtile);

	  }
	}
      }

    }


    //num_read
    for(int c=0; c<tile->latency; c++){
      tile->num_read.push_back(0);
    }
    for(map<int,int>::iterator in_op=tile->livein_ops.begin(); in_op!=tile->livein_ops.end(); in_op++){
      tile->num_read[in_op->second]++;
    }
  }
}

void ComputationGraph::TileGen()
{
  SetOpOut();
  TileGenPattern();
  BuildTileDep();
}

//---------------------------------------------------------------------------------------------------

void ComputationGraph::CP_Load(int m, int n, vector<vector<int>> &out_mtx)
{
  string cp = "CP: load matrix";

  Load_mtx* p1 = new Load_mtx(ops, cp, m, n, out_mtx);
  patterns.push_back(p1);
}

void ComputationGraph::CP_Store(const vector<vector<int>> &in_mtx)
{
  string cp = "CP: store matrix";

  Store_mtx* p1 = new Store_mtx(ops, cp, in_mtx);
  patterns.push_back(p1);
}

void ComputationGraph::CP_MtxMul(const vector<vector<int>> &in_mtx1, const vector<vector<int>> &in_mtx2, vector<vector<int>> &out_mtx)
{
  string cp = "CP: matrix multiplication";

  MtxMul* p1 = new MtxMul(ops, cp, in_mtx1, in_mtx2, out_mtx);
  patterns.push_back(p1);
}

void ComputationGraph::CP_MtxMulAdd(const vector<vector<int>> &in_mtx1, const vector<vector<int>> &in_mtx2, const vector<vector<int>> &in_mtx3, vector<vector<int>> &out_mtx){
  string cp = "CP: matrix multiplication and addition";

  MtxMulAdd* p1 = new MtxMulAdd(ops, cp, in_mtx1, in_mtx2, in_mtx3, out_mtx);
  patterns.push_back(p1);
}
/*
   void ComputationGraph::CP_NormalizeMtx(const vector<vector<int>> &in_mtx, vector<vector<int>> &out_mtx){
   string cp = "CP: normalize matrix";

//number of rows
int m = in_mtx.size();
//number of columns
int n = in_mtx[0].size();

//pattern1: sum of each column
vector<int> vec_sum;
ColSum p1(ops, cp, in_mtx, vec_sum);
patterns.push_back(p1);

vector<int> vec_const;
Vec_const(n, vec_const);	

//pattern2: mean of each column
vector<int> vec_mean;
Div_vec p2(ops, cp, vec_sum, vec_const, vec_mean);
patterns.push_back(p2);

vector<vector<int>> mtx_mean;
VecToMtx_horizontal(vec_mean, mtx_mean, m);

//pattern3: matrix sub by mean
vector<vector<int>> mtx_reduced_mean;
Sub_mtx p3(ops, cp, in_mtx, mtx_mean, mtx_reduced_mean);
patterns.push_back(p3);

//pattern4: sum of square of each column
vector<int> vec_squareacc;
SquareAcc_mtx p4(ops, cp, mtx_reduced_mean, vec_squareacc);
patterns.push_back(p4);

//pattern5: std of each column
vector<int> vec_std;
DivRoot p5(ops, cp, vec_squareacc, vec_const, vec_std);
patterns.push_back(p5);

vector<vector<int>> mtx_std;
VecToMtx_horizontal(vec_std, mtx_std, m);

//pattern6: matrix div by std
vector<vector<int>> mtx_normalized;
Div_mtx p6(ops, cp, mtx_reduced_mean, mtx_std, out_mtx);
patterns.push_back(p6);

//pattern9: correlation matrix
//vector<vector<int>> mtx_normalized_t;
//Transpose_mtx(mtx_normalized, mtx_normalized_t);
//MtxMul p9(ops, cp, mtx_normalized_t, mtx_normalized, out_mtx);
//patterns.push_back(p9);
}

void ComputationGraph::CP_CorrelMtx(const vector<vector<int>> &in_mtx, vector<vector<int>> &out_mtx){
string cp = "CP: correlation matrix";

//pattern1: correlation matrix
vector<vector<int>> in_mtx_tp;
Transpose_mtx(in_mtx, in_mtx_tp);
MtxMul p1(ops, cp, in_mtx_tp, in_mtx, out_mtx);
patterns.push_back(p1);
}


void ComputationGraph::CP_L2Norm(const vector<int> &in_vec, int &s_out)
{
string cp = "CP: l2 norm";

//pattern1: get square sum of the vector
int s_squaresum;
SquareAcc_vec p1(ops, cp, in_vec, s_squaresum);
patterns.push_back(p1);

//pattern2: scalar division
Root_s p2(ops, cp, s_squaresum, s_out);
patterns.push_back(p2);
}

void ComputationGraph::CP_HouseholderMtx(const vector<int> &in_vec, vector<vector<int>> &out_mtx)
{
  string cp = "CP: generate householder matrix";

  //cp: l2 norm
  int s_l2;
  CP_L2Norm(in_vec, s_l2);

  vector<int> vec_u;
  ReplaceLastEle(in_vec, s_l2, vec_u);

  //pattern1: square sum of vec_u
  int s_u;
  SquareAcc_vec p1(ops, cp, vec_u, s_u);
  patterns.push_back(p1);

  //pattern2: s_u divided by constant 2
  int s_h;
  Div_s p2(ops, cp, s_u, -1, s_h);
  patterns.push_back(p2);

  vector<int> vec_h;
  SToVec(s_h, vec_h, in_vec.size());

  //patter3: vec_u divided by vec_h
  vector<int> vec_uh;
  Div_vec p3(ops, cp, vec_u, vec_h, vec_uh);
  patterns.push_back(p3);

  vector<vector<int>> mtx_uh;
  VecToMtx_vertical(vec_uh, mtx_uh, 1);

  vector<vector<int>> mtx_u;
  VecToMtx_horizontal(vec_u, mtx_u, 1);

  //pattern4: mtxmul
  vector<vector<int>> mtx_z;
  MtxMul p4(ops, cp, mtx_uh, mtx_u, mtx_z);
  patterns.push_back(p4);

  //constant mtx: identity mtx
  vector<vector<int>> mtx_identity;
  Mtx_const(in_vec.size(), in_vec.size(), mtx_identity);	

  //pattern5: matrix substration
  Sub_mtx	p5(ops, cp, mtx_identity, mtx_z, out_mtx);
  patterns.push_back(p5);

}

void ComputationGraph::CP_OrthogonalTrans(const vector<vector<int>> &in_mtx, const vector<vector<int>> &mtx_p, vector<vector<int>> &out_mtx)
{
  string cp = "CP: orthogonal transformation";
  //pattern1: mtxmul
  vector<vector<int>> mtx1;
  MtxMul p1(ops, cp, in_mtx, mtx_p, mtx1);
  patterns.push_back(p1);

  vector<vector<int>> mtx_p_t;
  Transpose_mtx(mtx_p, mtx_p_t);

  //pattern2: mtxmul
  MtxMul p2(ops, cp, mtx_p_t, mtx1, out_mtx);
  patterns.push_back(p2);
}

void ComputationGraph::CP_JacobiMtx(vector<vector<int>> &in_mtx, int p, int q, vector<vector<int>> &out_mtx)
{
  string cp = "CP: generate jacobi mtx";

  int s_pp;
  int s_pq;
  int s_qq;
  GetElement(in_mtx, p, p, s_pp);
  GetElement(in_mtx, p, q, s_pq);
  GetElement(in_mtx, q, q, s_qq);

  //pattern 1
  //generate c,s
  int jacobi_c;
  int jacobi_s;
  int jacobi_ns;
  Jacobi_cs p1(ops, cp, s_pp, s_qq, s_pq, jacobi_c, jacobi_s, jacobi_ns);
  patterns.push_back(p1);

  Mtx_PlaneRotation(jacobi_c, jacobi_s, jacobi_ns, in_mtx.size(), p, q, out_mtx);
}

void ComputationGraph::CP_GivensMtx(vector<vector<int>> &in_mtx, int p, int q, int r, vector<vector<int>> &out_mtx)
{
  string cp = "CP: generate givens mtx";

  int s_pr;
  int s_qr;
  GetElement(in_mtx, p, r, s_pr);
  GetElement(in_mtx, q, r, s_qr);

  //pattern1
  //generate c,s
  int givens_c;
  int givens_s;
  int givens_ns;
  //need to zero apr
  Givens_cs p1(ops, cp, s_pr, s_qr, false, givens_c, givens_s, givens_ns);
  patterns.push_back(p1);

  Mtx_PlaneRotation(givens_c, givens_s, givens_ns, in_mtx.size(), p, q, out_mtx);
}

void ComputationGraph::CP_TridiagonalizeMtx(const vector<vector<int>> &in_mtx, vector<vector<int>> &out_mtx, vector<vector<int>> &out_mtx_z)
{
  string cp = "CP: tridiagonalize mtx";

  vector<vector<int>> mtx = in_mtx;
  vector<vector<int>> mtx_z;

  for(int i=in_mtx.size()-1; i>=2; i--){

    vector<int> vec_tozero;
    GetRowUnderDiag(mtx, vec_tozero, i);

    vector<vector<int>> mtx_householder;
    CP_HouseholderMtx(vec_tozero, mtx_householder);

    vector<vector<int>> mtx_p;
    Mtx_extend(mtx_householder, in_mtx.size(), mtx_p);


    //update matrix
    if(i==2){
      CP_OrthogonalTrans(mtx, mtx_p, out_mtx);
    }
    else{
      vector<vector<int>> mtx_temp;
      CP_OrthogonalTrans(mtx, mtx_p, mtx_temp);

      mtx = mtx_temp;
    }

    if(i == in_mtx.size()-1){
      mtx_z = mtx_p;
    }
    else if(i == 2){
      //pattern: update matrix z
      MtxMul p(ops, cp, mtx_z, mtx_p, out_mtx_z);
      patterns.push_back(p);
    }
    else{
      //pattern: update matrix z
      vector<vector<int>> mtx_z_temp;
      MtxMul p(ops, cp, mtx_z, mtx_p, mtx_z_temp);
      patterns.push_back(p);

      mtx_z = mtx_z_temp;
    }
  }
}


void ComputationGraph::CP_ZeroLastSubDiag(vector<vector<int>> &mtx, vector<vector<int>> &z, int row_idx)
{
  string cp = "CP: annihilate one sub-diag element";

  int p_jacobi = row_idx-1;
  int q_jacobi = row_idx;
  vector<vector<int>> mtx_jacobi;
  CP_JacobiMtx(mtx, p_jacobi, q_jacobi, mtx_jacobi);

  vector<vector<int>> mtx_temp;
  CP_OrthogonalTrans(mtx, mtx_jacobi, mtx_temp);
  mtx = mtx_temp;

  //pattern: matrix multiplication for z
  vector<vector<int>> z_temp;
  MtxMul p(ops, cp, z, mtx_jacobi, z_temp);
  patterns.push_back(p);	
  z = z_temp;

  if(row_idx >= 2){
    for(int i = row_idx-2; i >= 0; i--){
      int p_givens = i;
      int q_givens = i+1;
      int r_givens = i+2;
      vector<vector<int>> mtx_givens;
      CP_GivensMtx(mtx, p_givens, q_givens, r_givens, mtx_givens);

      vector<vector<int>> mtx_temp;
      CP_OrthogonalTrans(mtx, mtx_givens, mtx_temp);
      mtx = mtx_temp;

      //pattern: matrix multiplication for z
      vector<vector<int>> z_temp;
      MtxMul p(ops, cp, z, mtx_givens, z_temp);
      patterns.push_back(p);
      z = z_temp;
    }
  }
}


void ComputationGraph::CP_Diag(int max_unroll, const vector<vector<int>> &in_mtx, const vector<vector<int>> &in_mtx_z, vector<vector<int>> &out_mtx_z, vector<int> &out_diag){

  int n = in_mtx.size();

  vector<vector<int>> mtx = in_mtx;
  vector<vector<int>> mtx_z = in_mtx_z;

  for(int i=n-1; i>=1; i--){
    for(int c=0; c<max_unroll; c++){
      CP_ZeroLastSubDiag(mtx, mtx_z, i);
    }
  }

  GetDiag(mtx, out_diag);
  out_mtx_z = mtx_z;
}

void ComputationGraph::CP_PCA(const vector<vector<int>> &in_mtx, int k, vector<vector<int>> &out_mtx){
  vector<vector<int>> normal_mtx;
  CP_NormalizeMtx(in_mtx, normal_mtx);
  cout << "get cp: normalize mtx" << endl;	

  vector<vector<int>> cor_mtx;
  CP_CorrelMtx(normal_mtx, cor_mtx);
  cout << "get cp: correlation mtx" << endl;	

  vector<vector<int>> tridiag_mtx;
  vector<vector<int>> tridiag_mtx_z;
  CP_TridiagonalizeMtx(cor_mtx, tridiag_mtx, tridiag_mtx_z);
  cout << "get cp: tridiagonalize mtx" << endl;	

  vector<vector<int>> z_mtx;
  vector<int> diag;
  CP_Diag(1, tridiag_mtx, tridiag_mtx_z, z_mtx, diag);
  cout << "get cp: diagonalize mtx" << endl;	

  //get k columns from z_mtx;
  vector<vector<int>> eigen_mtx;
  GetColumns(z_mtx, k, eigen_mtx);
  CP_MtxMul(in_mtx, eigen_mtx, out_mtx);	
  cout << "get cp: mtx projection" << endl;	
}
*/

