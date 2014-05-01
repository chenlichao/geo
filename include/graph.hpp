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
		void calculateScale(map<int,node>& nodes){
			vector<int> assigned;
			copy_if(neighbors.begin(),neighbors.end(),
					back_inserter(assigned),[&nodes](const int& i){
					return nodes[i].scale!=0;
					});
			float s=0;
			for(auto it=assigned.begin();it!=assigned.end();it++){
				s+=nodes[*it].edges[id].mr * nodes[*it].scale;
			}
			s/=assigned.size();
			scale = s;
#ifdef _DEBUG
			cout<<"nodes "<<id<<" scale assigned: "<<s<<endl;
#endif
		}
	public:
		map<int, edge> edges;
		set<int> neighbors;
		float scale;
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
