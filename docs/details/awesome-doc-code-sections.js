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
//  Brief: Doxygen + doxygen-awesome-css + highlightjs == <3
//
// Code sections, with extra features :
//  - load content from 
//      - remote url
//          such as (in index.md: <div class='code_example' url='https://some/remote/path/to/file.cpp'></div> )
//  - synthax-coloration provided by highlightjs,
//      - theme selector
//  - buttons :
//      - send-to-godbolt
//      - copy-to-clipboard

// TODO: as an external standalone project ?
// TODO: not mandatory dependency to doxygen
// TODO: not mandatory dependency to doxygen-awesome-css
// TODO: class that extends HTMLElement == div with specific class (as placeholders) + replace/init function (for Markdown for instance)

if (hljs === undefined)
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

class CopyToClipboardButton extends HTMLButtonElement {
// Copy text context of this previousSibling HTMLelement

    static title        = "Copy to clipboard"
    static copyIcon     = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="32" height="32"><path d="M0 0h24v24H0V0z" fill="none"/><path d="M16 1H4c-1.1 0-2 .9-2 2v14h2V3h12V1zm3 4H8c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h11c1.1 0 2-.9 2-2V7c0-1.1-.9-2-2-2zm0 16H8V7h11v14z"/></svg>`
    static successIcon  = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="32" height="32"><path d="M0 0h24v24H0V0z" fill="none"/><path d="M9 16.17L4.83 12l-1.42 1.41L9 19 21 7l-1.41-1.41L9 16.17z"/></svg>`
    static successDuration = 980

    constructor() {
        super();

        this.title = CopyToClipboardButton.title
        this.innerHTML = CopyToClipboardButton.copyIcon

        this.style.zIndex = 2;
        this.style.position = 'absolute';
        this.style.top = 10 + 'px';
        this.style.right = 60 + 'px';

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
customElements.define('awesome-doc-code-sections_copy-to-clipboard-button', CopyToClipboardButton, {extends: 'button'});

function send_to_godbolt(element) {
    console.log('awesome-doc-code-sections.js:send_to_godbolt : ' + element)
    // TODO: detect language
}

class SendToGodboltButton extends HTMLButtonElement {
// TODO: open a godbolt tab with code content, or replace code section by a godbolt iframe

    static title = 'Try this on godbolt.org (compiler-explorer)'
    static icon = 
        `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="32" height="32">
        <switch><g><path d="M58.6 46.5c-.3-.5-.3-1.2 0-1.7.3-.6.7-1.3 1-2 .2-.5-.1-1-.7-1h-5.8c-.6 0-1.2.3-1.4.8-.7 1.1-1.6 2.2-2.6 3.2-3.7 3.7-8.6 5.7-13.9 5.7-5.3 0-10.2-2-13.9-5.7-3.8-3.7-5.8-8.6-5.8-13.9s2-10.2 5.8-13.9c3.7-3.7 8.6-5.7 13.9-5.7 5.3 0 10.2 2 13.9 5.7 1 1 1.9 2.1 2.6 3.2.3.5.9.8 1.4.8h5.8c.5 0 .9-.5.7-1-.3-.7-.6-1.3-1-2-.3-.5-.3-1.2 0-1.7l1.9-3.5c.4-.7.3-1.5-.3-2.1l-4.9-4.9c-.6-.6-1.4-.7-2.1-.3l-3.6 2c-.5.3-1.2.3-1.7 0-1.7-.9-3.5-1.7-5.4-2.2-.6-.2-1-.6-1.2-1.2l-1.1-3.9C40.1.5 39.5 0 38.7 0h-6.9C31 0 30.2.5 30 1.3l-1.1 3.9c-.2.6-.6 1-1.2 1.2-1.9.6-3.6 1.3-5.3 2.2-.5.3-1.2.3-1.7 0l-3.6-2c-.7-.4-1.5-.3-2.1.3l-4.9 4.9c-.6.6-.7 1.4-.3 2.1l2 3.6c.3.5.3 1.2 0 1.7-.9 1.7-1.7 3.5-2.2 5.3-.2.6-.6 1-1.2 1.2l-3.9 1.1c-.7.2-1.3.9-1.3 1.7v6.9c0 .8.5 1.5 1.3 1.7l3.9 1.1c.6.2 1 .6 1.2 1.2.5 1.9 1.3 3.6 2.2 5.3.3.6.3 1.2 0 1.7l-2 3.6c-.4.7-.3 1.5.3 2.1L15 57c.6.6 1.4.7 2.1.3l3.6-2c.6-.3 1.2-.3 1.7 0 1.7.9 3.5 1.7 5.3 2.2.6.2 1 .6 1.2 1.2l1.1 3.9c.2.7.9 1.3 1.7 1.3h6.9c.8 0 1.5-.5 1.7-1.3l1.1-3.9c.2-.6.6-1 1.2-1.2 1.9-.6 3.6-1.3 5.4-2.2.5-.3 1.2-.3 1.7 0l3.6 2c.7.4 1.5.3 2.1-.3l4.9-4.9c.6-.6.7-1.4.3-2.1l-2-3.5z" fill="#67c52a"/><path d="M23.5 37.7v4.4h23.8v-4.4H23.5zm0-7.8v4.4h19.6v-4.4H23.5zm0-7.9v4.4h23.8V22H23.5z" fill="#3c3c3f"/>
        </g></switch></svg>`;
    static successIcon  = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="32" height="32"><path d="M0 0h24v24H0V0z" fill="none"/><path d="M9 16.17L4.83 12l-1.42 1.41L9 19 21 7l-1.41-1.41L9 16.17z"/></svg>`

    constructor() {

        super();
        this.title = SendToGodboltButton.title;
        this.innerHTML = SendToGodboltButton.icon;
        this.style.zIndex = 2;
        this.style.position = 'absolute';
        this.style.top = 10 + 'px';
        this.style.right = 10 + 'px';
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
customElements.define('awesome-doc-code-sections_send-to-godbolt-button', SendToGodboltButton, {extends: 'button'});

class CodeSection extends HTMLElement {
// Code section, with synthax-coloration provided by highlightjs
// Additionaly, the language code language can be forced (`code_language` parameter, or `language` attributes),
// otherwise it is automatically detected based on fetched code content

    constructor(code, language) {
        super();

        // arguments
        if (code === undefined && this.textContent != undefined)
            code = this.textContent
        if (language === undefined && this.getAttribute('language') != undefined)
            language = this.getAttribute('language')
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

    load() {

        // code content
        let code_node = document.createElement('pre');
            code_node.style.zIndex = 1;
            code_node.style.position = 'relative'
        let code = code_node.appendChild(document.createElement('code'));
            code.textContent = this.code

        if (this.language != undefined)
            code.className = `hljs ${this.language}`;
        else
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
}
window.customElements.define('awesome-doc-code-sections_code-section', CodeSection);

// TODO : Optionaly wrap in table/tr/th/td ?
// TODO: factory/init: replace all <div class='awesome-doc-code-sections_code-section' [language='cpp'] />

class RemoteCodeSection extends CodeSection {
// Fetch some code as texts based on the `code_url` parameter (or `url` attribute),
// and creates a code-sections (pre/code) with synthax-color provided by hightlighthjs
// Additionaly, the language code language can be forced (`code_language` parameter, or `language` attributes),
// otherwise it is automatically detected based on fetched code content

    connectedCallback() {
        // console.log('awesome-doc-code-sections.js:RemoteCodeSection : connectedCallback with url attribute : ' + this.getAttribute('url'));
    }

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
}
window.customElements.define('awesome-doc-code-sections_remote-code-section', RemoteCodeSection);

// TODO: factory/init: replace all <div class='awesome-doc-code-sections_remote-code-section' url='...' [language='cpp'] />

// ============

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

    static url_base = 'https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.6.0/styles/'
    static url_ext = '.min.css'
    static dark_or_light_placeholder = '{dark_or_light}'

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

    static initialize = function() {

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

            var options = $('body').find('option[class=code_theme_option]');
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

var awesome_doc_code_sections = {}
awesome_doc_code_sections.HTML_elements = {}
awesome_doc_code_sections.HTML_elements.CopyToClipboardButton = CopyToClipboardButton
awesome_doc_code_sections.HTML_elements.SendToGodboltButton   = SendToGodboltButton
awesome_doc_code_sections.HTML_elements.RemoteCodeSection     = RemoteCodeSection
awesome_doc_code_sections.ThemeSelector = ThemeSelector // private?

awesome_doc_code_sections.replace_doxygen_awesome_frament_wrapper = function() {
// Replace code-sections generated by doxygen-awesome-css

    $(document).ready(function() {

        var place_holders = $('body').find('div[class=doxygen-awesome-fragment-wrapper]');
        console.log(`awesome-doc-code-sections.js:replace_doxygen_awesome_frament_wrapper : replacing ${place_holders.length} elements ...`)
        place_holders.each((index, value) => {
            value.className = 'awesome-doc-code-sections_code-section'
            
            // merge fragments
            const fragments = document.getElementsByClassName("fragment")
            for(const fragment of fragments) {
            
                let code_node = document.createElement('code');
                    code_node.innerHTML = fragment.textContent;
                    // TODO: parse code_content for specific tags ? (ex: expected output, header, etc.)
                let new_node  = document.createElement('pre');
                    new_node.appendChild(code_node)
                fragment.replaceWith(new_node)
            }

            // TODO: remove doxygen-awesome-fragment-copy-button js file include ?
        });
    })
}
awesome_doc_code_sections.inject_examples = () => { // private
    // expected format : <div class='code_example' url='path/to/code'></div>
    var place_holders = $('body').find('div[class^=code_example]');
    place_holders.each((index, value) => {

        if (value.getAttribute('url') === undefined) {
            console.error('awesome-doc-code-sections.js:inject_examples : div/code_example is missing an url attribute')
            return true; // ill-formed, skip this element but continue iteration
        }
        let url = value.getAttribute('url')
        console.log('awesome-doc-code-sections.js:inject_examples : processing example: ' + index + ' with index ' + url + ' ...')

        let language = (value.classList.length != 1 ? value.classList[1] : undefined);

        let example_element = new RemoteCodeSection(url, language);
            example_element.setAttribute('url', url);
            example_element.setAttribute('language', language)
        value.appendChild(example_element);
    });
}
awesome_doc_code_sections.initialize = function() {
    $(document).ready(function() {
        console.log('awesome-doc-code-sections.js:initialize: initializing code sections ...')
        awesome_doc_code_sections.replace_doxygen_awesome_frament_wrapper();
        awesome_doc_code_sections.inject_examples();
        awesome_doc_code_sections.ThemeSelector.initialize();
    })
}

// TODO: module
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Statements/export
// export { awesome_doc_code_sections }
// import adcs from '/path/to/awesome-doc-code-sections.js'