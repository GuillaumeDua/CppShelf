
<div style="position: absolute; top: 0; right: 0;">
  <a href="https://github.com/GuillaumeDua/CppShelf">
    <img loading="lazy" width="149" height="149" src="https://github.blog/wp-content/uploads/2008/12/forkme_right_red_aa0000.png?resize=149%2C149" class="attachment-full size-full" alt="Fork me on GitHub" data-recalc-dims="1" align="right"
    style="position: relative; top: 0; right: 0; z-index: 1;">
  </a>
  <a href="https://guillaumedua.github.io/CppShelf/">
    <img loading="lazy" src="gh-pages-logo.png" alt="Check documentation on GitHub-pages" align="right" class="attachment-full size-full" height="73"
    style="position: absolute; top: 0; right: 0; z-index: 3;"
    >
  </a>
</div>

# Overall presentation

<div class='awesome-doc-code-sections_code-section'>
<pre><code>
// some comment here
// ...

// @awesome-doc-code-sections::CE={
//  "language"            : "c++",
//  "compiler_id"         : "clang1400",
//  "compilation_options" : "-O2 -std=c++20",
//  "libs"                : [ {"id": "fmt", "version": "trunk"} ],
//  "includes_transformation" : [
//        [ "csl/",       "https://raw.githubusercontent.com/GuillaumeDua/CppShelf/main/includes/ag/csl/" ],
//        [ "toto",       "iostream" ]
//  ],
//  "add_in_doc_execution" : true
//  }
// #include "csl/ag.hpp"
#include <csl/ag.hpp> // @awesome-doc-code-sections::show::line

auto main() -> int {
// @awesome-doc-code-sections::show::block::begin

auto i = 42; // test
return i;
// @awesome-doc-code-sections::show::block::end
}
</code></pre>
</div>

<br />

<div class='awesome-doc-code-sections_code-section'>
<pre><code>
// some comment here
// ...

// @awesome-doc-code-sections::CE={
//  "language"            : "c++",
//  "compiler_id"         : "clang1400",
//  "compilation_options" : "-O2 -std=c++20",
//  "libs"                : [ {"id": "fmt", "version": "trunk"} ],
//  "includes_transformation" : [
//        [ "csl/",       "https://raw.githubusercontent.com/GuillaumeDua/CppShelf/main/includes/ag/csl/" ],
//        [ "toto",       "iostream" ]
//  ],
//  "add_in_doc_execution" : true
//  }

#include <csl/ag.hpp> // @awesome-doc-code-sections::show::line
#include <iostream>

auto main() -> int {
// @awesome-doc-code-sections::show::block::begin
std::cout << "a\nb\nc\nd\ne\nf";
std::cerr << "some stderr output\n";
std::cout << "a\nb\nc\nd\ne\nf";
auto i = 42; // this is a pretty long comment, don't you think so ?
return i;
// @awesome-doc-code-sections::show::block::end
}

</code></pre>
</div>

<br />

<div class='awesome-doc-code-sections_code-section' language="cpp">
<pre><code>
// some comment here
// ...

// @awesome-doc-code-sections::CE={
//  "add_in_doc_execution" : true
//  }


auto main() -> int {
  return 42;
}

BAD CODE // @awesome-doc-code-sections::skip::line 

// @awesome-doc-code-sections::skip::block::begin
// skip me !
// and me !
// and me too !
// @awesome-doc-code-sections::skip::block::end

</code></pre>
</div>

<br />

<div class='awesome-doc-code-sections_basic-code-section' language="cpp">
<pre><code>
// some comment here
// ...

// @awesome-doc-code-sections::CE={
//  "language"            : "c++",
//  "compiler_id"         : "clang1400",
//  "compilation_options" : "-O2 -std=c++20",
//  "libs"                : [ {"id": "fmt", "version": "trunk"} ],
//  "includes_transformation" : [
//        [ "csl/",       "https://raw.githubusercontent.com/GuillaumeDua/CppShelf/main/includes/ag/csl/" ]
//  ]
//  }
#include "csl/ag.hpp" // @awesome-doc-code-sections::show::line

auto main() -> int {
// @awesome-doc-code-sections::show::block::begin
auto i = 42; // test
return i;
// @awesome-doc-code-sections::show::block::end
}
</code></pre>
</div>