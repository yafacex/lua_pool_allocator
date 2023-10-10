echo "这是性能测试，结果生成在benchamark目录"
curr_dir=$(pwd)
if [[ ! -d $curr_dir/benchmark ]];then
    mkdir $curr_dir/benchmark
fi
$curr_dir/build/lua54 $curr_dir/tests/test.lua > $curr_dir/benchmark/lua54.txt
$curr_dir/build/lua54_pool $curr_dir/tests/test.lua > $curr_dir/benchmark/lua54_pool.txt