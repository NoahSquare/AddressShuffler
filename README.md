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
#### 7/20
Files under llvm/project/compiler-rt/lib/shuffler get compiled   
Added function call follows alloca instructions   
TODO: Try to link shuffler runtime library to clang
##### Modifications
- Edited file llvm/project/compiler-rt/lib/CMakeLists.txt
- Edited file llvm/lib/Transforms/Instrumentation/AddressShuffler.cpp
---
#### 7/21
Linked runtime library with clang, runtiem function successfully got called in instrumented program   
TODO: let the runtime function save mapping information
##### Modifications
- shuffler/llvm/projects/compiler-rt/include/CMakeLists.txt
- shuffler/llvm/projects/compiler-rt/lib/shuffler/shuffler_malloc.cc
- shuffler/llvm/tools/clang/lib/Driver/Tools.cpp
- shuffler/llvm/tools/clang/runtime/CMakeLists.txt
- ---
#### 7/25
- Trying to figure out a way to save/load address to/from runtime;
- Tried a procedure that reserves memory at compiling time and writes to the reserved memory at runtime, but didn't work(runtime seg fault);
##### Update
- Read Asan and figured out a way to use Dyanmic Alloca to achieve the communication.
- TODO: Setup the mapping table.
---
#### 7/30
- Setting up hashmap library
- Finded a usable hashmap at https://troydhanson.github.io/uthash/, map_info structure now can added/found to/from hashmap.    
- TODO: hashmap functions need to modify a bit to fit our structure (int->uptr)
---