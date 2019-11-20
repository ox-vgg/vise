/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

==== Copyright:

The library belongs to Relja Arandjelovic and the University of Oxford.
No usage or redistribution is allowed without explicit permission.
*/

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
