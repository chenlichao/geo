#include <algorithm>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <limits>
#include <map>
#include <set>

using namespace std;
using namespace boost::accumulators;

class edge {
public:
  edge() = default;
  edge(int af, int at, float amx, float amy, float amr, float avx, float avy,
       float avr, bool derived)
      : from(af), to(at), mx(amx), my(amy), mr(amr), vx(avx), vy(avy) {
    isDerived = derived;
    if (!derived) {
      vr = sqrt(avr) / mr;
    } else {
      vr = avr;
    }
  };

public:
  int from;
  int to;
  float mx, my, mr;
  float vx, vy, vr;
  bool isDerived;
};

class node {
public:
  node() = default;
  node(int aid) : id(aid) {
    edges = map<int, edge>();
    neighbors = set<int>();
  };
  void addEdge(int bid, const edge &aedge) {
    edges[bid] = aedge;
    neighbors.insert(bid);
  }
  void setThreshold(float tx, float ty, float ts) {
    thX = tx;
    thY = ty;
    thS = ts;
  }
  bool isNoise() { return isNoise(varX, varY, varS); }
  bool isNoise(float vx, float vy, float vs) {
    if ((thX == 0 || vx < thX) && (thY == 0 || vy < thY) &&
        (thS == 0 || vs < thS)) {
      return false;
    } else {
      return true;
    }
  }

  set<int> inter(const node &b) {
    set<int> res;
    set_intersection(neighbors.begin(), neighbors.end(), b.neighbors.begin(),
                     b.neighbors.end(), inserter(res, res.end()));
    return res;
  }
  void calculateScale(map<int, node> &nodes) {
    vector<int> assigned;
    copy_if(neighbors.begin(), neighbors.end(), back_inserter(assigned),
            [&nodes](const int &i) { return nodes[i].scale != 0; });
    float s = 0;
    float x = 0;
    float y = 0;

    accumulator_set<float, features<tag::variance>> accx;
    accumulator_set<float, features<tag::variance>> accy;
    accumulator_set<float, features<tag::variance>> accs;

    for (auto it = assigned.begin(); it != assigned.end(); it++) {
      float ts = nodes[*it].edges[id].mr * nodes[*it].scale;
      s += ts;
      float tx = nodes[*it].edges[id].mx * nodes[*it].scale + nodes[*it].absx;
      x += tx;
      float ty = nodes[*it].edges[id].my * nodes[*it].scale + nodes[*it].absy;
      y += ty;

      accx(tx);
      accy(ty);
      accs(ts);
    }
    s /= assigned.size();
    x /= assigned.size();
    y /= assigned.size();
    scale = s;
    absx = x;
    absy = y;
    varX = variance(accx);
    varY = variance(accy);
    varS = variance(accs);
    noise = isNoise();
#ifdef _DEBUG
    cout << "nodes " << id << " scale assigned: " << s << endl;
#endif
  }

public:
  map<int, edge> edges;
  set<int> neighbors;
  float scale = 0;
  float absx = 0;
  float absy = 0;
  bool noise = true;
  int id;
  float varX = 0;
  float varY = 0;
  float varS = 0;
  float thX = 0;
  float thY = 0;
  float thS = 0;
};

class graph {
public:
  graph() { nodes = map<int, node>(); };
  void addNode(int aid) {
    if (nodes.count(aid) == 0)
      nodes[aid] = node(aid);
  }

  void addNode(int aid, float fx, float fy, float fs) {
    if (nodes.count(aid) == 0) {
      nodes[aid] = node(aid);
      nodes[aid].setThreshold(fx, fy, fs);
    }
  }

public:
  map<int, node> nodes;
};
