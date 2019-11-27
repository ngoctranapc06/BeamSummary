// system includes
#include <iostream>
#include <string>
// system C includes
#include <getopt.h>

// user includes
#include "wgErrorCodes.hpp"
#include "wgLogger.hpp"
#include "wgBeamSummaryData.hpp"
#include "wgTopology.hpp"
// print_help
// prints an help message with all the arguments taken by the program
void print_help(const char * program_name) {
  std::cout << "the " << program_name << " integrates the BSD info into the\n"
      " TTree of the decoded _tree.root file\n"
      "  -h         : help\n"
      "  -f (char*) : input ROOT file (mandatory)\n"
      "  -p (char*) : input Pyrame config file (mandatory)\n"
      "  -o (char*) : output directory (default = same as input)\n"
      "  -n (int)   : DIF number (must be 0-7) (default = 0)\n"
      "  -r         : overwrite mode (default = false)\n"
      " <other options here>";
      exit(0);
}

int main(int argc, char* argv[]) {
  int opt;
  std::string input_file;
  std::string topology_source;
  std::string output_directory;
  bool overwrite = false;
  std::string pyrame_config_file("");
  unsigned dif = 0;

  while ((opt = getopt(argc, argv, "f:o:p:n:hr")) != -1) {
    switch (opt) {
      case 'f':
        input_file = optarg;
        break;
      case 'o':
        output_directory = optarg;
        break;
      case 'p':
        pyrame_config_file = optarg;
        break;
      case 'n':
        dif= std::stoi(optarg);
        break;
      case 'r':
        overwrite = true;
        break;

        // other options here

      case 'h':
      default:
        print_help(argv[0]);
        break;
    }
  }

  int result;
  if ((result = wgBeamSummaryData(input_file,
                                  topology_source,
//				  				  pyrame_config_file,
                                  output_directory,
                                  overwrite,
                                  dif)) != WG_SUCCESS) {
    Log.eWrite("[wgBeamSummaryData] returned error " +  std::to_string(result));
  }
  exit(result);
}
