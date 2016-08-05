#!/bin/sh
#clang -S -emit-llvm simple_test.c -o output.ll
if [ ! -d "./build" ]; then
  echo "Makefile before testing!"
  exit 1
fi
./build/bin/clang -shuffler -S -emit-llvm simple_test.c -o shuffler_output.ll 2> Err.txt
./build/bin/clang -shuffler simple_test.c 2> Err.txt
echo "------------- Shuffler IR Output --------------"
#cat ./output.ll
cat ./shuffler_output.ll
echo "-----------------------------------------------"
echo "Executing..."
./a.out
echo "Completed!"
rm Err.txt
rm *.ll
rm a.out
