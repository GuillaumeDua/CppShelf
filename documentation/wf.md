# Documentation : csl::wf

## Invocation utilities

### invoke

```cpp
constexpr auto func = []<typename T>(auto && value){};
func.template operator()<std::string>(42);  // regular function invocation

// std::invoke
std::invoke(&std::remove_cvref_t<decltype(func)>::template operator()<std::string, int>, func, 42);
//                                    notice the 2nd template-type-parameter here  ^^^

// csl::wf::invoke
csl::wf::invoke<std::string>(func, 42);         // template-type-parameters as invoke template-type-parameters
csl::wf::invoke(func, ttps<std::string>{}, 42); // or as 2nd optional parameters
```

Try it on [Godbolt here](https://godbolt.org/z/MbeqxEnG4)

### apply

```cpp

constexpr auto func = []<typename T>(auto && arg0, auto && arg1){};

// std::apply
using namespace std::placeholders;
std::apply(
    std::bind(
        &std::remove_cvref_t<decltype(func)>::template operator()<std::string, char, int>,
        func,
        _1, _2
    ), 
    std::tuple{'A', 42}
);

// csl::wf::apply
csl::wf::apply<std::string>(func, std::tuple{'A', 42});
csl::wf::apply(func, std::tuple{ ttps<std::string>{}, 'A', 42 });

```

### apply_before

```cpp
constexpr auto func = []<typename T>(auto && arg0, auto && arg1){};

// csl::wf::apply_before
csl::wf::apply_before(func, std::tuple{ ttps<std::string>{}, A{}, B{} });
csl::wf::apply_before(func, std::tuple{ ttps<std::string>{}, A{}},  B{});
csl::wf::apply_before(func, std::tuple{ ttps<std::string>{} }, A{}, B{} );

csl::wf::apply_before<std::string>(func, std::tuple{ A{}, B{} });
csl::wf::apply_before<std::string>(func, std::tuple{ A{} }, B{});
csl::wf::apply_before<std::string>(func, std::tuple{}, A{}, B{});
```

### apply_after

```cpp
constexpr auto func = []<typename T>(auto && arg0, auto && arg1){};

// csl::wf::apply_after
csl::wf::apply_after(func, std::tuple{ ttps<std::string>{}, A{}, B{} });

csl::wf::apply_after<std::string>(func, std::tuple{ A{}, B{} });
csl::wf::apply_after<std::string>(func, std::tuple{ B{} }, A{});
csl::wf::apply_after<std::string>(func, std::tuple{}, A{}, B{});
```