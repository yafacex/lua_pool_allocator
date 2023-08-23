
set curr_dir=%~dp0

rd /s /q %curr_dir%build
rd /s /q %curr_dir%make_lua_normal\build
rd /s /q %curr_dir%make_lua_pool\build

cd %curr_dir%make_lua_normal
md build
cd build
cmake ..
cmake --build . --config Release

cd %curr_dir%make_lua_pool
md build
cd build
cmake ..
cmake --build . --config Release

md %curr_dir%build
copy /Y  %curr_dir%make_lua_normal\build\Release\lua54.exe %curr_dir%build\
copy /Y  %curr_dir%make_lua_pool\build\Release\lua54_pool.exe %curr_dir%build\
pause