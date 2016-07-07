LLVM_DIR = ${CURDIR}/llvm
BUILD_DIR = ${CURDIR}/build

addrshuffler:
	mkdir -p ${BUILD_DIR}
	(cd ${BUILD_DIR} && \
		CC=clang CXX=clang++ \
			cmake -DLLVM_TARGETS_TO_BUILD=X86 \
			-DCMAKE_BUILD_TYPE=Release ${LLVM_DIR})
	(cd ${BUILD_DIR} && make -j`nproc`)

clean:
	rm -rf ${BUILD_DIR}
