

#ifdef __cplusplus
extern "C" {
#endif

void map_create();
void map_dump();
void map_clear();
void map_increment(char *);
size_t map_get(const char *);
const char *  map_search_prefix(char *, size_t);


#ifdef __cplusplus
}
#endif
