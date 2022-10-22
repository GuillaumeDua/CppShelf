
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
//  "language"            : "cpp",
//  "compiler_id"         : "clang1400",
//  "compilation_options" : "-O2 -std=c++20",
//  "libs"                : [ "fmt" ],
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

