chcp 65001
echo "这个是测试lua54_pool.exe的正确性的，测试通过后大概率能保证其正确性，不会造成崩溃或"
set curr_dir=%~dp0
cd lua-5.4.4-tests
%curr_dir%build\lua54_pool.exe -e "_port=true" all.lua
IF ERRORLEVEL 1 echo "Error!!!!!"
pause