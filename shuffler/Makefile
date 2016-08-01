LLVM_DIR = ${CURDIR}/llvm
BUILD_DIR = ${CURDIR}/build

addrshuffler: addrshuffler-config
	(cd ${BUILD_DIR} && make -j`nproc`)

addrshuffler-config: ${BUILD_DIR}
	(cd ${BUILD_DIR} && \
		CC=clang CXX=clang++ \
			cmake -DLLVM_TARGETS_TO_BUILD=X86 \
			-DCMAKE_BUILD_TYPE=Release ${LLVM_DIR})


test:
	(cd ${BUILD_DIR} && make check-shuffler)

old-test:
	(cd ${BUILD_DIR} && make check-asan)

${BUILD_DIR}:
	mkdir -p ${BUILD_DIR}

clean:
	rm -rf ${BUILD_DIR}

.phony:
	clean addrshuffler addrshuffler-config
