
#include <iostream>
#include <fstream>
#include "forked_stream.h"
#include "prefixed_stream.h"

void sample2()
{
  prefixed_ostream_holder prefix_holder_cout(&std::cout, "sample2() STDOUT: ");
  prefixed_ostream_holder prefix_holder_cerr(&std::cerr, "sample2() STDERR: ");

  std::cout << "test output from sample2()" << std::endl;
  return;
}

void sample()
{
  prefixed_ostream_holder prefix_holder_cout(&std::cout, "sample() STDOUT: ");
  prefixed_ostream_holder prefix_holder_cerr(&std::cerr, "sample() STDERR: ");

  std::cout << "test output from sample()" << std::endl;
  sample2();
  return;
}

/*int main(int argc, char *argv[])
{
  const char *filename = argv[1];
  std::ofstream fout(filename);
  forked_ostream_holder holder(&std::cout, fout);
  prefixed_ostream_holder prefix_holder_cout(&std::cout, "STDOUT: ");
  prefixed_ostream_holder prefix_holder_cerr(&std::cerr, "STDERR: ");

  if(fout) {
    std::cout << "Hello, world to cout" << std::endl;
    std::cerr << "Hello, world to cerr" << std::endl;
    sample();
    std::cout << "Second hello, world to cout" << std::endl;
    std::cerr << "Second hello, world to cerr" << std::endl;
  }
  else {
    holder.detach();
    if(argc <= 1) {
      std::cerr << "No filenames are specified." << std::endl;
    }
    else {
      std::cerr << "Failed to open \"" << filename << "\"." << std::endl;
    }
  }
  return 0;
}
*/