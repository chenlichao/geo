#include<set>
#include<algorithm>


using namespace std;


class edge {
	public:
		edge()=default;
		edge(int af,int at,float amx,float amy,float amr,
				float avx, float avy, float avr,bool derived):
			from ( af), to ( at), mx ( amx), my ( amy),
			mr ( amr),vx(avx),vy(avy)
	{
		isDerived = derived;
		if(!derived){
			vr = sqrt(avr)/mr;
		} else{
			vr = avr;
		}
	};
	public:
		int from; 
		int to;
		float mx,my,mr;
		float vx,vy,vr;
		bool isDerived;
};
class node {
	public:
		node()=default;
		node(int aid):id(aid){
			edges= map<int, edge>();
			neighbors=set<int>();
		};
		void addEdge(int bid, const edge &aedge){
			edges[bid]=aedge;
			neighbors.insert(bid);
		}
		set<int> inter(const node& b){
			set<int> res;
			set_intersection(neighbors.begin(), neighbors.end(),
					b.neighbors.begin(),b.neighbors.end(),
					inserter(res,res.end()));
			return res;
		}

	public:
		map<int, edge> edges;
		set<int> neighbors;
		int id;
};

class graph{
	public:
		graph(){
			nodes = map<int, node>();
		};
		void addNode(int aid){
			if (nodes.count(aid)==0)
				nodes[aid]=node(aid);
		}

	public:
		map<int, node> nodes;
};
