# How to compile

To compile, one needs a POSIX standard `make` program and a C compiler
with the name `gcc`.  If one wishes to use another C compiler, edit the
file `Makefile` and change the line `CC= gcc` to use the compiler in
question.

To compile on a Linux or compatible system (e.g. Cygwin) with readline
support (so, when invoked from a terminal, one has arrow history):

```
	make -f Makefile.readline
```

To compile this on a Mingw system:

```
	make -f Makefile.mingw32
```

The code is compatible with gcc (gcc 3.4.2 and gcc 11.3.0), clang (clang 
8.0.1), and will hopefully compile in other compilers, including C++ 
compilers, without issue.
