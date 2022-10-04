# Build and Install Dyninst


## 1. Build & Install

```shell
git clone https://github.com/dyninst/dyninst.git
git checkout -b V12 v12.0.0

mkdir dyninst_build && cd dyninst_build
cmake ../dyninst -DCMAKE_INSTALL_PREFIX=`pwd`/../dyninst_install -DSTERILE_BUILD=OFF

make -j48
make install
```

## 2. Dyninst examples

```shell
$ git clone https://github.com/dyninst/examples.git dyninst_examples

$ mkdir dyninst_examples_build && cd dyninst_examples_build
$ cmake ../dyninst_examples -DDyninst_DIR=`pwd`/../dyninst_install/lib/cmake/Dyninst
```
Run simple example:
```shell
$ cd ./dyninst_examples_build/codeCoverage
$ export DYNINST_INSTALL=path/to/dyninst_install
$ export DYNINSTAPI_RT_LIB=$DYNINST_INSTALL/lib/libdyninstAPI_RT.so
$ export LD_LIBRARY_PATH=.:$DYNINST_INSTALL/lib:$LD_LIBRARY_PATH
$ ./code_coverage -p testcc testcc.inst
$ ./testcc.inst
```



## 3. Build Dyninst-app out-of-source

