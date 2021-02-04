# Testing Lean 4 with AFL
I am fuzzing [Lean 4](https://github.com/leanprover/lean4) using [AFL](https://github.com/google/AFL/).
So far, this has uncovered a few crashes ([1](https://github.com/leanprover/lean4/issues/283)
[2](https://github.com/leanprover/lean4/issues/297)
[3](https://github.com/leanprover/lean4/issues/301)
[4](https://github.com/leanprover/lean4/commit/c4cfbceb710c72dacb525ce658b9a57922dea912)
[5](https://github.com/leanprover/lean4/issues/302)) as well as a [soundness bug](https://leanprover-community.github.io/archive/stream/270676-lean4/topic/Contradiction.3F.html#224625352).

The Lean 4 interpreter is a very complex fuzzing target, and also very slow. I'm
always trying to think of ways to make fuzzing more effective. This repository
contains the various data files that I use in my fuzzing setup, as well as a dump
of many of the commands that need to be run to get the fuzzing going.

In its current state, this repository is unlikely to be of much use to anyone except
me. I'm thinking of writing a
blog post about my fuzzing setup and the results when I'm done fuzzing Lean 4.
If you'd be interested in reading something like this, please let me know (for
example by opening an issue in this repository).

## Building the post-processing library
```bash
gcc -shared -Wall -O3 post_library.c -o post_library.so
```

## Building the modified GMP
```bash
wget https://gmplib.org/download/gmp/gmp-6.2.1.tar.lz
tar -xvf gmp-6.2.1.tar.lz
cd gmp-6.2.1
patch -p1 < ../lean4-afl/0001-Turn-out-of-memory-crashes-into-hangs.patch
./configure --prefix=/home/markus/.local
make -j4
make check -j4
make install
```

## Building lean4 with modified GMP on the server
```bash
CC=/home/markus/AFL/afl-clang-fast CXX=/home/markus/AFL/afl-clang-fast++ AFL_INST_RATIO=10 LEAN_CXX=/home/markus/AFL/afl-clang-fast++ CMAKE_PREFIX_PATH=/home/markus/.local cmake -DCMAKE_INSTALL_PREFIX=/home/markus/lean4/build ..
CC=/home/markus/AFL/afl-clang-fast CXX=/home/markus/AFL/afl-clang-fast++ AFL_INST_RATIO=10 LEAN_CXX=/home/markus/AFL/afl-clang-fast++ CMAKE_PREFIX_PATH=/home/markus/.local make -j4
make install
```

## Building lean4 with modified GMP locally
```bash
CC=/home/markus/code/AFL/afl-clang-fast CXX=/home/markus/code/AFL/afl-clang-fast++ AFL_INST_RATIO=10 LEAN_CXX=/home/markus/code/AFL/afl-clang-fast++ CMAKE_PREFIX_PATH=/home/markus/.local cmake -DCMAKE_INSTALL_PREFIX=/home/markus/code/lean4/build ..
CC=/home/markus/code/AFL/afl-clang-fast CXX=/home/markus/code/AFL/afl-clang-fast++ AFL_INST_RATIO=10 LEAN_CXX=/home/markus/code/AFL/afl-clang-fast++ CMAKE_PREFIX_PATH=/home/markus/.local make -j16
make install
```

## Minimizing the corpus between fuzzing runs
On the server:
```bash
cd fuzzing/output/fuzzer01/queue
tar -cf ../../corpus.tar *
cd ../../fuzzer02/queue
tar -rf ../../corpus.tar *
cd ../../fuzzer03/queue
tar -rf ../../corpus.tar *
cd ../../fuzzer04/queue
tar -rf ../../corpus.tar *
cd ../..
lzip corpus.tar
```
Locally:
```bash
mkdir corpus
cd corpus
scp SERVER:fuzzing/output/corpus.tar.lz corpus.tar.lz
tar -xf corpus.tar.lz
rm corpus.tar.lz
cd ..
```

Optionally, add Lean 4 test suite to corpus:
```bash
find /home/markus/code/lean4/tests/ -name "*.lean" -exec cp {} corpus \;
```

```bash
mkdir corpus-min
/home/markus/code/AFL/afl-cmin -t200 -m 5000 -i corpus/ -o corpus-min -- /home/markus/code/lean4/build/bin/lean @@
```
This can fail with `no instrumentation output detected` if the first testcase in the
corpus folder is a hang.

```bash
cd corpus-min
tar --lzip -cf ../corpus-min.tar.lz *
cd ..
scp corpus-min.tar.lz SERVER:fuzzing
```

## Running the fuzzer

```bash
AFL_POST_LIBRARY=/home/markus/lean4-afl/post_library.so /home/markus/AFL/afl-fuzz -m 1500 -i corpus/ -o output/ -x /home/markus/lean4-afl/lean4.dict -M fuzzer01 -- /home/markus/lean4/build/bin/lean @@
AFL_POST_LIBRARY=/home/markus/lean4-afl/post_library.so /home/markus/AFL/afl-fuzz -m 1500 -i corpus/ -o output/ -x /home/markus/lean4-afl/lean4.dict -S fuzzer02 -- /home/markus/lean4/build/bin/lean @@
AFL_POST_LIBRARY=/home/markus/lean4-afl/post_library.so /home/markus/AFL/afl-fuzz -m 1500 -i corpus/ -o output/ -x /home/markus/lean4-afl/lean4.dict -S fuzzer03 -- /home/markus/lean4/build/bin/lean @@
AFL_POST_LIBRARY=/home/markus/lean4-afl/post_library.so /home/markus/AFL/afl-fuzz -m 1500 -i corpus/ -o output/ -x /home/markus/lean4-afl/lean4.dict -S fuzzer04 -- /home/markus/lean4/build/bin/lean @@
```
