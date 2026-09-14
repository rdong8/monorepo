# AGENTS.md

Always means using the most correct, most modern, most powerful tool for the job. There is no such thing as overkill.

## Performance

- Annotate hot functions with `[[gnu::hot]]`

## Library Usage

- Avoid "rolling your own". Check Abseil, Boost, Folly and LLVM's Support library first.
- You should only ever use the `std::ranges`-based algorithms and range adapters when possible.
- Use the most appropriate container for the job
  - `std::map`/`std::unordered_map` is only appropriate when stable iterators are needed
  - Never use C-style arrays
  - `std::inplace_vector` or `llvm::SmallVector` should be preferred over an array + size
- Use `std::reference_wrapper` for non-nullable handles where a raw reference cannot be used (ie. for use in containers). Don't use a raw pointer unless it's actually nullable.
- Never use `std::function`. Instead:
  - Use `std::copyable_function`, `std::move_only_function` or `std::function_ref`
  - Only use a raw function pointer in the rare scenario that the 16 byte overhead of `std::function_ref` is unacceptable

## Concurrency

- Use the following criteria to align groups of variables by `std::hardware_destructive_interference_size`:
  - Thread of access
  - Mutability (MESI)
  - Hotness
  - Logical/program grouping (related variables together)
- For extremely performance sensitive code, consider aligning by `2 * std::hardware_destructive_interference_size` to avoid being impacted by adjacent-line prefetching on modern processors
- All new async code should use one of the following frameworks:
  - Nvidia's `<execution>` implementation (`bazel_dep(name = "stdexec)`) if possible
  - Boost.Cobalt if you need to use an existing Asio-based library (ie. Boost.Redis)
- Consider modern concurrency primitives like `std::counting_semaphore`, `std::latch`, `std::barrier`, etc. for multithreaded code

## Wire Serde

- Use `[[gnu::packed]]` to pack structs
- Use `[[no_unique_address]]` if a member of a wire struct is possibly empty, most often a templated member of a struct. This ensures you don't accidentally send an uninitialized byte from a size 1 empty struct over the wire.
- Use `<bit>` for bit manipulation

## Style

### Modules

We follow [best practices](https://chuanqixu9.github.io/c++/2025/12/30/C++20-Modules-Best-Practices.en.html#modules-native-best-practices) for C++20 modules:

- A subproject should declare only 1 module, and use module partition units for multiple TUs
- Use module implementation partition units, not module implementation units, to implement thiungs declared in the interface

### Variable Initialization

We use the following convention for variable initialization.

When initializing a variable from an expression, use `auto` and `=`.

```cpp
auto const x = get_thing();
auto const &y = get_handle();
```

When calling a constructor, specify the type and use direct initialization:

```cpp
std::span const span{pointer, size};

// Notice we recursively follow the convention for each clause in the designated initializer
Person const person{.name{"Richard"}, .age = 22};
```

Note the following edge cases:

```cpp
// When trying to avoid an initializer list ctor, it is acceptable to use copy initialization
std::vector<std::size_t> ones(100UZ, 1UZ);

// For MILs, always use direct initialization except to avoid initializer list ctors
Buffer::Buffer()
    : data{}
    , size{100UZ}
{
}

// WRONG: do not create a temporary and rely on RVO
// auto const name = std::string{"Richard"};
// auto const default_name = std::string{};
// Correct way:
std::string const name{"Richard"};
std::string const default_name{};

// Consequence of the rule: implicit conversions are NOT allowed
// WRONG: initializing from expression but not using `auto`
// int x = 16U;
// Correct way:
auto x = static_cast<int>(16U);
```

### Variable Naming

Almost never use abbreviated/undescriptive variable names (ie. `i` instead of `index` for a loop variable). We don't to end up with looking like the unreadable C code found in the Linux kernel. Examples of unacceptable abbreviations:

- "Order book" -> `ob`
- "Transmit" -> `xmit`
- "Exception handler" -> `eh`

Only extremely common acronyms are acceptable, most frequently acronyms that would be understood outside of the context of code.

### Spacing

Space expressions with many binary operators by breaking *before* the operator:

```cpp
self.full_class_name =                                                    //
    std::views::concat(self.package, std::views::single(node.identifier)) //
    | std::views::join_with('.')                                          //
    | std::ranges::to<std::string>();
```

For direct initializers, this can be done with a trailing comma:

```cpp
Person person{
    .name{"Richard"},
    .age = 22,
    .vehicle = Vehicle::Car,
    .school = School::University,
    // ...
    .time_zone = time_zone,
}
```

Always leave a blank line before and after multiline statements, including the braced bodies of if/while/for statements. The exception is when a preceding/following line is at a lower identation level.

```cpp
if (has_error())
{
    return; // Surrounding braces at a lower indentation level, blank lines not needed
}
//< This blank line is mandatory since the previous statement is multiline
Person person{
    .name{"Richard"},
    .age = 22,
    .vehicle = Vehicle::Car,
};
//< This blank line is mandatory since the previous statement is multiline
add_person(person);
```

If you want to group statements by concern, use a block or immediately invoked lambda instead of squishing lines together with no spacing.

### Comments

- Comments should be brief if present at all
- In general, verbose comments imply insufficiently verbose code
- Comments should not justify what the code is doing (unless it is non-obvious), but why it is doing it

### Other

- Always use the explicit object parameter/deducing this
- There's a clang-format pre-commit hook. You never need to run it manually.

## Bazel

- Use `implementation_deps` for C++ targets
- Never rely on transitive dependencies
- Use the fish shell for all shell targets. Use `argparse` and named arguments only.
- Avoid the system compiler for experimentation/testing. Make a temporary bazel target to play around with, ie. [`//src/tmp:tmp`](src/tmp/BUILD.bazel)

## Testing

- Use module implementation partition units to write unit tests. This allows testing of internal APIs of a project that may not be `export`'ed.
- Tests belong in an anonymous namespace nested in the namespace under test.
- If you need to test hidden members of a class, add the `-fno-access-control` flag to the test instead of incorrectly making a hidden member public

## Test Selection

Do not write stupid tests. Examples:

- Don't write an assertion with constants for both operands, ie. `static_assert(sizeof(MessageFrame) == 8UZ)`. If this is required by some protocol, then it belongs next to the struct declaration, not in the unit tests. Else you're just comparing a compile time constant against a magic number.

## Meta

- Avoid making compromises without consulting with me.
- Avoid workarounds/hacks, again consult with me first.
  - For example, for third party dependencies, I often prefer to patch them upstream instead of adding workarounds in my own code.
- PUSH BACK if you're prompted to do something that would lead to compromises.
