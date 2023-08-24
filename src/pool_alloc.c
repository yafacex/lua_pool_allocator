#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <lua.h>
#include "lauxlib.h"
#define CHUNK_COUNT 15
#define MAX_BLOCK_SIZE 640
#define CHUNK_SIZE 16 * 1024 //16k for one chunk,from 1k to 16k,the larger the faster(my test)
#define BOOL int
#define TRUE 1
#define FALSE 0
struct PoolChunkInfo {
	size_t blockSize;
	size_t blockCount;
};
struct PoolChunkInfo blockSizeMap[CHUNK_COUNT] =
{
	{8,0},//can not be less than 8 bytes
	{16,0},
	{32,0},
	{64,0},
	{96,0},
	{128,0},
	{160,0}, 
	{192,0},
	{224,0},
	{256,0},
	{320,0},
	{384,0},
	{448,0},
	{512,0},
	{MAX_BLOCK_SIZE,0}, 
};
struct PoolStat {
	size_t iCreate;
	size_t iFree;
	size_t iHitCreate;
	size_t iHitFree;
	size_t iChunkCount;
};
struct PoolStat Stats[CHUNK_COUNT];
int SizeToChunkId[MAX_BLOCK_SIZE + 1];

struct PoolChunk {
	size_t blockSize;
	size_t blockCount;
	size_t totalSize;
	struct PoolChunk* next;
	char* blocks;
};
struct PoolChunk* ChunkList[CHUNK_COUNT];
struct PoolBlock {//use for free blocks
	struct PoolBlock* next;
};
struct PoolBlock* FreeBlocks[CHUNK_COUNT];

void init_chunk(struct PoolChunk* chunk,int iChunk) {
	chunk->blockSize = blockSizeMap[iChunk].blockSize;
	chunk->blockCount = CHUNK_SIZE / chunk->blockSize;
	chunk->totalSize = chunk->blockSize * chunk->blockCount;
	chunk->next = NULL;
	chunk->blocks = malloc(chunk->totalSize);
	if (chunk->blocks != NULL) {
		memset((void*)chunk->blocks,0xEF,chunk->totalSize);
	}
	for (int iBlock = 0; iBlock < chunk->blockCount ; ++iBlock) {
		int iPos = iBlock * chunk->blockSize;
		char* currBlock = (chunk->blocks + iPos);
		if (iBlock < chunk->blockCount - 1) {
			void* nextBlock = (currBlock + chunk->blockSize);
			if (currBlock != NULL) {
				((struct PoolBlock*)currBlock)->next = nextBlock;
			}
		}
		else {
			if (currBlock != NULL) {
				((struct PoolBlock*)currBlock)->next = NULL;
			}
		}
	}
}
void init_pool_alloc() {
	memset(Stats,0,sizeof(struct PoolStat) * CHUNK_COUNT);
	//init size map
	{
		int iChunk = 0;
		for (int iSize = 1; iSize <= MAX_BLOCK_SIZE; ++iSize) {
			SizeToChunkId[iSize] = iChunk;
			if (iSize == blockSizeMap[iChunk].blockSize) {
				++iChunk;
			}
		}
	}
	//init chunks
	{
		for (int iChunk = 0; iChunk < CHUNK_COUNT; ++iChunk) {
			struct PoolChunk* chunk = malloc(sizeof(struct PoolChunk));
			ChunkList[iChunk] = chunk;
			init_chunk(chunk,iChunk);
			if (chunk != NULL) {
				FreeBlocks[iChunk] = chunk->blocks;
			}
		}
	}
}
void pool_alloc_stat(size_t osize, size_t nsize, BOOL hitCache)
{
	if (nsize == 0) {
		int isize = (int)osize;
		if (isize <= MAX_BLOCK_SIZE) {
			if (hitCache) {
				Stats[SizeToChunkId[isize]].iHitFree += 1;
			}
			else {
				Stats[SizeToChunkId[isize]].iFree += 1;
			}
		}
	}
	else {
		int isize = (int)nsize;
		if (isize <= MAX_BLOCK_SIZE) {
			if (hitCache) {
				Stats[SizeToChunkId[isize]].iHitCreate += 1;
			}
			else {
				Stats[SizeToChunkId[isize]].iCreate += 1;
			}
		}
	}
}
static int totalAllocTimes = 0;
void* simple_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
	(void)ud;   /* not used */
	//++totalAllocTimes;
	pool_alloc_stat(osize,nsize,FALSE);
	if (nsize == 0) {
		free(ptr);
		return NULL;
	}
	else {
		return realloc(ptr, nsize);
	}
}
void* pool_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
	//++totalAllocTimes;
	//printf("totalAllocTimes:%d\n",totalAllocTimes);
	(void)ud;   /* not used */
	if (nsize == 0) {
		if (ptr != NULL) {
			int isize = (int)osize;
			if (isize <= MAX_BLOCK_SIZE) {
				int iChunk = SizeToChunkId[isize];
				struct PoolBlock* block = FreeBlocks[iChunk];
				((struct PoolBlock*)ptr)->next = block;
				FreeBlocks[iChunk] = ptr;
				pool_alloc_stat(osize,nsize,TRUE);
				return NULL;
			}
			else {
				pool_alloc_stat(osize,nsize,FALSE);
				free(ptr);
				return NULL;
			}
		}
		else {
			return NULL;
		}
	}
	else {
		int i_nsize = (int)nsize;
		if (i_nsize <= MAX_BLOCK_SIZE) {
			int iChunk = SizeToChunkId[i_nsize];
			struct PoolBlock* block = FreeBlocks[iChunk];
			if (block == NULL) {
				struct PoolChunk* newChunk = malloc(sizeof(struct PoolChunk));
				if (newChunk != NULL) {
					init_chunk(newChunk,iChunk);
					newChunk->next = ChunkList[iChunk];
					ChunkList[iChunk] = newChunk;
					FreeBlocks[iChunk] = newChunk->blocks;
					block = newChunk->blocks;
					Stats[SizeToChunkId[iChunk]].iChunkCount += 1;
				}
			}
			else {
				//FreeBlocks[iChunk] = ((struct PoolBlock*)block)->next;
			}
			if (block != NULL) {
				FreeBlocks[iChunk] = block->next;
				if (ptr != NULL) {
					//luaM_shrinkvector_ lua table会缩容，所以这里要取最小值
					memcpy(block,ptr,osize < nsize? osize : nsize);
					//printf("block ptr:%p (%d/%lld)\n",block,isize,ChunkList[iChunk].blockSize);
				}
				else {
					//memset(block,0xFF,ChunkList[iChunk]->blockSize);
					//printf("block ptr:%p (%d/%lld)\n",block,isize,ChunkList[iChunk].blockSize);
				}
				pool_alloc_stat(osize,nsize,TRUE);
				return block;
			}
			else {
				//fatal,impossible
				lua_assert(false);
				return NULL;
			}
		}
		else {
			if (ptr != NULL) {
				int i_osize = (int)osize;
				if (i_osize <= MAX_BLOCK_SIZE) {
					//old memory also comes from pool
					void* new_ptr = malloc(nsize);
					if (new_ptr != NULL) {
						memcpy(new_ptr,ptr,osize);
					}

					int iChunk = SizeToChunkId[i_osize];
					struct PoolBlock* block = FreeBlocks[iChunk];
					((struct PoolBlock*)ptr)->next = block;
					FreeBlocks[iChunk] = ptr;
					pool_alloc_stat(osize,nsize,TRUE);
					return new_ptr;
				}
				else {
					return realloc(ptr, nsize);
				}
			}
			else {
				return realloc(ptr, nsize);
			}
		}
	}
}
void* alloc_entry(void *ud, void *ptr, size_t osize, size_t nsize)
{
#ifdef USE_LUA_ALLOC_POOL
	return pool_alloc(ud,ptr,osize,nsize);
#else
	return simple_alloc(ud,ptr,osize,nsize);
#endif
}
int alloc_getStat(lua_State* L)
{
	lua_newtable(L);
	size_t totalCaheMem = 0;

	for (int iChunk = 0; iChunk < CHUNK_COUNT; ++iChunk) {
		lua_pushinteger(L,iChunk + 1);
		lua_newtable(L);

		lua_pushstring(L,"blockSize");
		lua_pushinteger(L,blockSizeMap[iChunk].blockSize);
		lua_settable(L,-3);

		lua_pushstring(L,"blockCount");
		lua_pushinteger(L,ChunkList[iChunk]->blockCount);
		lua_settable(L,-3);

		lua_pushstring(L,"iCreate");
		lua_pushinteger(L,Stats[iChunk].iCreate);
		lua_settable(L,-3);

		lua_pushstring(L,"iFree");
		lua_pushinteger(L,Stats[iChunk].iFree);
		lua_settable(L,-3);

		lua_pushstring(L,"iHitCreate");
		lua_pushinteger(L,Stats[iChunk].iHitCreate);
		lua_settable(L,-3);

		lua_pushstring(L,"iHitFree");
		lua_pushinteger(L,Stats[iChunk].iHitFree);
		lua_settable(L,-3);

		lua_pushstring(L,"iChunkCount");
		lua_pushinteger(L,Stats[iChunk].iChunkCount);
		lua_settable(L,-3);

		size_t mem = Stats[iChunk].iChunkCount * CHUNK_SIZE;
		totalCaheMem += mem;
		lua_pushstring(L,"chunkMem");
		lua_pushinteger(L,mem);
		lua_settable(L,-3);

		lua_settable(L,-3);
	}

	lua_pushstring(L,"cacheMem");
	lua_pushinteger(L,totalCaheMem);
	lua_settable(L,-3);

	return 1;
}
static const luaL_Reg lib[] = {
  {"getStat",alloc_getStat },
  {NULL, NULL}
};


LUAMOD_API int luaopen_alloc (lua_State *L) {
  luaL_newlib(L, lib);
  return 1;
}
