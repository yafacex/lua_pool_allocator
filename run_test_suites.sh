echo "这个是测试lua54_pool的正确性的，测试通过后大概率能保证其正确性，不会造成崩溃或"
curr_dir=$(pwd)
cd lua-5.4.4-tests
$curr_dir/build/lua54_pool -e "_port=true" all.lua