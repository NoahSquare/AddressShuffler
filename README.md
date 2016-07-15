# Editlog
---
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
##### Modifications
- Edited file llvm/lib/Transforms/Instrumentation/AddressShuffler.cpp
---
#### 7/14
Adding compiler-rt files, continuing replacing alloca instructions
##### Modifications
- Edited file llvm/lib/Transforms/Instrumentation/AddressShuffler.cpp
- Added files under llvm/project/compiler-rt/lib/shuffler
---
#### 7/15
Continuing replacing alloca instructions
##### Modifications
- Edited file llvm/lib/Transforms/Instrumentation/AddressShuffler.cpp
---