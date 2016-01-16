#include <map>
#include <iostream>
#include "boost/accumulators/accumulators.hpp"
#include "boost/accumulators/statistics/stats.hpp"
#include "boost/accumulators/statistics/mean.hpp"
#include "boost/accumulators/statistics/moment.hpp"
#include "boost/accumulators/statistics/variance.hpp"
#include "boost/accumulators/statistics/count.hpp"
#include "boost/program_options.hpp"

using namespace std;
using namespace boost::accumulators;

namespace po = boost::program_options;

int main(int argc, char **argv) {
  string ifname;
  string ofname;

  po::options_description desc("General Options");
  desc.add_options()
		("help", "produce help message")
		("input,i", po::value<string>(&ifname), "input file")
		("output,o", po::value<string>(&ofname), "output file");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    cout << desc << "\n";
    return 0;
  }

  auto dicx =
      map<int, accumulator_set<double, stats<tag::mean, tag::variance>>>();
  auto dicy =
      map<int, accumulator_set<double, stats<tag::mean, tag::variance>>>();
  auto dicr =
      map<int, accumulator_set<double, stats<tag::mean, tag::variance>>>();

  int from, to;
  float x, y, r;
  FILE *fin = fopen(ifname.c_str(), "r");
  FILE *fout = fopen(ofname.c_str(), "w");
  int i = 1;
  while (fscanf(fin, "%d\t%d\t%f\t%f\t%f", &from, &to, &x, &y, &r) != EOF) {
    dicx[from * 10000 + to](x);
    dicy[from * 10000 + to](y);
    dicr[from * 10000 + to](log(r));
    i++;
  }
  fclose(fin);
  int total = dicx.size();
  cout << "there are " << total << " records" << endl;
  int ind = 0;
  int progress = 1;
  for (auto it = dicx.begin(); it != dicx.end(); it++) {
    ind++;
    if (ind > total * progress / 100) {
      cout << progress << " has been done \t" << ind << " records processed"
           << endl;
      progress++;
    }
    int c = boost::accumulators::count(it->second);
    if (c > 1) {
      fprintf(fout, "%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\n", it->first, c,
              variance(it->second) * c / (c - 1),
              variance(dicy[it->first]) * c / (c - 1),
              variance(dicr[it->first]) * c / (c - 1), mean(it->second),
              mean(dicy[it->first]), mean(dicr[it->first]));
    } else {
      fprintf(fout, "%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\n", it->first, c,
              variance(it->second), variance(dicy[it->first]),
              variance(dicr[it->first]), mean(it->second),
              mean(dicy[it->first]), mean(dicr[it->first]));
    }
  }
  fclose(fout);
  cout << "done." << endl;
}
