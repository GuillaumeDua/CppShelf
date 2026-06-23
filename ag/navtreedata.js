/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "📦 csl::ag", "index.html", [
    [ "Overall presentation", "index.html#overall-presentation", [
      [ "Overview demo", "index.html#overview-demo", null ],
      [ "Introduction", "index.html#introduction", null ],
      [ "Philosophy &amp; design choices", "index.html#philosophy--design-choices", null ],
      [ "Getting starting", "index.html#getting-starting", [
        [ "Integration", "index.html#integration", [
          [ "Plain download", "index.html#plain-download", null ],
          [ "CMake", "index.html#cmake", null ]
        ] ],
        [ "Configuration", "index.html#configuration", [
          [ "Bitfields support", "index.html#bitfields-support", null ],
          [ "Highier limit for aggregate field count", "index.html#highier-limit-for-aggregate-field-count", null ],
          [ "Formatting and printing (experimentale)", "index.html#formatting-and-printing-experimentale", null ]
        ] ]
      ] ],
      [ "Content", "index.html#content", [
        [ "Concepts", "index.html#concepts", [
          [ "unqualified_aggregate&lt;T&gt;", "index.html#unqualified_aggregatet", null ],
          [ "aggregate&lt;T&gt;", "index.html#aggregatet", null ],
          [ "aggregate_constructible_from&lt;T, args_ts...&gt;", "index.html#aggregate_constructible_fromt-args_ts", null ],
          [ "aggregate_constructible_from_n_values&lt;T, std::size_t N&gt;", "index.html#aggregate_constructible_from_n_valuest-stdsize_t-n", null ],
          [ "tuplelike&lt;T&gt;", "index.html#tupleliket", null ],
          [ "csl::ag::concepts::structured_bindable&lt;T&gt;", "index.html#cslagconceptsstructured_bindablet", null ]
        ] ],
        [ "Type-traits", "index.html#type-traits", [
          [ "csl::ag::size&lt;T&gt;", "index.html#cslagsizet", null ],
          [ "csl::ag::element&lt;std::size_t, concepts::aggregate&gt;", "index.html#cslagelementstdsize_t-conceptsaggregate", null ],
          [ "csl::ag::view_element&lt;std::size_t, concepts::aggregate&gt;", "index.html#cslagview_elementstdsize_t-conceptsaggregate", null ]
        ] ],
        [ "to-tuple conversion", "index.html#to-tuple-conversion", [
          [ "Owning conversion", "index.html#owning-conversion", null ],
          [ "Non-owning conversion (view, lightweight accessor)", "index.html#non-owning-conversion-view-lightweight-accessor", null ]
        ] ],
        [ "tuplelike interface", "index.html#tuplelike-interface", [
          [ "std::tuple_element", "index.html#stdtuple_element", null ],
          [ "std::get", "index.html#stdget", null ]
        ] ],
        [ "Functional API", "index.html#functional-api", [
          [ "csl::ag::apply", "index.html#cslagapply", null ],
          [ "csl::ag::for_each", "index.html#cslagfor_each", null ]
        ] ],
        [ "Formatting and printing", "index.html#formatting-and-printing", [
          [ "using std::format", "index.html#using-stdformat", null ],
          [ "using fmt", "index.html#using-fmt", null ],
          [ "using std::ostream", "index.html#using-stdostream", null ],
          [ "using <span class=\"tt\">to_string&lt;format_options&gt;</span>", "index.html#using-to_stringformat_options", null ]
        ] ]
      ] ],
      [ "Homogeneity API with tuple-likes", "index.html#homogeneity-api-with-tuple-likes", null ],
      [ "Current limitations", "index.html#current-limitations", null ],
      [ "(Internal details) Where's the magic ?", "index.html#internal-details-wheres-the-magic-", [
        [ "Fast path - for default_initializable&lt;T&gt; without bitfield support", "index.html#fast-path---for-default_initializablet-without-bitfield-support", null ],
        [ "Slow path - for non default_initializable&lt;T&gt; or when bitfield support is enabled", "index.html#slow-path---for-non-default_initializablet-or-when-bitfield-support-is-enabled", null ]
      ] ],
      [ "Example", "index.html#example", null ]
    ] ],
    [ "Concepts", "concepts.html", "concepts" ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html"
];

const SYNCONMSG = 'click to disable panel synchronization';
const SYNCOFFMSG = 'click to enable panel synchronization';
const LISTOFALLMEMBERS = 'List of all members';