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

// awesome-doc-code-sections
//
//  Brief: Doxygen + doxygen-awesome-css + highlightjs == <3 (awesome-doc-code-sections)
//         Note that doxygen-awesome-css is not a mandatory dependency
//
// Code sections, with extra features :
//  - load content from 
//      - remote url (js: RemoteCodeSection)
//          such as (in index.md: <div class='awesome-doc-code-sections_remote-code-section' url='https://some/remote/path/to/file.cpp'></div> )
//      - local inner HTML (js: CodeSection)
//          awesome-doc-code-sections_code-section
//  - synthax-coloration provided by highlightjs,
//      - theme selector
//  - toggle dark/light theme
//  - buttons :
//      - send-to-godbolt
//      - copy-to-clipboard
//      - toggle light/dark mode

// TODO: as an external standalone project ?
// TODO: test behavior without theme selector (provide default behavior)
// TODO: not mandatory dependency to doxygen             (WIP)
// TODO: ToggleDarkMode: make AwesomeDoxygenCSS and awesome-doc-code-sections buttons update each others
// TODO: highlightjs makes clickable code elements not clickable anymore. Fix that ?
// TODO: Test with Marp

if (typeof hljs === 'undefined')
    console.error('awesome-doc-code-sections.js: depends on highlightjs, which is missing')

class ParsedCodeContent {
    constructor(code_content) {
        var lines = code_content.split('\n')

        // search for "// @awesome-doc-code-sections:"

        // @awesome-doc-code-sections:language=cpp
        // @awesome-doc-code-sections:include 
    }
}

// ============
// HTMLElements

// TODO: toggle dark/light mode if doxygen-awesome-css is not detected

class CopyToClipboardButton extends HTMLButtonElement {
// Copy text context of this previousSibling HTMLelement

    static HTMLElement_name = 'awesome-doc-code-sections_el_copy-to-clipboard-button'
    static title            = "Copy to clipboard"
    static copyIcon         = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="32" height="32"><path d="M0 0h24v24H0V0z" fill="none"/><path d="M16 1H4c-1.1 0-2 .9-2 2v14h2V3h12V1zm3 4H8c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h11c1.1 0 2-.9 2-2V7c0-1.1-.9-2-2-2zm0 16H8V7h11v14z"/></svg>`
    static successIcon      = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="32" height="32"><path d="M0 0h24v24H0V0z" fill="none"/><path d="M9 16.17L4.83 12l-1.42 1.41L9 19 21 7l-1.41-1.41L9 16.17z"/></svg>`
    static successDuration  = 980

    constructor() {
        super();

        this.title = CopyToClipboardButton.title
        this.innerHTML = CopyToClipboardButton.copyIcon

        this.style.zIndex = 2;
        this.style.position = 'absolute';
        this.style.top = 5 + 'px';
        this.style.right = 55 + 'px';

        this.onclick = function(){ 

            this.innerHTML = CopyToClipboardButton.successIcon
            this.style.fill = 'green'

            let text = this.previousSibling.textContent
            navigator.clipboard.writeText(text).then(
                function() {
                    console.log('awesome-doc-code-sections.js:CopyToClipboardButton: success');
                },
                function(err) {
                    console.error('awesome-doc-code-sections.js:CopyToClipboardButton: failed: ', err);
                }
            );
            window.setTimeout(() => {
                this.style.fill = 'black'
                this.innerHTML = CopyToClipboardButton.copyIcon
            }, CopyToClipboardButton.successDuration);
        }
    }
}
customElements.define(CopyToClipboardButton.HTMLElement_name, CopyToClipboardButton, {extends: 'button'});

function send_to_godbolt(element) {
    console.log('awesome-doc-code-sections.js:send_to_godbolt : ' + element)
    // TODO: detect language
}

class SendToGodboltButton extends HTMLButtonElement {
// TODO: open a godbolt tab with code content, or replace code section by a godbolt iframe
// TODO: language -> parent.classList or language attribute

    static HTMLElement_name = 'awesome-doc-code-sections_el_send-to-godbolt-button'
    static title            = 'Try this on godbolt.org (compiler-explorer)'
    static icon             = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="32" height="32"><switch><g><path d="M58.6 46.5c-.3-.5-.3-1.2 0-1.7.3-.6.7-1.3 1-2 .2-.5-.1-1-.7-1h-5.8c-.6 0-1.2.3-1.4.8-.7 1.1-1.6 2.2-2.6 3.2-3.7 3.7-8.6 5.7-13.9 5.7-5.3 0-10.2-2-13.9-5.7-3.8-3.7-5.8-8.6-5.8-13.9s2-10.2 5.8-13.9c3.7-3.7 8.6-5.7 13.9-5.7 5.3 0 10.2 2 13.9 5.7 1 1 1.9 2.1 2.6 3.2.3.5.9.8 1.4.8h5.8c.5 0 .9-.5.7-1-.3-.7-.6-1.3-1-2-.3-.5-.3-1.2 0-1.7l1.9-3.5c.4-.7.3-1.5-.3-2.1l-4.9-4.9c-.6-.6-1.4-.7-2.1-.3l-3.6 2c-.5.3-1.2.3-1.7 0-1.7-.9-3.5-1.7-5.4-2.2-.6-.2-1-.6-1.2-1.2l-1.1-3.9C40.1.5 39.5 0 38.7 0h-6.9C31 0 30.2.5 30 1.3l-1.1 3.9c-.2.6-.6 1-1.2 1.2-1.9.6-3.6 1.3-5.3 2.2-.5.3-1.2.3-1.7 0l-3.6-2c-.7-.4-1.5-.3-2.1.3l-4.9 4.9c-.6.6-.7 1.4-.3 2.1l2 3.6c.3.5.3 1.2 0 1.7-.9 1.7-1.7 3.5-2.2 5.3-.2.6-.6 1-1.2 1.2l-3.9 1.1c-.7.2-1.3.9-1.3 1.7v6.9c0 .8.5 1.5 1.3 1.7l3.9 1.1c.6.2 1 .6 1.2 1.2.5 1.9 1.3 3.6 2.2 5.3.3.6.3 1.2 0 1.7l-2 3.6c-.4.7-.3 1.5.3 2.1L15 57c.6.6 1.4.7 2.1.3l3.6-2c.6-.3 1.2-.3 1.7 0 1.7.9 3.5 1.7 5.3 2.2.6.2 1 .6 1.2 1.2l1.1 3.9c.2.7.9 1.3 1.7 1.3h6.9c.8 0 1.5-.5 1.7-1.3l1.1-3.9c.2-.6.6-1 1.2-1.2 1.9-.6 3.6-1.3 5.4-2.2.5-.3 1.2-.3 1.7 0l3.6 2c.7.4 1.5.3 2.1-.3l4.9-4.9c.6-.6.7-1.4.3-2.1l-2-3.5z" fill="#67c52a"/><path d="M23.5 37.7v4.4h23.8v-4.4H23.5zm0-7.8v4.4h19.6v-4.4H23.5zm0-7.9v4.4h23.8V22H23.5z" fill="#3c3c3f"/></g></switch></svg>`;
    static successIcon      = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="32" height="32"><path d="M0 0h24v24H0V0z" fill="none"/><path d="M9 16.17L4.83 12l-1.42 1.41L9 19 21 7l-1.41-1.41L9 16.17z"/></svg>`

    constructor() {

        super();

        this.title = SendToGodboltButton.title;
        this.innerHTML = SendToGodboltButton.icon;
        this.style.zIndex = 2;
        this.style.position = 'absolute';
        this.style.top = 5 + 'px';
        this.style.right = 5 + 'px';
        this.style.borderRadius = 3

        this.addEventListener(
            'click',
            () => {
                console.log("awesome-doc-code-sections.js:SendToGodboltButton : clicked !")
                this.innerHTML = SendToGodboltButton.successIcon
                this.style.fill = 'green'

                // TODO

                window.setTimeout(() => {
                    this.innerHTML = SendToGodboltButton.icon
                    this.style.fill = 'black'
                }, 1000);
            }
        ); // TODO: godbolt CE API (and inject csl::ag header include<raw_github_path.hpp>)
    }
}
customElements.define(SendToGodboltButton.HTMLElement_name, SendToGodboltButton, {extends: 'button'});

class CodeSection extends HTMLElement {
// Code section, with synthax-coloration provided by highlightjs
// Additionaly, the language code language can be forced (`code_language` parameter, or `language` attributes),
// otherwise it is automatically detected based on fetched code content
//
// <CodeSection language='cpp'>[some code here]</CodeSection>

    static HTMLElement_name = 'awesome-doc-code-sections_code-section'

    constructor(code, language) {
        super();

        // arguments
        if (code === undefined && this.textContent !== undefined)
            code = this.textContent
        if (! language)
            language = this.getAttribute('language') || undefined
        if (language !== undefined && !language.startsWith("language-"))
            language = `language-${language}`

        this.code = code;
        this.language = language;

        // TODO: parse for specific format (or use regex + specific tags ?)
        // - expected output tag
        // - {code}{separator}{expected_output}
        
        if (code !== undefined)
            this.load();
        else
            this.innerHTML = '<p>awesome-doc-code-sections:CodeSection : missing code</p>'
    }

    connectedCallback() {
    }

    load() {

        // code content
        let code_node = document.createElement('pre');
            code_node.style.zIndex = 1;
            code_node.style.position = 'relative'
        let code = code_node.appendChild(document.createElement('code'));
            code.textContent = this.code

        code.classList.add('hljs')
        if (this.language !== undefined && this.language !== null)
            code.classList.add(`${this.language}`);
        hljs.highlightElement(code)

        // buttons
        let copy_button = new CopyToClipboardButton()
            copy_button.style.zIndex = code_node.style.zIndex + 1
        code_node.appendChild(copy_button)

        let CE_button = new SendToGodboltButton
            CE_button.style.zIndex = code_node.style.zIndex + 1
        code_node.appendChild(CE_button)

        this.innerHTML = code_node.outerHTML;
    }

    static Initialize_DivHTMLElements() {
    // expected format :
    // <div class='awesome-doc-code-sections_code-section' [language='cpp']>
    //  some code here
    // </div>

        var place_holders = $('body').find(`div[class=${CodeSection.HTMLElement_name}]`);
        console.log(`awesome-doc-code-sections.js:CodeSection : Initialize_DivHTMLElements : replacing ${place_holders.length} elements ...`)
        place_holders.each((index, value) => {
    
            let language = value.getAttribute('language')
            let code = value.textContent
    
            let node = new CodeSection(code, language);
                node.setAttribute('language', language)
            value.replaceWith(node);
        });
    }
}
customElements.define(CodeSection.HTMLElement_name, CodeSection);

// TODO : Optionaly wrap in table/tr/th/td ?

class RemoteCodeSection extends CodeSection {
// Fetch some code as texts based on the `code_url` parameter (or `url` attribute),
// and creates a code-sections (pre/code) with synthax-color provided by hightlighthjs
// Additionaly, the language code language can be forced (`code_language` parameter, or `language` attributes),
// otherwise it is automatically detected based on fetched code content

    static HTMLElement_name = 'awesome-doc-code-sections_remote-code-section'

    constructor(code_url, language) {
        super();

        if (code_url === undefined && this.getAttribute('url') != undefined)
            code_url = this.getAttribute('url')
        if (language === undefined && this.getAttribute('language') != undefined)
            language = this.getAttribute('language')

        // TODO: parse for specific format (or use regex + specific tags ?)
        // - expected output tag
        // - {code}{separator}{expected_output}
       
        if (code_url === undefined) {
            this.innerHTML = '<p>awesome-doc-code-sections:RemoteCodeSection : missing code_url</p>'
            return
        }
        if (language === undefined) { // perhaps nothing is better than that ... ?
            console.log(`awesome-doc-code-sections.js:RemoteCodeSection : attempting fallback language : ${language}`)
            language = RemoteCodeSection.get_url_extension(code_url)
        }

        this.code_url = code_url;
        super.language = language;

        this.#load();
    }

    static get_url_extension( url ) {
        try {
            return url.split(/[#?]/)[0].split('.').pop().trim();
        }
        catch (error) {
            return undefined
        }
    }

    #load() {

        let apply_code = (code) => {
            super.code = code;
            super.load();
        }

        let xhr = new XMLHttpRequest();
        xhr.open('GET', this.code_url); // TODO: async
        xhr.onerror = function() {
            console.error(`awesome-doc-code-sections.js:RemoteCodeSection : Network Error`);
        };
        xhr.onload = function() {

            if (xhr.status != 200) {
                return;
            }
            apply_code(xhr.responseText)
        };
        xhr.send();
    }

    static Initialize_DivHTMLElements() {
    // expected format :
    // <div class='awesome-doc-code-sections_remote-code-section' url='some/url/to/code[.language]' [language='cpp']>
    // </div>
        var place_holders = $('body').find(`div[class=${RemoteCodeSection.HTMLElement_name}]`);
        console.log(`awesome-doc-code-sections.js:RemoteCodeSection : Initialize_DivHTMLElements : replacing ${place_holders.length} elements ...`)
        place_holders.each((index, value) => {

            if (value.getAttribute('url') === undefined) {
                console.error('awesome-doc-code-sections.js:inject_examples : div/code_example is missing an url attribute')
                return true; // ill-formed, skip this element but continue iteration
            }
            let url = value.getAttribute('url')
            let language = value.getAttribute('language')
            // let language = (value.classList.length != 1 ? value.classList[1] : undefined);

            let node = new RemoteCodeSection(url, language);
                node.setAttribute('url', url);
                node.setAttribute('language', language)
            value.replaceWith(node);
        });
    }
}
customElements.define(RemoteCodeSection.HTMLElement_name, RemoteCodeSection);

class ThemeSelector {
// For themes, see https://cdnjs.com/libraries/highlight.js
// The default one is the first one
//
// Use theme name, without light or dark specification
// examples
//
// <select class="code_theme_selector">
//     <option class="code_theme_option" value="tokyo-night"></option>
//     <option class="code_theme_option" value="base16/google"></option>
// </select>
//
// Current limitation: not a dedicated HTMLElement
//
// Note that an HTML placeholder for stylesheet is necessary/mandatory
//   <link id='code_theme_stylesheet' rel="stylesheet" crossorigin="anonymous" referrerpolicy="no-referrer" />

    static HTMLElement_name = 'awesome-doc-code-sections_theme-selector'
    static url_base = 'https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.6.0/styles/'
    static url_ext = '.min.css'
    static dark_or_light_placeholder = '{dark_or_light}'
    static stylesheet_HTML_placeholder_id = 'code_theme_stylesheet'

    static BuildUrl(arg) {
        if (typeof arg !== 'string' && ! arg instanceof String) {
            console.error('ThemeSelector.BuildUrl: invalid argument')
            return
        }
        return ThemeSelector.url_base
            + arg + '-'
            + ThemeSelector.dark_or_light_placeholder
            + ThemeSelector.url_ext
        ;
    }

    static check_stylesheet_HTML_placeholder() {

        var style_placeholder = document.getElementById(ThemeSelector.stylesheet_HTML_placeholder_id)
        if (style_placeholder === undefined || style_placeholder === null) 
            console.error(
                `awesome-doc-code-sections.js:ThemeSelector : missing stylesheet HTML placeholder\n
                <link id="${ThemeSelector.stylesheet_HTML_placeholder_id}" rel="stylesheet"/>`
            )
    }
    static Initialize_SelectHTMLElements = function() {

        let onOptionSelectedChange = function() {
            let selected_option = $(this).find('option:selected')
            console.log('awesome-doc-code-sections.js:ThemeSelector : switching to ' + selected_option.text())

            let html_node = document.getElementsByTagName('html')[0];

            if (html_node.className !== "dark-mode" &&
                html_node.className !== "light-mode")
                return;

            let theme_color = html_node.className.replace('-mode', '')
            let new_stylesheet_url = ThemeSelector.BuildUrl(selected_option.text())
                .replace(ThemeSelector.dark_or_light_placeholder, theme_color)
            console.log('awesome-doc-code-sections.js:ThemeSelector : switching to stylesheet : ' + new_stylesheet_url)
            document.getElementById('code_theme_stylesheet').href = new_stylesheet_url

            hljs.highlightAll()
        }

        $(document).ready(function() {
            console.log('awesome-doc-code-sections.js:ThemeSelector : initializing themes selector ...')

            ThemeSelector.check_stylesheet_HTML_placeholder()

            var options = $('body').find('select[class=code_theme_selector] option[class=code_theme_option]');
            options.each((index, element) => {

                let value = element.getAttribute('value')
                if (element === undefined || element == null) {
                    console.error('ThemeSelector: invalid argument')
                    return
                }

                element.textContent = value
            })

            var selectors = $('body').find('select[class=code_theme_selector]');
            selectors.each((index, element) => {
                element.onchange = onOptionSelectedChange
                element.onchange() // initialization
            })
        })
    }
}
// Theme style switch
const highlightjs_stylesheet_href_mutationObserver = new MutationObserver((mutationsList, observer) => {

    mutationsList.forEach(mutation => {
        if (mutation.attributeName !== 'href')
            return;

        let code_stylesheet = document.getElementById('code_theme_stylesheet');
        if (mutation.oldValue === code_stylesheet.href ||
            code_stylesheet.href === window.location.href)
            return
        console.log('awesome-doc-code-sections_dark-mode.js:onHighlightjsHrefChange: Switching highlighths stylesheet \n from : ' + mutation.oldValue + '\n to   : ' + code_stylesheet.href)

        hljs.highlightAll();
    })
});
highlightjs_stylesheet_href_mutationObserver.observe(
    document.getElementById(ThemeSelector.stylesheet_HTML_placeholder_id),
    { 
        attributes: true,
        attributeFilter: [ 'href' ],
        attributeOldValue: true
    }
)

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

// ============

var awesome_doc_code_sections = {}
    awesome_doc_code_sections.HTML_elements = {}
    awesome_doc_code_sections.HTML_elements.CopyToClipboardButton = CopyToClipboardButton
    awesome_doc_code_sections.HTML_elements.SendToGodboltButton   = SendToGodboltButton
    awesome_doc_code_sections.HTML_elements.ToggleDarkModeButton  = ToggleDarkModeButton
    awesome_doc_code_sections.HTML_elements.CodeSection           = CodeSection
    awesome_doc_code_sections.HTML_elements.RemoteCodeSection     = RemoteCodeSection
    awesome_doc_code_sections.ThemeSelector = ThemeSelector
    awesome_doc_code_sections.ToggleDarkMode = ToggleDarkMode

// TODO: make sure that doxygen elements are also still clickable with pure doxygen (not doxygen-awesome-css)
awesome_doc_code_sections.initialize_doxygenCodeSections = function() {
// Replace code-sections generated by doxygen (and possibly altered by doxygen-awesome-css)
// like `<pre><code></code></pre>`,
// or placeholders like `\include path/to/example.ext`

    // DoxygenAwesomeFragmentCopyButton wraps code in 
    //  div[class="doxygen-awesome-fragment-wrapper"] div[class="fragment"] div[class="line"]
    // otherwise, default is 
    //  div[class="fragment"] div[class="line"]

    // clickable documentation elements are :
    //  div[class="doxygen-awesome-fragment-wrapper"] div[class="fragment"] div[class="line"]
    //      <a class="code" href="structcsl_1_1ag_1_1size.html">csl::ag::size&lt;A&gt;::value</a>

    let doc_ref_links = new Map(); // preserve clickable documentation reference links

    var place_holders = $('body').find('div[class=doxygen-awesome-fragment-wrapper]');
    console.log(`awesome-doc-code-sections.js:initialize_doxygenCodeSections : replacing ${place_holders.length} elements ...`)
    place_holders.each((index, value) => {

        let lines = $(value).find('div[class=fragment] div[class=line]')

        // WIP
        let links = lines.find('a[class="code"]')
        links.each((index, value) => {
            doc_ref_links.set(value.textContent, value.href)
        })
        // /WIP

        let code = $.map(lines, function(value) { return value.textContent }).join('\n')
        let node = new CodeSection(code, undefined);
            $(value).replaceWith(node)
    })

    var place_holders = $('body').find('div[class=fragment]')
    console.log(`awesome-doc-code-sections.js:initialize_doxygenCodeSections : replacing ${place_holders.length} elements ...`)
    place_holders.each((index, value) => {

        let lines = $(value).find('div[class=line]')

        // WIP
        let links = lines.find('a[class="code"]')
        links.each((index, value) => {
            doc_ref_links.set(value.textContent, value.href)
        })
        // /WIP

        let code = $.map(lines, function(value) { return value.textContent }).join('\n')
        let node = new CodeSection(code, undefined);
            $(value).replaceWith(node)
    })

    // documentation reference links
    doc_ref_links.forEach((values, keys) => {
        // console.log(">>>>>>> " + value.href + " => " + value.textContent)
        console.log(">>>>>>> " + values + " => " + keys)
    })

    var place_holders = $('body').find('awesome-doc-code-sections_code-section pre code') // span or text
    place_holders.filter(function() {
        return $(this).text().replace(/toto/g, '<a href=".">toto</a>');
      })
}
awesome_doc_code_sections.initialize_PreCodeHTMLElements = function() {

    $('body').find('pre code').each((index, value) => { // filter
        if ($(value).parent().parent().prop('nodeName').toLowerCase().startsWith("awesome-doc-code-sections_"))
            return
        
            let existing_node = $(value).parent()

            let language = value.getAttribute('language')
            let code = existing_node.text()

            let node = new CodeSection(code, language);
                node.setAttribute('language', language)
            existing_node.replaceWith(node);
    })

    // TODO: same for only code elements ?
}

awesome_doc_code_sections.options = new class{

    doxygen_awesome_css_compatibility   = false
    pre_code_compatibility              = false
    auto_hide_buttons                   = false
    toggle_dark_mode                    = (typeof DoxygenAwesomeDarkModeToggle !== 'undefined')

    configure = function(obj) {
        if (obj === undefined || obj === null)
            return
        this.doxygen_awesome_css_compatibility  = obj.doxygen_awesome_css_compatibility || this.doxygen_awesome_css_compatibility
        this.pre_code_compatibility             = obj.pre_code_compatibility            || this.pre_code_compatibility
        this.auto_hide_buttons                  = obj.auto_hide_buttons                 || this.auto_hide_buttons
        this.toggle_dark_mode                   = obj.toggle_dark_mode                  || this.toggle_dark_mode
    }
}()

awesome_doc_code_sections.initialize_ButtonsAutoHide = function() {
// TODO: How to do that dynamically (resize event, observer, etc.)?

    let auto_hide_element = (container, element) => {
        element.hide()
        container.mouseover(() => {
            element.show()
        });
        container.mouseout(() => {
            element.hide()
        });
    }

    $('body').find('button[is^=awesome-doc-code-sections_el_]').each((index, value) => { 

        let node_containing_button = $(value).parent().parent()

        if (!node_containing_button.prop('nodeName').toLowerCase().startsWith("awesome-doc-code-sections_"))
            return // unlikely

        if (awesome_doc_code_sections.options.auto_hide_buttons
        ||  node_containing_button.width() < 400 
        // ||  node_containing_button.width() < (window.screen.availWidth / 2)
        )   auto_hide_element(node_containing_button, $(value))
    })
}

awesome_doc_code_sections.initialize = function() {

    $(function() {
        $(document).ready(function() {

            console.log('awesome-doc-code-sections.js:initialize: initializing code sections ...')

            awesome_doc_code_sections.ToggleDarkMode.initialize()

            // replace <select/> with proper HTML elements
            awesome_doc_code_sections.ThemeSelector.Initialize_SelectHTMLElements();
            // replace <div/> with proper HTML elements
            awesome_doc_code_sections.HTML_elements.CodeSection.Initialize_DivHTMLElements();
            awesome_doc_code_sections.HTML_elements.RemoteCodeSection.Initialize_DivHTMLElements();

            if (awesome_doc_code_sections.options.doxygen_awesome_css_compatibility === true) {
                console.log(`awesome-doc-code-sections.js:initialize: doxygen-awesome-css compatiblity ...`)
                awesome_doc_code_sections.initialize_doxygenCodeSections()
            }

            if (awesome_doc_code_sections.options.pre_code_compatibility) {
                console.log(`awesome-doc-code-sections.js:initialize: existing pre-code compatiblity ...`)
                awesome_doc_code_sections.initialize_PreCodeHTMLElements();
            }

            awesome_doc_code_sections.initialize_ButtonsAutoHide()
        })
    })
}

// TODO: module
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Statements/export
// export { awesome_doc_code_sections }
// import adcs from '/path/to/awesome-doc-code-sections.js'