This is a lua pool allocator for optimizing performance of allocating small object.

The main idea is prealloc chunks by fixed size and reuse them later.

I come up an idea to optimize lua's default allocator.I search the internet and found no one.

This alloctor speed up 10%~30% testing by some allocating small table and string tests.See it at 'tests' and 'benchmark' 

Install:
copy 'src/pool_alloc.h' and 'src/pool_alloc.c' to your project and use code below to create your lua state. 

```c
init_pool_alloc();
lua_State *L = lua_newstate(pool_alloc,NULL);
```

You can also call
```c
luaopen_alloc(L);
```
which can register alloc module,then call 
local stats = alloc.getStat()
to get statistics infomation about the cache hit count,chunk count and block count.


Benchmark:

```
test_mix use time:2.919000 / 3.657000  = 79.9%
test_vector use time:0.115000 / 0.167000  =  68.8%
test_small_strings use time:0.344000 / 0.412000 = 83.4%
```

build_all.bat    build lua54.exe and lua54_pool.exe with cmake

run_benchmark_tests.bat    run benchmark test with tests/test.lua

run_test_suites.bat    run lua-5.4.4-tests

below is core code's link :
https://github.com/yafacex/lua_pool_allocator/blob/main/src/pool_alloc.c

TODO:Support chunk count max size to avoid too many memory allocatings.



游戏开发，想优化下lua的内存分配，尤其在游戏计算时用到大量向量计算，分配了很多临时table的情况。

核心思路是对640字节以下的内存申请，预分配内存并复用，类似池化(MemoryPool)。

通过了个人写的测试用例和lua的官方test suites。

项目使用还需谨慎，有实验结果的可以反馈给我（943100400@qq.com）。

性能测试结果在benchmark/lua54.txt和benchmark/lua54_pool.txt查看



核心代码在 'src/pool_alloc.h' 和 'src/pool_alloc.c'，拷贝这两个文件到你的工程

https://github.com/yafacex/lua_pool_allocator/blob/main/src/allocator/pool_alloc.c

调用下面两行创建lua state并启用缓存。
```c
init_pool_alloc();
lua_State *L = lua_newstate(pool_alloc,NULL);
```


```c
luaopen_alloc(L);
```
可以注册alloc模块，通过alloc.getStat()可以看到缓存的统计参数包括缓存命中，block大小，chunk个数等。



build_all.bat    通过cmake生成lua54.exe和lua54_pool.exe

run_benchmark_tests.bat    执行性能测试文件tests/test.lua，结果输出到benchmark目录

run_test_suites.bat    run lua-5.4.4-tests  跑lua官方的测试，确保没有问题。

TODO:
支持chunk个数上限，避免一帧里创建太多table造成内存过高，且无法回收。