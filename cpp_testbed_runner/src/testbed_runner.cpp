#include <chrono>
#include <cstdio>
#include <memory>
#include <string>
#include <thread> // sleep_for
#include <getopt.h>

#include "rclcpp/rclcpp.hpp"
//#include "rcutils/cmdline_parser.h"

#include "std_msgs/msg/string.hpp"

//#include "setup_reader.hpp"
#include "testbed_robot.hpp"

static unsigned int count = 5;
static std::string _home(getenv("HOME"));
static const std::string default_setup_file(_home.append("/gits/ns3_testbed/csv_setup/example1.csv"));
static std::string input_file = default_setup_file;
static bool use_nns = false;
static bool use_pipe = false;
static bool use_staggered = false;
static bool verbose = false;

void print_usage()
{
  printf("Usage:\n");
  printf("testbed_runner -h|-H|-c <count>|--count <count>|-i <file>|--input_file <file>|-n|--use_nns|-p|--use_pipe|-s|--use_staggered|-v|--verbose\n");
  printf("options:\n");
  printf("-h : Print this help function.\n");
  printf("-c <count>: Number of robots, starting at 1.\n");
  printf("-i <input setup file>: The CSV input setup file.\n");
  printf("-n: Use network namespace.\n");
  printf("-p: Use pipe.\n");
  printf("-s: Use staggered start.\n");
  printf("-v: verbose.\n");
}

// parse user input
void get_options(int argc, char *argv[]) {

  // parse options
  int option_index; // not used
  while (1) {

    const struct option long_options[] = {
      // options
      {"help",                          no_argument, 0, 'h'},
      {"Help",                          no_argument, 0, 'H'},
      {"count",                   required_argument, 0, 'c'},
      {"input_file",              required_argument, 0, 'i'},
      {"use_nns",                 required_argument, 0, 'n'},
      {"use_pipe",                required_argument, 0, 'p'},
      {"use_staggered",                 no_argument, 0, 's'},
      {"verbose",                       no_argument, 0, 'v'},

      // end
      {0,0,0,0}
    };

    int ch = getopt_long(argc, argv, "hHc:i:npsv", long_options, &option_index);

    if (ch == -1) {
      // no more arguments
      break;
    }
    if (ch == 0) {
      // command options set flags and use ch==0
      continue;
    }
    switch (ch) {
      case 'h': {	// help
        print_usage();
        exit(0);
      }
      case 'H': {	// Help
        print_usage();
        exit(0);
      }
      case 'c': {	// count
        count = std::atoi(optarg);
        break;
      }
      case 'i': {	// input_file
        input_file = optarg;
        break;
      }
      case 'n': {	// nns
        use_nns = true;
        break;
      }
      case 'p': {	// pipe
        use_pipe = true;
        break;
      }
      case 's': {	// staggered start
        use_staggered = true;
        break;
      }
      case 'v': {	// count
        verbose = true;
        break;
      }
      default:
        std::cerr << "unexpected command character " << ch << "\n"
                  << "See usage.\n";
        exit(1);
    }
  }
}

// start, stay here until done.
void _start_robots(unsigned int count, bool use_nns,
                   pipe_writer_t* pipe_writer_ptr,
                   bool use_staggered,
                   bool verbose, publishers_subscribers_t* ps_ptr) {

  std::vector<std::thread*> threads;

  for(unsigned int i = 1; i<= count; i++) {
    std::cout << "Starting testbed runner " << i << std::endl;

    // nns#, R#
    std::stringstream ss1;
    std::stringstream ss2;
    ss1 << "nns" << i;
    ss2 << "R" << i;
    std::string nns = ss1.str();
    std::string r = ss2.str();

    std::cout << "Starting " << nns << " " << r << std::endl;
    threads.push_back(new std::thread(testbed_robot_run, nns, r,
                             use_nns, pipe_writer_ptr, verbose, ps_ptr));

    // sleep to stagger
    if (use_staggered) {
      std::this_thread::sleep_for(std::chrono::milliseconds(88));
    }
  }

  // run until all join
  std::cout << "Running..." << std::endl;
  for (std::vector<std::thread*>::iterator it = threads.begin();
       it != threads.end(); ++it) {
    (*it)->join();
  }
  std::cout << "Done." << std::endl;
}


int main(int argc, char * argv[])
{
  // Force flush of the stdout buffer.
  // This ensures a correct sync of all prints
  // even when executed simultaneously within the launch file.
  setvbuf(stdout, NULL, _IONBF, BUFSIZ);

  get_options(argc, argv);

  // pointer to static list of publishers and subscribers
  publishers_subscribers_t* publishers_subscribers_ptr =
                      new publishers_subscribers_t(input_file, verbose);

  std::cout << "num publisers: "
            << publishers_subscribers_ptr->publishers.size() << "\n";
  std::cout << "num subscribers: "
            << publishers_subscribers_ptr->subscribers.size() << "\n";

  // call once per process for rclcpp
  rclcpp::init(argc, argv);

  // logger
  pipe_writer_t pipe_writer(use_pipe);

  // start
  _start_robots(count, use_nns, &pipe_writer, use_staggered, verbose,
                publishers_subscribers_ptr);

  rclcpp::shutdown();
  return 0;
}

