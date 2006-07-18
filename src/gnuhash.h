#ifndef _GNUHASH_H_
#define _GNUHASH_H_
#ifdef _WIN32
#include <hash_map>
#else
#include <ext/hash_map>
#define stdext __gnu_cxx
#include "hashtable.h"
namespace stdext{
  template<> class hash<std::string> {
  public:
    size_t operator () (const std::string&key) const{
      size_t _HASH_INTSIZE =(sizeof(size_t)*8);
      size_t _HASH_SALT_0 =0x7EF92C3B;
      size_t _HASH_SALT_1 =0x9B;
      size_t k = 0;
      for(std::string::const_iterator start = key.begin(); start!=key.end(); start++) {
        k ^= (*start&_HASH_SALT_1);
        k ^= _HASH_SALT_0;
        k  = (((k>>4)&0xF)|(k<<(_HASH_INTSIZE-4)));
        k ^= *start;
      }
      return k;   
    }
  };

  template<> class hash<void *> {
    hash<size_t> a;
  public:
    size_t operator () (const void *key) const{
      return a((size_t)key);
    }
  };
  template<> class hash<const void *> {
    hash<size_t> a;
  public:
    size_t operator () (const void * const &key) const{
      return a((size_t)key);
    }
  };

  template<> class hash<const Unit *> {
    hash<size_t> a;
  public:
    size_t operator () (const Unit * const &key) const{
      return a((size_t)key);
    }
  };


}
#endif
#endif