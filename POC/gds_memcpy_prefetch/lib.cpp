#include <map>
#include <iostream>
#include <iomanip>
#include "lib.h"

// #define HEX( x )
//    std::setw(2) << std::setfill('0') << hex << (int)( x )

typedef std::map<std::string, size_t> Map;
static Map * map;

void map_create() {
  map = new Map();
}

void map_increment(char * k) {
  Map* m = reinterpret_cast<Map*> (map);
  (*m)[std::string(k)]++;
}

size_t map_get(const char * k) {
  Map* m = reinterpret_cast<Map*> (map);
  return (*m)[std::string(k)];
}

void map_clear() {
  map->clear();
}

const char * map_search_prefix(char * prefix, size_t c){
  for(std::map<std::string, size_t>::iterator i = map->begin(); i != map->end(); i++)
  {
    size_t j;
    for(j = 0; j < c; j++){
      if(i->first[j] != prefix[j])
        break;
    }
    if(j == c)
      return i->first.c_str();

  }
  return NULL;
}

void map_dump() {
  for(std::map<std::string, size_t>::iterator i = map->begin(); i != map->end(); i++){
    for(int j = 0; j < i->first.size(); j++)
      std::cout <<  std::setw(2) << std::setfill('0') << std::hex << (short(i->first[j]) & 0xff) ; 

    std::cout << "-" << i->first << ": " << i->second << std::endl;
  }
}


