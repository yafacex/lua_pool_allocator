chcp 65001
echo "这是性能测试，结果生成在benchamark目录"
set curr_dir=%~dp0
md %curr_dir%benchmark
%curr_dir%build/lua54 %curr_dir%tests\test.lua > %curr_dir%benchmark\lua54.txt
%curr_dir%build/lua54_pool %curr_dir%tests\test.lua > %curr_dir%benchmark\lua54_pool.txt
pause