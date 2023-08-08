#include <map>
#include <iostream>
#include <iomanip>
#include "lib.h"

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


double bit_test(std::string s) {
  double c = 0;
  for(int i = 0; i < s.size(); i++){
    c += __builtin_popcount(s[i] & 0xff);
  }
  return c / (s.size() * 8) ;
}



void map_dump() {

  int cc = 0;
  for(std::map<std::string, size_t>::iterator i = map->begin(); i != map->end(); i++){

    size_t score = i->second;
    double bitscore = bit_test(i->first);

    if(score > 16 && bitscore < 0.65 && bitscore > 0.35)
    {
      std::cout << std::setw(2) << std::setfill('0') << cc << ' ';

      for(int j = 0; j < i->first.size(); j++)
        std::cout <<  std::setw(2) << std::setfill('0') << std::hex << (short(i->first[j]) & 0xff) ; 

      std::cout << ' ' << std::dec << score;
      std::cout << ' ' << std::fixed << bitscore;
      std::cout << std::endl;
      cc++;
    }
  }
}