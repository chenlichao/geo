#include<set>
#include<limits>
#include<map>
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
			float x=0;
			float y=0;
			float maxs=numeric_limits<float>::min();//or lowest() in c++11?
			float mins=numeric_limits<float>::max();

			for(auto it=assigned.begin();it!=assigned.end();it++){
				float ts =nodes[*it].edges[id].mr * nodes[*it].scale; 
				s+=ts;
				x+=nodes[*it].edges[id].mx * nodes[*it].scale + nodes[*it].absx;
				y+=nodes[*it].edges[id].my * nodes[*it].scale + nodes[*it].absy;
				if (ts>maxs)
				{
					maxs=ts;
				}
				if (ts<mins)
				{
					mins =ts;
				}		
			}
			s/=assigned.size();
			x/=assigned.size();
			y/=assigned.size();
			scale = s;
			absx = x;
			absy = y;
			if (maxs-mins>0.1){
				noise = true;
			}else{
				noise = false;
			}
#ifdef _DEBUG
			cout<<"nodes "<<id<<" scale assigned: "<<s<<endl;
#endif
		}
	public:
		map<int, edge> edges;
		set<int> neighbors;
		float scale=0;
		float absx=0;
		float absy=0;
		bool noise=true;
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
