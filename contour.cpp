// Check all pair of OCS node to see if they are pairs of alternatives
// according to some other nodes.
//


#define _DEBUG
#include <map>
#include <iostream>
#include <algorithm>
#include <graph.hpp>
#include "boost/accumulators/accumulators.hpp"
#include "boost/accumulators/statistics/stats.hpp"
#include "boost/accumulators/statistics/mean.hpp"
#include "boost/accumulators/statistics/moment.hpp"
#include "boost/accumulators/statistics/variance.hpp"
#include "boost/accumulators/statistics/count.hpp"
#include "boost/program_options.hpp"
#include "boost/program_options.hpp"


using namespace std;
using namespace boost::accumulators;

namespace po = boost::program_options;


int main(int argc, char** argv)
{
	string ifname;
	string tfname;
	string ofname;

	po::options_description desc("General Options");
	desc.add_options()
		("help", "produce help message")
		("input,I",po::value<string>(&ifname),"input file")
		("table,T",po::value<string>(&tfname),"lookup table file")
		("output,O",po::value<string>(&ofname),"output file");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help")) {
		cout<<desc<<"\n";
		return 0;
	}
	auto tabmx = map<int,float>();
	auto tabmy = map<int,float>();
	auto tabmr = map<int,float>();

	auto tabvx = map<int,float>();
	auto tabvy = map<int,float>();
	auto tabvr = map<int,float>();

	int from, to, fromto;
	float mx, my, mr;
	float vx, vy, vr;
	FILE *fin = fopen(ifname.c_str(),"r");
	FILE *ftable = fopen(tfname.c_str(),"r");
	FILE *fout=fopen(ofname.c_str(),"w");
	int i = 1;
	int ct;

	while (fscanf(ftable, "%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f", &fromto, &ct,&vx, &vy, &vr,
				&mx, &my, &mr)!=EOF)
	{
		tabmx[fromto]=mx;
		tabmy[fromto]=my;
		tabmr[fromto]=mr;
		tabvx[fromto]=vx;
		tabvy[fromto]=vy;
		tabvr[fromto]=vr;
		i++;
	}
	fclose(ftable);
	cout<<"there are "<<i<<" entries."<<endl;

	auto g = graph();
	while (fscanf(fin, "%d\t%d", &from,&to)!=EOF) {
		g.addNode(from);
		g.addNode(to);
		fromto = from*10000+to;
		edge aedge(from, to, tabmx[fromto],tabmy[fromto],tabmr[fromto],
				tabvx[fromto],tabvy[fromto],tabvr[fromto],false);
		edge bedge(to, from, -tabmx[fromto]/tabmr[fromto],-tabmy[fromto]/tabmr[fromto],1/tabmr[fromto],
				tabvx[fromto],tabvy[fromto],tabvr[fromto],true);
		g.nodes[from].addEdge(to,aedge);
		g.nodes[to].addEdge(from,bedge);
	}


	i=0;

	//Sort nodes index by degrees
	vector<int> nids(g.nodes.size());
	size_t j=0;
	for(auto it = g.nodes.begin();j<g.nodes.size();j++,it++){
		nids[j]=it->first;
	}
	sort(nids.begin(),nids.end(),
			[&g](const int&a,const int&b){
			return g.nodes[a].neighbors.size()>g.nodes[b].neighbors.size();
			});


	//set initial values
	set<int> visited;
	set<int> queue;
	map<int,vector<float>> result;

	queue.insert(nids[0]);
	while(queue.size()>0){
		int current = *queue.begin();
		visited.insert(current);
		queue.erase(queue.begin());
		if (current==nids[0]){
			g.nodes[nids[0]].scale=1;
		}else { 
			g.nodes[current].calculateScale(g.nodes);
		}
		vector<int> unvisited;
		copy_if(g.nodes[current].neighbors.begin(),
				g.nodes[current].neighbors.end(),back_inserter(unvisited),
				[&visited](const int& ai){
				return (visited.count(ai)==0);
				});
		cout<<"there are "<<unvisited.size()<<" unvisited neighbors"<<endl;
		queue.insert(unvisited.begin(),unvisited.end());
	}

	for(auto it=g.nodes.begin();it!=g.nodes.end();it++){
		result[it->first].push_back(it->second.scale);
	}
	for(int j=0;j<5;j++){
		for(auto it=nids.begin();it!=nids.end();it++){
			if(g.nodes[*it].scale!=0){
				g.nodes[*it].calculateScale(g.nodes);
			}
		}
		for(auto it=g.nodes.begin();it!=g.nodes.end();it++){
			result[it->first].push_back(it->second.scale);
		}

	}

	for(auto it=result.begin();it!=result.end();it++){
		fprintf(fout,"%d",it->first);
		for(auto it2=it->second.begin();it2!=it->second.end();it2++){
			fprintf(fout,"\t%f",*it2);
		}
		fprintf(fout,"\n");
	}

	cout << "there are "<<g.nodes.size()<<" node."<<endl;
	fclose(fin);
	fclose(fout);
	cout << "done." << endl;
}
