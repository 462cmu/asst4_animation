#ifndef CMU462_PNG_H
#define CMU462_PNG_H

#include <map>
#include <vector>

#include "CMU462/CMU462.h" // Standard 462 Vectors, etc.


namespace CMU462 {

struct PNG {
  int width;
  int height;
  std::vector<unsigned char> pixels;
}; // class PNG

class PNGParser {
 public:
  static int load( const unsigned char* buffer, size_t size, PNG& png );
  static int load( const char* filename, PNG& png );
  static int save( const char* filename, const PNG& png );
}; // class PNGParser

} // namespace CMU462

#endif // CMU462_PNG_H
