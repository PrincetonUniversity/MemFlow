#ifndef COMPUTATIONGRAPH
#define COMPUTATIONGRAPH

#include<vector>
#include "Pattern.hpp"
#include "Util.hpp"
#include "Setting.hpp"

using namespace std;

class ComputationGraph{
  public:
    ComputationGraph(){};
    ~ComputationGraph();

    void CP_Load(int m, int n, vector<vector<int>> &out_mtx);
    void CP_Store(const vector<vector<int>> &in_mtx);

    void CP_MtxMul(const vector<vector<int>> &in_mtx1, const vector<vector<int>> &in_mtx2, vector<vector<int>> &out_mtx);
    void CP_MtxMulAdd(const vector<vector<int>> &in_mtx1, const vector<vector<int>> &in_mtx2, const vector<vector<int>> &in_mtx3, vector<vector<int>> &out_mtx);

    void CP_NormalizeMtx(const vector<vector<int>> &in_mtx, vector<vector<int>> &out_mtx);
    void CP_CorrelMtx(const vector<vector<int>> &in_mtx, vector<vector<int>> &out_mtx);	

    void CP_L2Norm(const vector<int> &in_vec, int &s_out);
    void CP_HouseholderMtx(const vector<int> &in_vec, vector<vector<int>> &out_mtx);
    void CP_OrthogonalTrans(const vector<vector<int>> &in_mtx, const vector<vector<int>> &mtx_p, vector<vector<int>> &out_mtx);
    void CP_JacobiMtx(vector<vector<int>> &in_mtx, int p, int q, vector<vector<int>> &out_mtx);
    void CP_GivensMtx(vector<vector<int>> &in_mtx, int p, int q, int r, vector<vector<int>> &out_mtx);
    void CP_TridiagonalizeMtx(const vector<vector<int>> &in_mtx, vector<vector<int>> &out_mtx, vector<vector<int>> &out_mtx_z);
    void CP_ZeroLastSubDiag(vector<vector<int>> &mtx, vector<vector<int>> &z, int row_idx);
    void CP_Diag(int max_unroll, const vector<vector<int>> &in_mtx, const vector<vector<int>> &in_mtx_z, vector<vector<int>> &out_mtx_z, vector<int> &out_diag);
    void CP_PCA(const vector<vector<int>> &in_mtx, int k, vector<vector<int>> &out_mtx);

    void PrintOps();
    void PrintTiles();
    void SetOpOut();
    void TileSize();
    void TileGenPattern();
    void BuildTileDep();
    void TileGen();
    void Scheduling();

    int m;
    int n;
    int k;

    vector<Operation> ops;
    vector<Pattern*> patterns;
    vector<Tile> tiles;

};

#endif