![Acorn - Elegant and Simple Programming Language](logo.png)

![Travis Build Status](https://img.shields.io/travis/thomasleese/acorn.svg)

> Acorn is a elegant, minimalistic, high-level programming language implemented
> in C++ with LLVM. It borrows ideas from MATLAB, Julia, Python, Ruby and many
> other languages. It supports features such as multiple-dispatch, operating
> overloading, static typesystem with inference and concurrency.

## What does it look like?

```ruby
def hello
  print("Hello, world!")
end

hello()
```

## How to build?

```bash
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ cd ..
$ cmake --build build --config Debug
```

## How to use?

```bash
$ ./build/src/acornc test.acorn
$ ./test
```
