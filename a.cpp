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
 
//アームを表す木の構造体
//実装だるすぎて辺の重みは何も考慮していません->だるいけど考慮します...
struct Arm_tree{
    int sz; //頂点数
    vector<Edge> p; //木の順列表現
    vector<vector<Edge> > g; //木の隣接リスト表現
    vector<pair<int,int>> now; //各頂点の今の座標
    vector<string> op_history; //各回の操作を保存
    vector<bool> is_hand;

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

        is_hand.resize(sz,false);

        is_init = true;
    }

    //実際に操作を行う関数
    //cに入れた命令を行う (UDLR)
    //vに入っている関節を回す・頂点iの親を軸としてi以下の頂点を回す．
    //putに入っている頂点はPにする
    //この設計がいい感じかはまだ未検討
    void op(char c,vint &v,vint &put){
        string ops(2*sz,'.');
        
        //アーム全体を動かす処理
        pair<int,int> ps = {0,0};
        if(c == 'L') ps.second--;
        if(c == 'R') ps.second++;
        if(c == 'U') ps.first--;
        if(c == 'D') ps.first++;
        rep(i,sz){
            now[i] = make_pair(now[i].first+ps.first,now[i].second+ps.second);
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
                    assert(s[u][v] == '1');
                    s[u][v] = '0';
                }
                ops[sz+x] = 'P';
                is_hand[x] = !is_hand[x];
            }
        }

        op_history.push_back(ops);
    }

    //操作を行うとどの頂点がどこに行くかをシミュレートして座標を返す関数
    vector<pair<int,int> > sim_op(char c,vint &v){
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

    //各頂点の現在座標を出す関数
    void output_now(){
        rep(i,sz){
            if(i <= 9) cout << " ";
            cout << i << " -> (" << now[i].first << "," << now[i].second << ") leaf? = " << is_leaf(i) << endl;
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
        cout << 0 << " " << 0 << endl;
        output_history();
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


//テスト用・ローカルでしか回さない
void test(){
    rep(i,v)if(i){
        int tar = 2*((i+1)/2)-2;
        ans_tree.add_edge(i,tar);
    }
    ans_tree.init_for_arm();
}

//solve関数
//ver1 戦略
//・横一列の葉を使ってグリッド上を走査する
//・回転の頭いい使い方がわからないのでまずなしでやる
void solve(){
    rep(i,v)if(i){
        int tar = 2*((i+1)/2)-2;
        ans_tree.add_edge(i,tar);
    }
    ans_tree.init_for_arm();

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