// Check all pair of OCS node to see if they are pairs of alternatives
// according to some other nodes.
//


#define _DEBUG
#include <map>
#include <iostream>
#include <fstream>
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
	string oxfname;
	string oyfname;
	string onfname;
	string config;
	float thresx=0;
	float thresy=0;
	float thress=0;

	po::options_description desc("General Options");
	desc.add_options()
		("help", "produce help message")
		("config,C",po::value<string>(&config),"configuration file")
		("input,I",po::value<string>(&ifname),"input file")
		("table,T",po::value<string>(&tfname),"lookup table file")
		("output,O",po::value<string>(&ofname),"output file")
		("xoutput,X",po::value<string>(&oxfname),"x output file")
		("youtput,Y",po::value<string>(&oyfname),"y output file")
		("noutput,N",po::value<string>(&onfname),"isnoisy output file")
		("thresx",po::value<float>(&thresx),"threshold x")
		("thresy",po::value<float>(&thresy),"threshold y")
		("thress",po::value<float>(&thress),"threshold s");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help")) {
		cout<<desc<<"\n";
		return 0;
	} else if (vm.count("config")) {
		cout << config << endl;
		ifstream fconf(config);
		po::store(po::parse_config_file(fconf, desc), vm);
		po::notify(vm);
		fconf.close();
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
	FILE *foutx=fopen(oxfname.c_str(),"w");
	FILE *fouty=fopen(oyfname.c_str(),"w");
	FILE *foutn=fopen(onfname.c_str(),"w");
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

	//Load graph
	auto g = graph();
	while (fscanf(fin, "%d\t%d", &from,&to)!=EOF) {
		g.addNode(from,thresx,thresy,thress);
		g.addNode(to,thresx,thresy,thress);
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
	map<int,vector<float>> resultx;
	map<int,vector<float>> resulty;
	map<int,vector<bool>> resultn;

	queue.insert(nids[0]);
	while(queue.size()>0){
		int current = *queue.begin();
		visited.insert(current);
		queue.erase(queue.begin());
		if (current==nids[0]){
			g.nodes[nids[0]].scale=1;
			g.nodes[nids[0]].absx=0;
			g.nodes[nids[0]].absy=0;
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
		resultx[it->first].push_back(it->second.absx);
		resulty[it->first].push_back(it->second.absy);
		resultn[it->first].push_back(it->second.noise);
	}
	for(int j=0;j<10;j++){
		for(auto it=nids.begin();it!=nids.end();it++){
			if(g.nodes[*it].scale!=0){
				g.nodes[*it].calculateScale(g.nodes);
				/*
				if (*it==124){
					cout<<"Node 124 Variance:\tx: "<<g.nodes[*it].varX
						<<"\ty: "<<g.nodes[*it].varY<<"\t s: "
						<<g.nodes[*it].varS<<endl;

				}
				*/
			}
		}
		for(auto it=g.nodes.begin();it!=g.nodes.end();it++){
			result[it->first].push_back(it->second.scale);
			resultx[it->first].push_back(it->second.absx);
			resulty[it->first].push_back(it->second.absy);
			resultn[it->first].push_back(it->second.noise);
		}

	}

	for(auto it=result.begin();it!=result.end();it++){
		fprintf(fout,"%d",it->first);
		for(auto it2=it->second.begin();it2!=it->second.end();it2++){
			fprintf(fout,"\t%f",*it2);
		}
		fprintf(fout,"\n");
	}

	for(auto it=resultx.begin();it!=resultx.end();it++){
		fprintf(foutx,"%d",it->first);
		for(auto it2=it->second.begin();it2!=it->second.end();it2++){
			fprintf(foutx,"\t%f",*it2);
		}
		fprintf(foutx,"\n");
	}
	for(auto it=resulty.begin();it!=resulty.end();it++){
		fprintf(fouty,"%d",it->first);
		for(auto it2=it->second.begin();it2!=it->second.end();it2++){
			fprintf(fouty,"\t%f",*it2);
		}
		fprintf(fouty,"\n");
	}
	for(auto it=resultn.begin();it!=resultn.end();it++){
		fprintf(foutn,"%d",it->first);
		for(auto it2=it->second.begin();it2!=it->second.end();it2++){
			fprintf(foutn,"\t%s",(*it2)?"true":"false");
		}
		fprintf(foutn,"\n");
	}

	cout << "there are "<<g.nodes.size()<<" node."<<endl;
	fclose(fin);
	fclose(fout);
	fclose(foutx);
	fclose(fouty);
	fclose(foutn);
	cout << "done." << endl;
}
