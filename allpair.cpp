// Check all pair of OCS node to see if they are pairs of alternatives
// according to some other nodes.

#include "boost/accumulators/accumulators.hpp"
#include "boost/accumulators/statistics/count.hpp"
#include "boost/accumulators/statistics/mean.hpp"
#include "boost/accumulators/statistics/moment.hpp"
#include "boost/accumulators/statistics/stats.hpp"
#include "boost/accumulators/statistics/variance.hpp"
#include "boost/program_options.hpp"
#include <fstream>
#include <graph.hpp>
#include <iostream>
#include <map>

using namespace std;
using namespace boost::accumulators;

namespace po = boost::program_options;

int main(int argc, char **argv) {
  string ifname;
  string tfname;
  string ofname;
  string efname;
  string nettype;
  string config; // configuration file
  float thre1;
  float thre2;
  float thre3;

  // clang-format off

  po::options_description desc("General Options");
  desc.add_options()
	  ("help", "produce help message")
	  ( "input,I", po::value<string>(&ifname), "input file")
	  ("table,T", po::value<string>(&tfname), "lookup table file")
	  ("network,N", po::value<string>(&nettype), "network type:dup or alt")
	  ( "config,C", po::value<string>(&config), "configuration file")
	  ( "threshold1", po::value<float>(&thre1), "Threshold 1")
	  ("threshold2", po::value<float>(&thre2), "Threshold 2")
	  ( "threshold3", po::value<float>(&thre3), "Threshold 3")
	  ("output,O", po::value<string>(&ofname), "output file")
	  ( "eloutput,E", po::value<string>(&efname), "edge list output file");
  // clang-format on

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    cout << desc << "\n";
    return 0;
  } else if (vm.count("config")) {
    cout << config << endl;
    ifstream fconf(config);
    po::store(po::parse_config_file(fconf, desc), vm);
    po::notify(vm);
    fconf.close();
  }

  if (vm.count("threshold1")) {
    cout << "threshold1 = " << thre1 << "\n";
  }
  if (vm.count("threshold2")) {
    cout << "threshold2 = " << thre2 << "\n";
  }
  if (vm.count("threshold3")) {
    cout << "threshold3 = " << thre3 << "\n";
  }
  auto tabmx = map<int, float>();
  auto tabmy = map<int, float>();
  auto tabmr = map<int, float>();

  auto tabvx = map<int, float>();
  auto tabvy = map<int, float>();
  auto tabvr = map<int, float>();

  int from, to, fromto;
  float mx, my, mr;
  float vx, vy, vr;
  FILE *fin = fopen(ifname.c_str(), "r");
  FILE *ftable = fopen(tfname.c_str(), "r");
  FILE *fout = fopen(ofname.c_str(), "w");
  FILE *fel = fopen(efname.c_str(), "w");
  int i = 0;
  int ct;
  // read edge arguments table
  while (fscanf(ftable, "%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f", &fromto, &ct, &vx,
                &vy, &vr, &mx, &my, &mr) != EOF) {
    tabmx[fromto] = mx;
    tabmy[fromto] = my;
    tabmr[fromto] = exp(mr);
    tabvx[fromto] = vx;
    tabvy[fromto] = vy;
    tabvr[fromto] = vr;
    i++;
  }
  fclose(ftable);

  cout << "there are " << i << " entries." << endl;

  // read edge list
  auto g = graph();
  while (fscanf(fin, "%d\t%d", &from, &to) != EOF) {
    g.addNode(from);
    g.addNode(to);
    fromto = from * 10000 + to;
    edge aedge(from, to, tabmx[fromto], tabmy[fromto], tabmr[fromto],
               tabvx[fromto], tabvy[fromto], tabvr[fromto], false);
    edge bedge(to, from, -tabmx[fromto] / tabmr[fromto],
               -tabmy[fromto] / tabmr[fromto], 1 / tabmr[fromto], tabvx[fromto],
               tabvy[fromto], tabvr[fromto], true);
    g.nodes[from].addEdge(to, aedge);
    g.nodes[to].addEdge(from, bedge);
  }

  i = 0;
  if (nettype == "alt") {
    cout << "createing alternative parts network" << endl;
    for (auto it = g.nodes.begin(); it != g.nodes.end(); ++it)
      for (auto it2 = next(it); it2 != g.nodes.end(); ++it2) {
        // if (it->second.neighbors.count(it2->second.id)>0)
        //	continue;
        // else{
        auto temp = it->second.inter(it2->second);
        if (temp.size() > 0) {
          int displayed = 0;
          int eldisplayed = 0;
          int tc = 0;
          for (auto itk = temp.begin(); itk != temp.end(); itk++) {
            auto e1 = it->second.edges[*itk];
            auto e2 = it2->second.edges[*itk];

            auto delta_x =
                -e1.mx / e1.mr + e2.mx / e2.mr + 0.5 / e1.mr - 0.5 / e2.mr;
            auto delta_y =
                -e1.my / e1.mr + e2.my / e2.mr + 0.5 / e1.mr - 0.5 / e2.mr;

            // auto delta_x = -e1.mx/e1.mr +e2.mx/e2.mr;
            // auto delta_y = -e1.my/e1.mr +e2.my/e2.mr;

            auto l1 = std::max(abs(delta_x), abs(delta_x * e1.mr)) +
                      std::max(abs(delta_y), abs(delta_y * e1.mr));

            // if(it->first==709||it2->first==709||(l1<thre1&&
            if (l1 < thre1 && abs(log(e1.mr) - log(e2.mr)) < thre3) {
              tc++;
              if (!displayed) {
                displayed = 1;
                fprintf(fout, "%d<->%d:\n", it->first, it2->first);
              }
              if (tc > 1 && !eldisplayed) {
                i++;
                eldisplayed = 1;
                fprintf(fel, "%d\t%d\n", it->first, it2->first);
              }
              fprintf(fout, "%d:\t%f,%f\t%f,%f\t%f,%f\n", *itk, e1.mx, e2.mx,
                      e1.my, e2.my, e1.mr, e2.mr);
            }
          }
        }
        //}
      }

    cout << "there are " << g.nodes.size() << " node." << endl;
    cout << "there are " << i << " pairs." << endl;
  } else if (nettype == "dup") {
    cout << "createing duplicate parts network" << endl;
    for (auto it = g.nodes.begin(); it != g.nodes.end(); ++it)
      for (auto it2 = next(it); it2 != g.nodes.end(); ++it2) {
        if (it->second.neighbors.count(it2->second.id) <= 0)
          continue;
        else {
          auto temp = it->second.inter(it2->second);
          auto intersize = temp.size();
          auto size1 = it->second.neighbors.size();
          auto size2 = it2->second.neighbors.size();
          if (intersize > size1 * thre1 && intersize > size2 * thre1) {
            i++;
            fprintf(fout, "%d<->%d:\t%lu\t%lu\t%lu\n", it->first, it2->first,
                    intersize, size1, size2);
            fprintf(fel, "%d\t%d\n", it->first, it2->first);
          }
        }
      }
    cout << "there are " << g.nodes.size() << " node." << endl;
    cout << "there are " << i << " pairs." << endl;

  } else {
    cerr << "network type not given!" << endl;
  }

  fclose(fin);
  fclose(fout);
  fclose(fel);
  cout << "done." << endl;
}
