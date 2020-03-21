#### 1. About

This tool aims to find memory scrubbing operations, which might be eliminated/removed by compilers, particularly by the DSE(dead store elimination) pass at O1 or higher optimization levels.

Note that it is a GCC implementation version for the USENIX Security 2017 paper "**Dead Store Elimination (Still) Considered Harmful**", where a LLVM-based tool was designed and implemented.

##### Security problem we care about.

Developers might explictly **scrub** sensitive data from memory, via storing random values, after its last use so that a memory disclosure vulnerability could not reveal this data. (Please refer to `examples/1-memset-passwd.c` as an example. Also you may want to check out the detailed explaination presented in Section 1 of the USENIX Security 2017 paper).

However, DSE pass in compilers would remove these **memory scrubbing operations** since they have no effect on the program result, either because the stored value is overwritten or it is never read again.

It's a case of the "correctness-security gap" problem between the C standard and compilers.



#### 2. How to use the LLVM-based tool provided by the USENIX Security 2017 paper

##### Intallation

Download the code from the [repo](https://github.com/zhaomoy/Scrubbing-Safe-DSE) and install.

Note that the OS i'm using is Ubuntu 14.04.

```
cd PATH_TO_SEC17
git clone https://github.com/zhaomoy/Scrubbing-Safe-DSE.git
cd Scrubbing-Safe-DSE
mkdir build
cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=host ..
make -j 8
```

The tool is `PATH_TO_SEC17/Scrubbing-Safe-DSE/build/bin/clang`.

##### LLVM DSE pass.

Take `examples/1-memset-passwd.c` as an example.

```c
  1 #include <stdio.h>
  2 #include <string.h>
  3
  4 void main()
  5 {
  6     char pwd[20];
  7     char str[] = "password";
  8     printf("str is %s\n", str);
  9     printf("str len is %d\n", strlen(str));
 10
 11     // sensitive information, i.e. password, is stored in pwd array.
 12     memcpy(pwd, str, strlen(str));
 13     // use the password
 14     printf("pwd is %s\n", pwd);
 15
 16     // clear the sensitive information.
 17     memset(pwd, 0, sizeof(pwd));
 18     return;
 19 }
```

We can see that the `memset` at line 17 is a memory scubbing operation, but compilers may treat it as a deat store since the stored value is never used.

At O1 or higher optimization level, the DSE pass is activated and line 17 would be removed. We can check it by referring the IR produced by compilers. The following is the LLVM IR for `examples/1-memset-passwd.c`.

```bash
PATH_TO_SEC17/Scrubbing-Safe-DSE/build/bin/clang -O1 -S -emit-llvm examples/1-memset-passwd.c
cat 1-memset-passwd.ll
```

From the ll file, we can see the `memset` at line 17 in source code is indeed removed/optimized.

```
  1 ; ModuleID = 'examples/1-memset-passwd.c'
  2 source_filename = "examples/1-memset-passwd.c"
  3 target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
  4 target triple = "i686-pc-linux-gnu"
  5
  6 @main.str = private unnamed_addr constant [9 x i8] c"password\00", align 1
  7 @.str = private unnamed_addr constant [11 x i8] c"str is %s\0A\00", align 1
  8 @.str.1 = private unnamed_addr constant [15 x i8] c"str len is %d\0A\00", align 1
  9 @.str.2 = private unnamed_addr constant [11 x i8] c"pwd is %s\0A\00", align 1
 10
 11 ; Function Attrs: nounwind
 12 define void @main() local_unnamed_addr #0 {
 13   %1 = alloca [20 x i8], align 1
 14   %2 = alloca [9 x i8], align 1
 15   %3 = getelementptr inbounds [20 x i8], [20 x i8]* %1, i32 0, i32 0
 16   call void @llvm.lifetime.start(i64 20, i8* %3) #4
 17   %4 = getelementptr inbounds [9 x i8], [9 x i8]* %2, i32 0, i32 0
 18   call void @llvm.lifetime.start(i64 9, i8* %4) #4
 19   call void @llvm.memcpy.p0i8.p0i8.i32(i8* %4, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @main.str, i32 0, i32 0), i32 9
    , i32 1, i1 false)
 20   %5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i32 0, i32 0), i8* %4)
 21   %6 = call i32 @strlen(i8* %4) #5
 22   %7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str.1, i32 0, i32 0), i32 %6)
 23   %8 = call i32 @strlen(i8* %4) #5
 24   call void @llvm.memcpy.p0i8.p0i8.i32(i8* %3, i8* %4, i32 %8, i32 1, i1 false)
 25   %9 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.2, i32 0, i32 0), i8* %3)
 26   call void @llvm.lifetime.end(i64 9, i8* %4) #4
 27   call void @llvm.lifetime.end(i64 20, i8* %3) #4
 28   ret void
 29 }
 30
 31 ; Function Attrs: argmemonly nounwind
```

##### Tool in USENIX Security 2017 paper

The tool, named `Scrubbing Finder`, can be activated by providing compilcation options for LLVM, i.e. `-fsanitize=sec-dse -fsanitize=byte-counter`

```bash
root@shqking:~/code/safe-dse-gcc# ./dse-llvm/bin/clang -O1 -fsanitize=sec-dse -fsanitize=byte-counter examples/1-memset-passwd.c -o a
examples/1-memset-passwd.c:4:1: warning: return type of 'main' is not 'int' [-Wmain-return-type]
void main()
^
examples/1-memset-passwd.c:4:1: note: change return type to 'int'
void main()
^~~~
int
1 warning generated.
root@shqking:~/code/safe-dse-gcc# ./a
str is password
str len is 8
pwd is password
               

[secdse] keep 20 bytes of store
```

From the log we can see that,

- No message about whether a memory scrubbing operation would be removed by compiler, is provided at the <u>compilation</u> phase by `Scrubbing Finder`
- At <u>runtime</u> phase, `Scrubbing Finder` dumps how many bytes it prevents the compiler from removing.

Note that the implementation details of `Scrubbing Finder` can be found in `Scrubbing-Safe-DSE/lib/Transforms/Scalar/DeadStoreElimination.cpp` if you are interested in.

**Note that** as discussed in Appendix A in the Security'17 paper, Scrubbing Finder differentiates removed scrubs from other dead stores, that is 

1. a store that is overwritted by another store with no read in between, which means the first store is used as a cleaning initialization. Please refer to cases 2-4 under `example` directory as an example.
2. the stored value and the number of bytes stores should be a constant, as discussed in Section 7.1. Please refer to case 5-6 under `example` directory as an example.

##### A patch we provide

We give a patch (see `code/usenix17.diff`) for `Scrubbing Finder` tool. 

This patch would show detailed information for each **will-be-removed** memory scrubbing operation.

```bash
root@shqking:~/code/safe-dse-gcc# ./dse-llvm/bin/clang -O1 -fsanitize=sec-dse -fsanitize=byte-counter examples/1-memset-passwd.c -o a
examples/1-memset-passwd.c:4:1: warning: return type of 'main' is not 'int' [-Wmain-return-type]
void main()
^
examples/1-memset-passwd.c:4:1: note: change return type to 'int'
void main()
^~~~
int
--------- possible scrubbing removal ---------
Location:
Removed IR Instruction:   call void @llvm.memset.p0i8.i32(i8* %3, i8 0, i32 20, i32 1, i1 false)
Additional Info:
--------- possible scrubbing removal ---------
Location:
Removed IR Instruction:   call void @llvm.memset.p0i8.i32(i8* %3, i8 0, i32 20, i32 1, i1 false)
Additional Info:
1 warning generated.
```

TODO: we will figure out whether the log is duplicate.



#### 3. Our GCC version implementation

We hacked the DSE pass inside GCC compiler, i.e. `gcc/tree-ssa-dse.c` file.

> /*Dead store elimination is fundamentally a walk of the post-dominator tree and a backwards walk of statements within each block. */

 `code/gcc-safe-dse.diff` is a patch for GCC-7.4.0. 

Our tool can

- Run by default along gcc-7.4.0. TODO: we may add a option flag.
- dump the removed scrubs into a temp file (`/tmp/sec-dse.log`) at compilation phase, incluing the location and correspoing source code.
- differentiate removed scrubs from other dead store, as `Scrubbing Finder` does, that is, excluding (a) initializaiton, (b) variable stored value, (c) variable number of bytes stored.

As for the motivating example, our tool would produce the following log.

```bash
root@shqking:~/code/safe-dse-gcc# ls /tmp/sec-dse.log
ls: cannot access /tmp/sec-dse.log: No such file or directory
root@shqking:~/code/safe-dse-gcc# ../gcc74/dest/bin/gcc -O1 examples/1-memset-passwd.c
root@shqking:~/code/safe-dse-gcc# cat /tmp/sec-dse.log
memset (&pwd, 0, 20);
file:examples/1-memset-passwd.c
line number:17

```



#### 4. Test cases

We present 6 test cases under `example` directory. As shown in the follwing table:

- Column 2: only case 1 is our target. Case 2-4 are memroy initialization, and cases 5-6 use variable value or bytes cout.
- Column 3-4: theoretically both GCC and LLVM would remove all the memory scrubbing operations in these 6 cases at O1 optimization level. However, both GCC and LLVM miss case 2 and case 4.
- Column 5-6: as for the removed scrubs (case 1, 3, 5, 6), both `Scrubbing Finder` and our tool can distinguish our target and other dead store, i.e. dumpping logs for our target. 

| Case | Scrubbing Op    | GCC O1 | LLVM O1 | Scrubbing Finder | Our Tool |
| ---- | --------------- | ------ | ------- | ---------------- | -------- |
| 1    | **our target**  | remove | remove  | Log              | Log      |
| 2    | init via for    | keep   | keep    | -                | -        |
| 3    | init via memcpy | remove | remove  | -                | -        |
| 4    | init via memset | keep   | Keep    | -                | -        |
| 5    | Var. number     | remove | Remove  | -                | -        |
| 6    | Var. value      | remove | Remove  | -                | -        |



#### 5. Potential bugs we have found

Note that the main reason we implemented this GCC version tool is that not all software/projects can be easily built using LLVM as the toolchain.

We have applied our tool to several open-source projects, such as linux kernel and glibc. And we found several potential bugs, i.e. memory srcubbing operations for sensitive information are used in the projects. More information will be discussed once the potential bugs get confirmed by the developers.



------

Please feel free to contact with me (shqking@gmail.com) if you have any problem.
