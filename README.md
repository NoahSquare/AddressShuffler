# AddressShuffler
TODO: Description
## Installation
To build clang/llvm
> $ make
## Usage
To test AddressShuffler
> $ sh test.sh

To browse test file
> $ cat simple_test.c
## History
#### 7/7
Setting up LLVM Pass for the shuffler, Shuffler Pass now will be loaded and run automatically
##### Modifications
- Added file llvm/lib/Transforms/Instrumentation/AddressShuffler.cpp
- Edited file llvm/lib/Transforms/Instrumentation/CMakeLists.txt
- Edited file llvm/include/llvm/InitializePasses.h
- Edited file llvm/include/llvm/Transforms/Instrumentation.h
- Edited file llvm/tools/clang/lib/CodeGen/BackendUtil.cpp
---
#### 7/8
Replacing alloca instructions to malloc
#
---
#### 7/14
Adding compiler-rt files, continuing replacing alloca instructions
##### Modifications
- Edited file llvm/lib/Transforms/Instrumentation/AddressShuffler.cpp
- Added files under llvm/project/compiler-rt/lib/shuffler
---
#### 7/15
Continuing replacing alloca instructions
#
---
#### 7/18
Tested Load from malloc address space   
TODO: Try to setup and insert runtime calls to save map information   

*Added source directory and files to cmake list and makefile, but still not got compiled; Expected that llvm/project/compiler-rt/lib/shuffler/shuffler_malloc.cc could get compiled*
##### Modifications
- Edited file llvm/lib/Transforms/Instrumentation/AddressShuffler.cpp
- Edited file llvm/project/compiler-rt/lib/CMakeLists.txt
- Edited file llvm/project/compiler-rt/lib/Makefile.mk   
#
---