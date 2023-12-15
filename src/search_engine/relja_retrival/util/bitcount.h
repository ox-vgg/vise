/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

*/
/*
#if 0
// for debug (older valgrind doesn't support popcount, etc

// only use for uint{8,16,32,64}_t types!
template <class T>
int bitcount(T n){
   int count = 0;
   while (n){
      count += n & 1u;
      n >>= 1;
   }
   return count;
}

#define bitcount64(x) bitcount<uint64_t>(x)

#else

#define bitcount64(x) __builtin_popcountll(x)

#endif
*/
// to support cross compilation
// Abhishek Dutta <adutta@robots.ox.ac.uk>, 13 March 2020
#ifdef _WIN32
#include <intrin.h>
#define bitcount64(x) __popcnt64(x)
#else
#define bitcount64(x) __builtin_popcountll(x)
#endif