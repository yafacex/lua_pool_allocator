curr_dir=$(pwd)
emptydir(){
    if [[ -d $1 ]];then
        rm -rf $1
    fi
    mkdir $1
}

emptydir $curr_dir/build

make_one(){
    cd $curr_dir/$1
    emptydir build
    cd build
    cmake ..
    cmake --build . --config Release
    cp $curr_dir/$1/build/$2 $curr_dir/build/
    chmod +x $curr_dir/build/$2
}

make_one make_lua_normal lua54
make_one make_lua_pool lua54_pool
