# Documentation : csl::wf

## invoke

```cpp
constexpr auto func = []<typename T>(auto && value){};
func.template operator()<std::string>(42);

// STL
std::invoke(&std::remove_cvref_t<decltype(func)>::template operator()<std::string, int>, func, 42);
//                                     notice the 2nd template-type-paramter here  ^^^

// csl::wf
csl::wf::invoke<std::string>(func, 42);
csl::wf::invoke(func, ttps<std::string>{}, 42);
```

<iframe width="800px" height="200px" src="https://tinyurl.com/7pdtw2nt"></iframe>

Try it on [Godbolt here](https://godbolt.org/z/MbeqxEnG4)

