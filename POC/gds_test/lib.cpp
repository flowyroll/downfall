#include <map>
#include <iostream>
#include "lib.h"

typedef std::map<std::string, int> Map;
static Map * map;

void* map_create() {
  map = new Map();
  return reinterpret_cast<void*> (map);
}

void map_increment(char * k) {
  Map* m = reinterpret_cast<Map*> (map);
  (*m)[std::string(k)]++;
}

void map_dump() {
  for(std::map<std::string, int>::iterator i = map->begin(); i != map->end(); i++)
    std::cout << i->first << ": " << i->second << std::endl;
}


