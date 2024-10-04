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

//辺の構造体・何も考えてない設計
struct Edge{
    int to = -1;
    int w;
};
 
struct Arm_tree{
    //アームを表す木の構造体
    //実装だるすぎて辺の重みは何も考慮していません->だるいけど考慮します...

    int sz; //頂点数・辺数
    vector<Edge> p; //木の順列表現
    vector<vector<Edge> > g; //木の隣接リスト表現
    vector<pair<int,int>> now; //各頂点の今の座標

    bool is_init = false;
    Arm_tree(int sz = 0) : sz(sz),p(sz),g(sz) {};

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
        g[v].push_back((Edge){u,w});

    }

    //木を作った後に初期化をする関数
    //各頂点の初期位置もここで算出
    void init_for_arm(){
        assert(sz == p.size());
        assert(sz == g.size());
        bool ok = true;

        rep(i,sz)if(g[i].size() == 0) ok = false;
        if(!ok){
            cerr << "This tree has a independent vertex" << endl;
            assert(!ok);
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

        is_init = true;
    }

    //頂点vは葉ですか？
    bool is_leaf(int v){
        return g[v].size() == 1;
    }

    //各頂点の現在座標を出す関数
    void output_now(){
        rep(i,sz) cout << i << " -> (" << now[i].first << "," << now[i].second << ")" << endl;
    }

    void output_p(){}

    //木をよく見るグラフ表記で出力する関数
    void output_g(){
        cout << sz << " " << sz-1 << endl;
        rep(i,sz)if(i) cout << p[i].to << " " << i << endl;
    }

};

//答えの木を保持するやつをグローバルに確保
Arm_tree ans_tree;

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

void test(){
    rep(i,v)if(i){
        int tar = 2*((i+1)/2)-2;
        ans_tree.add_edge(i,tar);
    }
    ans_tree.init_for_arm();
}

//
void solve(){
    ans_tree.add_edge(0,1);
}

void output(){
    ans_tree.output_g();
    ans_tree.output_now();
    
}

void dbg(){
}

int main() {
    ios::sync_with_stdio(false);
	cin.tie(nullptr);
    input();
    test();
    output();
    //is_local = true;
    if(is_local) dbg();
}