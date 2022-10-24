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

class ToggleDarkMode /*StaticObserver*/ {
// Handle dark/light mode info,
// altering document class-list by adding `dark-mode` or `light-mode`
//
//  Note: DoxygenAwesomeDarkModeToggle is a great alternative to this,
//        that you should use instead when using awesome-doc-code-sections in conjunction with DoxygenAwesomeCSS
//        Both can be used at the same time though (compatiblity/inter-operability is a default feature)

    static prefersLightModeInDarkModeKey    = "prefers-light-mode-in-dark-mode"
    static prefersDarkModeInLightModeKey    = "prefers-dark-mode-in-light-mode"

    static initialize = function() {

        console.log(`awesome-doc-code-sections.js: ToggleDarkMode:initialize ...`)

        // if (DoxygenAwesomeDarkModeToggle !== undefined) {
        //     console.error('awesome-doc-code-sections.js:initialize: toggle light/dark mode : conflict with DoxygenAwesomeDarkModeToggle detected, aborting')
        // }

        ToggleDarkMode.enableDarkMode(ToggleDarkMode.userPreference)
        window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', event => {
            ToggleDarkMode.onSystemPreferenceChanged()
        })
        document.addEventListener("visibilitychange", visibilityState => {
            if (document.visibilityState === 'visible') {
                ToggleDarkMode.onSystemPreferenceChanged()
            }
        });
    }

    static enableDarkMode(enable) {
        if (ToggleDarkMode.darkModeEnabled = enable) {
            document.documentElement.classList.add("dark-mode")
            document.documentElement.classList.remove("light-mode")
        }
        else {
            document.documentElement.classList.remove("dark-mode")
            document.documentElement.classList.add("light-mode")
        }
    }

    static get systemPreference() {
        return window.matchMedia('(prefers-color-scheme: dark)').matches
    }

    static get userPreference() {
        return (
            !ToggleDarkMode.systemPreference &&  localStorage.getItem(ToggleDarkMode.prefersDarkModeInLightModeKey)
        ||   ToggleDarkMode.systemPreference && !localStorage.getItem(ToggleDarkMode.prefersLightModeInDarkModeKey)
        );
    }
    static set userPreference(userPreference) {

        ToggleDarkMode.darkModeEnabled = userPreference

        if (!userPreference) {
            if (ToggleDarkMode.systemPreference)
                localStorage.setItem(ToggleDarkMode.prefersLightModeInDarkModeKey, true)
            else
                localStorage.removeItem(ToggleDarkMode.prefersDarkModeInLightModeKey)
        }
        else {
            if (!ToggleDarkMode.systemPreference)
                localStorage.setItem(ToggleDarkMode.prefersDarkModeInLightModeKey, true)
            else
                localStorage.removeItem(ToggleDarkMode.prefersLightModeInDarkModeKey)
        }
        ToggleDarkMode.onUserPreferenceChanged()
    }

    static onSystemPreferenceChanged() {
        ToggleDarkMode.darkModeEnabled = ToggleDarkMode.userPreference
        ToggleDarkMode.enableDarkMode(ToggleDarkMode.darkModeEnabled)
    }
    static onUserPreferenceChanged() {
        ToggleDarkMode.enableDarkMode(ToggleDarkMode.darkModeEnabled)
    }
}
awesome_doc_code_sections.ToggleDarkMode = ToggleDarkMode
class ToggleDarkModeButton extends HTMLButtonElement {

    static HTMLElement_name                 = "awesome-doc-code-sections_toggle-dark-mode-button"

    static lightModeIcon                    = `<svg xmlns="http://www.w3.org/2000/svg" enable-background="new 0 0 24 24" height="24px" viewBox="0 0 24 24" width="24px" fill="#FCBF00"><rect fill="none" height="24" width="24"/><circle cx="12" cy="12" opacity=".3" r="3"/><path d="M12,9c1.65,0,3,1.35,3,3s-1.35,3-3,3s-3-1.35-3-3S10.35,9,12,9 M12,7c-2.76,0-5,2.24-5,5s2.24,5,5,5s5-2.24,5-5 S14.76,7,12,7L12,7z M2,13l2,0c0.55,0,1-0.45,1-1s-0.45-1-1-1l-2,0c-0.55,0-1,0.45-1,1S1.45,13,2,13z M20,13l2,0c0.55,0,1-0.45,1-1 s-0.45-1-1-1l-2,0c-0.55,0-1,0.45-1,1S19.45,13,20,13z M11,2v2c0,0.55,0.45,1,1,1s1-0.45,1-1V2c0-0.55-0.45-1-1-1S11,1.45,11,2z M11,20v2c0,0.55,0.45,1,1,1s1-0.45,1-1v-2c0-0.55-0.45-1-1-1C11.45,19,11,19.45,11,20z M5.99,4.58c-0.39-0.39-1.03-0.39-1.41,0 c-0.39,0.39-0.39,1.03,0,1.41l1.06,1.06c0.39,0.39,1.03,0.39,1.41,0s0.39-1.03,0-1.41L5.99,4.58z M18.36,16.95 c-0.39-0.39-1.03-0.39-1.41,0c-0.39,0.39-0.39,1.03,0,1.41l1.06,1.06c0.39,0.39,1.03,0.39,1.41,0c0.39-0.39,0.39-1.03,0-1.41 L18.36,16.95z M19.42,5.99c0.39-0.39,0.39-1.03,0-1.41c-0.39-0.39-1.03-0.39-1.41,0l-1.06,1.06c-0.39,0.39-0.39,1.03,0,1.41 s1.03,0.39,1.41,0L19.42,5.99z M7.05,18.36c0.39-0.39,0.39-1.03,0-1.41c-0.39-0.39-1.03-0.39-1.41,0l-1.06,1.06 c-0.39,0.39-0.39,1.03,0,1.41s1.03,0.39,1.41,0L7.05,18.36z"/></svg>`
    static darkModeIcon                     = `<svg xmlns="http://www.w3.org/2000/svg" enable-background="new 0 0 24 24" height="24px" viewBox="0 0 24 24" width="24px" fill="#FE9700"><rect fill="none" height="24" width="24"/><path d="M9.37,5.51C9.19,6.15,9.1,6.82,9.1,7.5c0,4.08,3.32,7.4,7.4,7.4c0.68,0,1.35-0.09,1.99-0.27 C17.45,17.19,14.93,19,12,19c-3.86,0-7-3.14-7-7C5,9.07,6.81,6.55,9.37,5.51z" opacity=".3"/><path d="M9.37,5.51C9.19,6.15,9.1,6.82,9.1,7.5c0,4.08,3.32,7.4,7.4,7.4c0.68,0,1.35-0.09,1.99-0.27C17.45,17.19,14.93,19,12,19 c-3.86,0-7-3.14-7-7C5,9.07,6.81,6.55,9.37,5.51z M12,3c-4.97,0-9,4.03-9,9s4.03,9,9,9s9-4.03,9-9c0-0.46-0.04-0.92-0.1-1.36 c-0.98,1.37-2.58,2.26-4.4,2.26c-2.98,0-5.4-2.42-5.4-5.4c0-1.81,0.89-3.42,2.26-4.4C12.92,3.04,12.46,3,12,3L12,3z"/></svg>`
    static title                            = "Toggle Light/Dark Mode"

    toggleMode() {
        ToggleDarkMode.userPreference = !ToggleDarkMode.userPreference
        this.updateIcon()
    }

    constructor() {
        super()

        let _this = this
        $(function() {
            $(document).ready(function() {
                
                _this.title = ToggleDarkModeButton.title
                _this.addEventListener('click', _this.toggleMode);
                _this.updateIcon()

                window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', event => {
                    _this.updateIcon()
                })
                window.matchMedia('(prefers-color-scheme: light)').addEventListener('change', event => {
                    _this.updateIcon()
                })
                document.addEventListener("visibilitychange", visibilityState => {
                    if (document.visibilityState === 'visible') {
                        _this.updateIcon()
                    }
                });
            })
        })
    }

    updateIcon() {

        this.innerHTML = ToggleDarkMode.darkModeEnabled
            ? ToggleDarkModeButton.darkModeIcon
            : ToggleDarkModeButton.lightModeIcon
        ;
    }
}
customElements.define(ToggleDarkModeButton.HTMLElement_name, ToggleDarkModeButton, {extends: 'button'});
awesome_doc_code_sections.HTML_elements.ToggleDarkModeButton  = ToggleDarkModeButton
    
// light/dark theme switch
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
