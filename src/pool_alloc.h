void* alloc_entry(void* ud, void* ptr, size_t osize, size_t nsize);
void* pool_alloc(void *ud, void *ptr, size_t osize, size_t nsize);
void init_pool_alloc();
int luaopen_alloc (lua_State *L) ;