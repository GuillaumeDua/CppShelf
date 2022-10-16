// MIT License
//
// Copyright (c) 2021-2022 Guillaume Dua "Guss"
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// highlightjs + doxygen-awesome-css + dark/light-mode == <3
//  prerequisites : doxygen-awesome-css-darkmode-toggle
//  prerequisites : highlightjs

if (typeof hljs === 'undefined')
    console.error('awesome-doc-code-sections_dark-mode.js: depends on highlightjs, which is missing')

if (typeof awesome_doc_code_sections === 'undefined')
    console.error('awesome-doc-code-sections_dark-mode.js: depends on awesome-doc-code-sections.js, which is missing')

// ============

// light/dark theme switch
awesome_doc_code_sections.ThemeSelector.can_handle_darkMode = true
awesome_doc_code_sections.ThemeSelector.toggleDarkLightMode_onTheme = function(isDarkModeEnabled) {
    
    let code_stylesheet = document.getElementById(awesome_doc_code_sections.ThemeSelector.stylesheet_HTML_placeholder_id);
    if (isDarkModeEnabled)
        code_stylesheet.href = code_stylesheet.href.replace('-light', '-dark')
    else
        code_stylesheet.href = code_stylesheet.href.replace('-dark', '-light')
}
// Toggle dark/light mode : doxygen-awesome-css & awesome-doc-code-sections inter-operability/compatibility
awesome_doc_code_sections.ToggleDarkMode.updateToggleIcons = function(isDarkModeEnabled) {
    
    if (awesome_doc_code_sections.options.doxygen_awesome_css_compatibility
    &&  typeof DoxygenAwesomeDarkModeToggle !== 'undefined')
    {
        $("body").find("doxygen-awesome-dark-mode-toggle").each((index, value) => {
            DoxygenAwesomeDarkModeToggle.darkModeEnabled = isDarkModeEnabled
            value.updateIcon()
        })
        $("body").find('button[is="awesome-doc-code-sections_toggle-dark-mode-button"]').each((index, value) => {
            awesome_doc_code_sections.ToggleDarkMode.darkModeEnabled = isDarkModeEnabled
            value.updateIcon()
        })
    }
}

// ============

awesome_doc_code_sections.onHTMLClassChange_updateDarkLightMode = function(mutationsList, observer) {
// TODO: only last mutation matters ?

    mutationsList.forEach(mutation => {

        if (mutation.attributeName !== 'class')
            return;
        if (mutation.oldValue !== null &&
            mutation.oldValue !== "dark-mode" &&
            mutation.oldValue !== "light-mode")
            return;
        let html_node = document.getElementsByTagName('html')[0];
        if (mutation.oldValue === html_node.className)
            return;

        console.log('awesome-doc-code-sections_dark-mode.js:onHTMLClassChange: Switching theme from : ' + mutation.oldValue + ' to ' + html_node.className);

        let isDarkModeEnabled = (html_node.className === 'dark-mode')

        awesome_doc_code_sections.ThemeSelector.toggleDarkLightMode_onTheme(isDarkModeEnabled)
        awesome_doc_code_sections.ToggleDarkMode.updateToggleIcons(isDarkModeEnabled)
    })
}
const html_class_mutationObserver = new MutationObserver(awesome_doc_code_sections.onHTMLClassChange_updateDarkLightMode);
html_class_mutationObserver.observe(
    document.getElementsByTagName('html')[0],
    { 
        attributes: true,
        attributeFilter: [ 'class' ],
        attributeOldValue: true
    }
);


// TODO: move other related stuffs here ...