#include <bits/stdc++.h>
using namespace std;

/*
author : winter
for AHC038 2024-10-04
ver1.0
・アームを表すクラスをとりあえず作る (設計は詰めてない)
・木は片側のみのムカデグラフみたいにする
・移動もあまり頭いい方法ではない貪欲
・ここのメモ更新するのだるいので結構忘れそう
*/

#pragma GCC optimize("Ofast,unroll-loops,no-stack-protector,fast-math")
#pragma GCC target("avx2,bmi,bmi2,lzcnt,popcnt")
#pragma GCC target("sse,sse2,sse3,ssse3,sse4,popcnt,abm,mmx,avx,avx2,fma")
#define rep(i,n) for(int i=0;i<n;i++)
#define Rep(i,a,b) for(int i=a;i<b;i++)
#define ALL(x) (x).begin(),(x).end()
#define dbgv(x); for(auto now : x) cout << now << " "; cout << endl;
//using P = pair<int,int>;
using ll = long long;
using ull = unsigned long long;
//*/
template<class T> inline bool chmax(T& a, T b) { if (a < b) { a = b; return 1; } return 0; }
template<class T> inline bool chmin(T& a, T b) { if (a > b) { a = b; return 1; } return 0; }
typedef pair<int, int> pii;
typedef pair<ll, ll> pll;
typedef vector<ll> vll;
typedef vector<int> vint;
random_device rnd;
mt19937 rng(252521);
bool is_local = false;

//各種エラー・不具合検知用定数
const int OPERATION_SUCCESS    = 0;
const int OPERATION_LIMIT_OVER = -2521;
const int INVALID_MOVE         = -2522;

//Timerクラス (焼ける未来が見えないので使うか怪しい)
class Timer {
    public:
        Timer() {
            begin();
            elapsed_time_ = 0.0;
        }

        void begin() {
            start_time_ = chrono::system_clock::now();
        }

        double get_time() {
            chrono::system_clock::time_point end_time = chrono::system_clock::now();
            elapsed_time_ = chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time_).count();
            elapsed_time_ *= 1e-9; // nanoseconds -> seconds
            return elapsed_time_;
        }

        double get_last_time() const {
            return elapsed_time_;
        }

        bool yet(double time_limit) {
            return get_time() < time_limit;
        }

        double progress(double time_limit) {
            return get_time() / time_limit;
        }

    private:
        chrono::system_clock::time_point start_time_;
        double elapsed_time_;
};
constexpr double time_limit = 1.95;
Timer timer;

//各種変数のたまり場
int n,m,v;
vector<string> s,t;
bool is_input = false; //入力したか？の変数，使うかこれ？
const int first_v = 5; //頂点数5で頭いいアルゴリズム思いついたとき用，頂点多いほど良さそうなので使わなさそう
const int op_limit = 100000; //操作回数の上限値
//const int op_limit = 110; //操作回数の上限値 (デバッグ時)

//2座標のマンハッタン距離
int dist(pair<int,int> a,pair<int,int> b){
    return abs(a.first-b.first)+abs(a.second+b.second);
}

//座標a -> 座標bに移動するとき次の一手でどの向きに進むか
//現状アームを横に伸ばしているので上下移動でラッキー狙いをするため，上下移動優先 (ver1)
char dir(pair<int,int> a,pair<int,int> b){
    if(a.first < b.first) return 'D';
    if(a.first > b.first) return 'U';
    if(a.second < b.second) return 'R';
    if(a.second > b.second) return 'L';
    return '.';
}

//座標がn×nのマス目の中に含まれているか判定するための関数
//主に移動後の根が座標内に残っているか見るために使う
bool is_valid(pair<int,int> p){
    if(p.first < 0 || p.second < 0 || p.first >= n || p.second >= n) return false;
    else return true;
}

//辺の構造体・何も考えてない設計
struct Edge{
    int to = -1;
    int w;
};

//辺に付与した頂点の向きベクトルを回転させたもの
pair<int,int> rotate(pair<int,int> p,char c){
    if(c == 'L') return make_pair(-p.second,p.first);
    if(c == 'R') return make_pair(p.second,-p.first);
    return p;
}

//どこにたこ焼きがあるかなどを管理する構造体(というか関数の集合体)
struct Grid_info{

    //盤面が完成しきっているか判定
    bool is_clear(){
        rep(i,n)rep(j,n)if(s[i][j] == '1' || t[i][j] == '1') return false;
        return true;
    }

    //盤面上のたこ焼きの個数を返す
    int tako_count(){
        int res = 0;
        rep(i,n)rep(j,n){
            res += (s[i][j] == '1');
        }
        return res;
    }

    //盤面上の目的地の個数を返す
    int dest_count(){
        int res = 0;
        rep(i,n)rep(j,n){
            res += (t[i][j] == '1');
        }
        return res;
    }

    //x列目にたこ焼きがあるか
    bool exist_col(int x){
        int cnt = 0;
        rep(i,n) cnt += (s[i][x] == '1');
        return cnt > 0;
    }

    //たこやきの座標一覧を返す
    vector<pair<int,int> > tako_pos(){
        vector<pair<int,int> > res;
        rep(i,n)rep(j,n)if(s[i][j] == '1'){
            res.push_back(make_pair(i,j));
        }
        return res;
    }

    //目的地の座標一覧を返す
    vector<pair<int,int> > dest_pos(){
        vector<pair<int,int> > res;
        rep(i,n)rep(j,n)if(t[i][j] == '1'){
            res.push_back(make_pair(i,j));
        }
        return res;
    } 
};
 
//アームを表す木の構造体
//実装だるすぎて辺の重みは何も考慮していません->だるいけど考慮します...
struct Arm_tree{
    int sz; //頂点数
    vector<Edge> p; //木の順列表現
    vector<vector<Edge> > g; //木の隣接リスト表現
    vector<pair<int,int>> now; //各頂点の今の座標
    vector<string> op_history; //各回の操作を保存
    vector<vector<pair<int,int>> > edge_dir; //辺の向き
    vector<bool> is_hand; //今たこ焼きを持っているか

    bool is_init = false;
    Arm_tree(int sz = 1) : sz(sz),p(sz),g(sz) {};

    //辺の追加をする関数
    //u-vに重みwの辺を張る，initしてからadd_edgeしたら怒る
    void add_edge(int u,int v,int w=1){
        
        if(u > v) swap(u,v);
        if(p.size() <= v) p.resize(v+1),g.resize(v+1),sz = v+1;

        //未定義動作の類をはじいとく
        if(u == v){
            cerr << "You may add edge for same vertex" << endl;
            assert(u != v);
        }
        if(p[v].to != -1){
            if(is_init) cerr << "You can't add edge after initialize" << endl;
            cerr << "This edge break the tree" << endl;
            assert(p[v].to == -1);
        }

        p[v] = {u,w};
        g[u].push_back((Edge){v,w});
        //g[v].push_back((Edge){u,w});
    }

    //木を作った後に初期化をする関数
    //各頂点の初期位置もここで算出
    void init_for_arm(){
        assert(sz == p.size());
        assert(sz == g.size());
        bool ok = true;
        edge_dir.resize(sz,vector<pair<int,int>>(sz,make_pair(-1,-1)));

        rep(i,sz)if(g[i].size() == 0) ok = false;
        if(!ok){
            cerr << "This tree has a independent vertex" << endl;
            assert(ok);
        }

        //DFSして各頂点の座標を求める
        const pair<int,int> default_coord = make_pair(-1,-1);
        now.resize(sz,default_coord);
        queue<int> q;
        q.push(0);
        now[0] = make_pair(0,0);
        while(!q.empty()){
            int nxt = q.front();
            q.pop();
            for(auto [to,w] : g[nxt])if(now[to] == default_coord){
                q.push(to);
                now[to] = make_pair(now[nxt].first,now[nxt].second+w);
            }
        }

        is_hand.resize(sz,false);

        is_init = true;
    }

    //実際に操作を行う関数
    //cに入れた命令を行う (UDLR)
    //vに入っている関節を回す・頂点iの親を軸としてi以下の頂点を回す．
    //putに入っている頂点はPにする
    //この設計がいい感じかはまだ未検討
    int op(char c,vector<int,char> &v,vint &put){
        assert(is_init);
        string ops(2*sz,'.');
        
        //アーム全体を動かす処理
        pair<int,int> ps = {0,0};
        if(c == 'L') ps.second--;
        if(c == 'R') ps.second++;
        if(c == 'U') ps.first--;
        if(c == 'D') ps.first++;
        rep(i,sz){
            now[i].first += ps.first;
            now[i].second += ps.second;
        }

        //移動後に根が座標外に出るような操作は受け付けない
        if(!is_valid(now[0])){
            rep(i,sz){
                now[i].first -= ps.first;
                now[i].second -= ps.second;
            }
            return INVALID_MOVE;
        }
        ops[0] = c;

        //うまく回転させる処理普通にわからないんですが...

        //putの座標でつかむ処理
        for(int x : put){
            if(!is_leaf(x)){
                cerr << "You operated [put] for not leaf vertex" << endl;
            }else{
                if(is_hand[x]){
                    auto [u,v] = now[x];
                    assert(s[u][v] != '1');
                    if(t[u][v] == '1') t[u][v] = '0';
                    else s[u][v] = '1';
                }else{
                    auto [u,v] = now[x];
                    //assert(s[u][v] == '1');
                    s[u][v] = '0';
                }
                ops[sz+x] = 'P';
                is_hand[x] = !is_hand[x];
            }
        }

        if(op_history.size() < op_limit){
            op_history.push_back(ops);
        }
        else return OPERATION_LIMIT_OVER;

        return OPERATION_SUCCESS;
    }

    //操作を行うとどの頂点がどこに行くかをシミュレートして座標を返す関数
    //実際にアームを動かすわけではないことに注意
    vector<pair<int,int> > sim_op(char c,vector<int,char> &v){
        vector<pair<int,int> > res = now;

        //アーム全体を動かす処理
        pair<int,int> ps = {0,0};
        if(c == 'L') ps.second--;
        if(c == 'R') ps.second++;
        if(c == 'U') ps.first--;
        if(c == 'D') ps.first++;
        rep(i,sz){
            res[i] = make_pair(now[i].first+ps.first,now[i].second+ps.second);
        }

        //回転はまだうまくできていないので略
        return res;
    }

    //頂点vは葉ですか？
    bool is_leaf(int v){
        return g[v].size() == 1;
    }

    //葉の頂点リストを返す
    vector<int> leafs(){
        vint res;
        rep(i,sz)if(is_leaf(i)) res.push_back(i);
        return res;
    }

    //葉の今いる座標とともにリストを返す
    vector<pair<int,pair<int,int>>> leaf_pos(){
        vector<pair<int,pair<int,int>>> res;
        auto l = leafs();
        for(int v : l) res.push_back(make_pair(v,now[v]));
        return res;
    }

    //葉でフリーなもののうち一番先頭の番号と座標を返す
    pair<int,pair<int,int>> free_leaf_top(){
        auto l = leaf_pos();
        for(auto p : l)if(!is_hand[p.first]) return p;
        return make_pair(-1,make_pair(-1,-1));
    }

    //葉でフリーでないもののうち一番先頭の番号と座標を返す
    pair<int,pair<int,int>> nfree_leaf_top(){
        auto l = leaf_pos();
        for(auto p : l)if(is_hand[p.first]) return p;
        return make_pair(-1,make_pair(-1,-1));
    }

    //各頂点の現在座標を出す関数
    //エラー出力
    void output_now(){
        rep(i,sz){
            if(i <= 9) cout << " ";
            cerr << i << " -> (" << now[i].first << "," << now[i].second << ") leaf? = " << is_leaf(i) << endl;
        }
    }

    void output_p(){}

    //木をよく見るグラフ表記で出力する関数
    void output_g(){
        cout << sz << " " << sz-1 << endl;
        rep(i,sz)if(i) cout << p[i].to << " " << i << endl;
    }

    //今までの操作列を出力
    void output_history(){
        for(string s : op_history){
            assert(s.size() == 2*sz);
            cout << s << endl;
        }
    }

    //AHCの提出時に必要な情報を出力する
    void output(){
        cout << sz << endl;
        rep(i,sz)if(i){
            cout << p[i].to << " " << p[i].w << endl;
        }
        //開始座標を原点に固定してることに注意(ver1)
        cout << 0 << " " << 0 << endl;
        output_history();
    }
};

//答えの木を保持するやつをグローバルに確保
Arm_tree ans_tree;
Grid_info g;

//入力を受け取る
void input(){
    is_input = true;
    cin >> n >> m >> v;
    s.resize(n);
    t.resize(n);
    rep(i,n) cin >> s[i];
    rep(i,n) cin >> t[i];

    //既に置いてあるときは空欄にしておく
    rep(i,n)rep(j,n){
        if(s[i][j] == '1' && s[i][j] == t[i][j]){
            s[i][j] = t[i][j] = '0';
        }
    }
}


//テスト用・ローカルでしか回さない
void test(){
    rep(i,v)if(i){
        int tar = 2*((i+1)/2)-2;
        ans_tree.add_edge(i,tar);
    }
    ans_tree.init_for_arm();
}

//ver1 戦略
//横一列の葉を使ってグリッド上を走査する
//たこやきの取得・設置も地獄の愚直をする
//回転の頭いい使い方がわからないのでまずなしでやる
Arm_tree solve_1(){
    //初期化と片ムカデグラフを作る
    Arm_tree res;
    rep(i,v)if(i){
        int tar = 2*((i+1)/2)-2;
        res.add_edge(i,tar);
    }
    res.init_for_arm();

    auto vs = res.leaf_pos();
    const int INF = 252521;
    while(!g.is_clear()){
        int tako_dist = INF;
        int dest_dist = INF;
        char tako_dir = '.';
        char dest_dir = '.';
        char op_dir  = '.';
        auto f_pos = res.free_leaf_top();
        auto nf_pos = res.nfree_leaf_top();
        vector<int,char> cir;
        vint put;

        //一番近いたこ焼きを拾う処理
        if(g.tako_count() > 0){
            auto tako_pos = g.tako_pos();
            assert(tako_pos.size() > 0);
            sort(ALL(tako_pos),[&](auto i,auto j){return dist(f_pos.second,i) < dist(f_pos.second,j);});
            tako_dist = dist(f_pos.second,tako_pos[0]);
            tako_dir = dir(f_pos.second,tako_pos[0]);
        }

        //一番近い目的地に向かう処理
        if(g.dest_count() > 0){ //完成より前に目的地がなくなることはないため，ここは必ず通る想定
            auto dest_pos = g.dest_pos();
            assert(dest_pos.size() > 0);
            sort(ALL(dest_pos),[&](auto i,auto j){return dist(nf_pos.second,i) < dist(nf_pos.second,j);});
            dest_dist = dist(nf_pos.second,dest_pos[0]);
            dest_dir = dir(nf_pos.second,dest_pos[0]);
        }

        //盤面が完成していないとき，どちらかは必ず更新される．
        assert(tako_dist != INF || dest_dist != INF);

        if(tako_dist <= dest_dist){ //たこ焼き優先の分岐
            op_dir = tako_dir;
        }
        else if(tako_dist > dest_dist){ //目的地優先の分岐
            op_dir = dest_dir;
        }else{
            bool THIS_SECTION_IS_NOT_USE = false;
            //assert(THIS_SECTION_IS_NOT_USE);
        }

        if(f_pos.first == -1){ //たこ焼きをもう持てない
            op_dir = dest_dir;
        }
        else if(nf_pos.first == -1){ //置くためのたこ焼きがない
            op_dir = tako_dir;
        }

        auto sim_pos = res.sim_op(op_dir,cir);
        rep(i,res.sz)if(res.is_leaf(i)){
            auto [x,y] = sim_pos[i];
            if(x < 0 || y < 0 || x >= n || y >= n) continue;
            //cerr << res.now[i].first << " " << res.now[i].second << " " << op_dir << endl;
            //cerr << x << " " << y << endl;
            if(s[x][y] == '1' && !res.is_hand[i]) put.push_back(i);
            if(t[x][y] == '1' && res.is_hand[i]) put.push_back(i);
        }

        /*/
        cerr << "op = " << op_dir << endl;
        cerr << "f_pos = " << f_pos.first << " pos = (" << f_pos.second.first << "," << f_pos.second.second << ")" << endl;
        cerr << "nf_pos = " << nf_pos.first << " pos = (" << nf_pos.second.first << "," << nf_pos.second.second << ")" << endl;
        cerr << "tako_dist = " << tako_dist << " dest_dist = " << dest_dist << endl;
        cerr << "tako_dir = " << tako_dir << " dest_dir = " << dest_dir << endl; 
        rep(i,n){
            cerr << s[i] << endl;
        }
        //*/

        if(!res.op(op_dir,cir,put)) break;
    }

    return res;
}

//solve関数
void solve(){
    ans_tree = solve_1();
}

void output(){
    ans_tree.output();
}

void dbg(){
}

int main() {
    ios::sync_with_stdio(false);
	cin.tie(nullptr);
    input();
    //test();
    solve();
    output();

    /*
    提出前に正しい関数だけ残してるか確認！！
    以下のis_localもちゃんとしてるか確認！！
    */

    //is_local = true;
    if(is_local) dbg();
}