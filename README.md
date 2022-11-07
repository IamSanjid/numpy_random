# numpy_random
Simple C++ thread-safe interface to use NumPy's Random Legacy distributions directly.
There's no extra dependency required. Just build and grab the `src/numpy_random.h` and link `libnumpyrandom` static library to your project.

# Usage
```c++
template <typename RngEngine>
class RandomState { /*..*/ }
```

`RandomState` accepts `RngEngine` which should be a Random Engine implementation type and must implement `operator()` to return it's next state. It can return as any of the default C++ arithmetic types or custom arithmetic type(custom `uint128_t`) and must implement `operator>>`, `operator&` and also should be castable to other C++ default integral types. The `RngEngine` can also return it's next state as arithmetic container type (eg. returning an array of `uint32_t`), the container type must implement `operator[index]` and **it is recommended that `size_t size()` should be also implemented otherwise the container's size will be determined using unsafe way which will probably only work for Stack Arrays.** 

The arithmetic container type implementation probably will be a little bit slow compared to raw arithmetic types.

# Example
Using the standard library's `std::mt19937` Random Implementation.

```c++
#include <iostream>
#include <random>
#include "numpy_random.h"

int main() {
  auto random = RandomState<std::mt19937>();
  random.get_engine().seed(0);
  
  std::cout << random.rand_int(0, 9) << std::endl;
  std::cout << random.rand_int(0, 9) << std::endl;
  std::cout << random.rand_int(0, 9) << std::endl;
  std::cout << random.rand_int(0, 9) << std::endl;
  
  return 0;
}
```
Here `get_engine()` returns the direct reference to the engine instance, it is thread-safe, uses simple mutex mechanism to do so.

Using [official pcg random generator](https://github.com/imneme/pcg-cpp).
```c++
#include <iostream>
#include <pcg_random.hpp>
#include "numpy_random.h"

int main() {
  NumpySeedSequence<uint64_t> seed_source(0);
  auto random = RandomState<pcg64>();
  random.get_engine().seed(seed_source);
  
  std::cout << random.rand_int(0, 9) << std::endl;
  std::cout << random.rand_int(0, 9) << std::endl;
  std::cout << random.rand_int(0, 9) << std::endl;
  std::cout << random.rand_int(0, 9) << std::endl;
  
  return 0;
}
```

Here `NumpySeedSequence` is a simple implmentation of NumPy Random's [SeedSequences](https://github.com/numpy/numpy/blob/fcafb6560e37c948a594dce36d300888148bc599/numpy/random/bit_generator.pyx#L246) with slightly different behaviour. This was used to generate an initial state for `pcg` implementation. The implementation may feel little bit wrong because of the endianness. May be try to use `std::reverse` to tempo fix it :).

```c++
template <typename result_type = unsigned int, size_t pool_size = 4>
class NumpySeedSequence { /*...*/ }
```
Please check [SeedSequences](https://github.com/numpy/numpy/blob/fcafb6560e37c948a594dce36d300888148bc599/numpy/random/bit_generator.pyx#L246) to understand how this works.

The `result_type` can be either `uint32_t` or `uint64_t`.
```c++
template <typename DestIter>
void generate(DestIter start, DestIter finish) { /*...*/ }
result_type operator()() { /*...*/ }
```

* `operator()` returns the next state.
* `generate` returns a chunk of states, if the result type is `uint64_t` and the destination container type's size is less than `uint64_t` then it tries to follow roughly something like [this](https://github.com/numpy/numpy/blob/fcafb6560e37c948a594dce36d300888148bc599/numpy/random/bit_generator.pyx#L440), so basically we just reverse the bytes after converting to native endianness, which I know feels wrong but don't know *yet* how to handle it or just lazy to think about it right now as of writing :). So the behavior might get changed in the future.

Note: NumPy's `SeedSequence` is slightly different, NumPy's implementation resets it's hash after every `n_words` generation which slightly feels wrong, so this implementation doesn't reset it. In order to get the same behaviour as the NumPy's implementation don't reuse the same instance just create a new instance after every generation. (Again this should be used to set the initial state of a Random Engine so one time use should be enough.)

# FIN
All credits go to NumPy developers, this was actually a learning to project to learn about `template`s. Some things may get broken, please open an issue and help to improve ourselves.