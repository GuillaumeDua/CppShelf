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
// TODO: compatibility with https://github.com/EnlighterJS/EnlighterJS instead of highlightjs
// TODO: compatibility with Marp
//
// TODO: test behavior without theme selector   (provide default behavior)
// TODO: not mandatory dependency to doxygen    (WIP)
// TODO: highlightjs makes clickable code elements not clickable anymore. Fix that ?
//          https://stackoverflow.com/questions/74114767/highlightjs-how-to-create-custom-clickable-sequence-of-characters
// TODO: hide warnings for undefined/fallback hljs language
// TODO: soft errors (replace HTMLElement content with red error message, rather than stopping the process)
// TODO: make Initialize_DivHTMLElements generic

if (typeof hljs === 'undefined')
    console.error('awesome-doc-code-sections.js: depends on highlightjs, which is missing')

var awesome_doc_code_sections = {}
    awesome_doc_code_sections.configuration = {}
    awesome_doc_code_sections.configuration.GodboltLanguages = new Map()
    // key   : language_hljs_name
    // value : {
    //      language,       // not mandatory, if same as key. Refers to https://godbolt.org/api/languages
    //      compiler_id,
    //      default_options // not mandatory
    // }

class ParsedCode {
// @awesome-doc-code-sections::split     // create a table/tr/td ? tr0: code, tr1: output
//  `code` as an array ? or multiples ParsedCode (ParsedCode.next() ?)
// @awesome-doc-code-sections::keep : keep tag anyway as comment (for documentation purpose)

// @awesome-doc-code-sections::language=cpp                  // use for CE compiler-id and hljs
// @awesome-doc-code-sections::CE::compiler_id=clang1500
// @awesome-doc-code-sections::CE::compilation_options=-O2 -std=c++20
// @awesome-doc-code-sections::CE::libs=fmt
// @awesome-doc-code-sections::includes_transformation=local_prefix|example_prefix|remote_prefix

// @awesome-doc-code-sections::show::block::begin,end
// @awesome-doc-code-sections::CE::line::skip
// @awesome-doc-code-sections::CE::line::replace_with=

    static tag = '// @awesome-doc-code-sections'

    code    = ''
    ce_code = ''
    ce_options = {}

    constructor(code_content) {

        this.#parse(code_content)
        this.#apply_ce_transformations()
    }

    #parse(code_content) {
        // TODO: regexes: non-greedy any (.*?)

        // CE options
        let regexp = new RegExp(`${ParsedCode.tag}::CE=({(.*\n//.*)+}\n?)`, 'gm')
        let matches = [...code_content.matchAll(regexp)] // expect exactly 1 match
        if (matches.length > 1)
            console.error(`awesome-doc-code-sections.js:ParsedCode::constructor: multiples CE configurations`)
        matches.map((match) => {
            let result = match[1].replaceAll('\n//', '')
            // remove from original content
            code_content = code_content.slice(0, match.index)
                         + code_content.slice(match.index + match[0].length)
            return result
        }).forEach((value) => {
            this.ce_options = JSON.parse(value)
        })

        // TODO: skip block, line

        // Example: show block, line
        regexp = new RegExp(`(^${ParsedCode.tag}::show::block::begin\n(?<block>(^.*$\n)+)${ParsedCode.tag}::show::block::end\n?)|(^(?<line>.*)\s*${ParsedCode.tag}::show::line$)`, 'gm')
        matches = [...code_content.matchAll(regexp)]
        let code_only_show = matches
        // TODO: reverse() so we can use indexes to remove elements rather than replace()
            .map((match) => {
                let result = match.groups.block !== undefined
                    ? match.groups.block 
                    : match.groups.line
                // remove from original content
                code_content = code_content.replace(match[0], result)
                return result
            })
            .join('\n')

        this.code = (code_only_show !== "" ? code_only_show : code_content)
        this.ce_code = code_content
    }
    #apply_ce_transformations() {

        // includes_transformation
        if (this.ce_options.includes_transformation !== undefined) {
            this.ce_options.includes_transformation.forEach((value) => {
                // replace includes
                
                const regex = new RegExp(`^(\\s*\\#.*?[\\"|\\<"].*?)(${value[0]})(.*?[\\"|\\>"])`, 'gm')
                this.ce_code = this.ce_code.replace(regex, `$1${value[1]}$3`)
            })
        }
    }
}
awesome_doc_code_sections.ParsedCode = ParsedCode

class remote_resources_cache {
    #remote_files = new Map() // uri -> text

    static async #fetch_remote_file(uri) {

        try {
            let response = await fetch(uri)
            return await response.text()
        }
        catch (error) {
            console.error(
                "awesome-doc-code-sections.js:remote_resources_cache: error\n" +
                "\t" + error
            )
        }
    }

    async get(uri) {
        if (! this.#remote_files.has(uri)) {
            this.#remote_files.set(
                uri,
                await remote_resources_cache.#fetch_remote_file(uri)
            )
        }
        return this.#remote_files.get(uri)
    }
}

class ce_API {
// fetch CE API information asynchronously

    static #static_initializer = (async function(){
        ce_API.#fetch_languages()
    })()

    // cache
    static languages = undefined
    static #remote_files_cache = new remote_resources_cache()

    static async #fetch_languages() {
    // https://godbolt.org/api/languages
        try {
            let response = await fetch('https://godbolt.org/api/languages')
            let datas = await response.text()

            let text = datas.split('\n')
            text.shift() // remove header
            ce_API.languages = text.map((value) => {
            // keep only ids
                return value.slice(0, value.indexOf(' '))
            })
        }
        catch (error) {
            console.error(
                "awesome-doc-code-sections.js:ce_API: godbolt API exception (fetch_languages)\n" +
                "\t" + error
            )
        }
    }
    static open_in_new_tab(request_data) {
    // https://godbolt.org/clientstate/

        console.log(request_data)

        let body  = JSON.stringify(request_data);
        let state = btoa(body); // base64 encoding
        let url   = "https://godbolt.org/clientstate/" + encodeURIComponent(state);
    
        // Open in a new tab
        window.open(url, "_blank");
    }
    static async fetch_execution_result(ce_options, code) {
    // https://godbolt.org/api/compiler/${compiler_id}/compile

        // POST /api/compiler/<compiler-id>/compile endpoint is not working with remote header-files in `#include`s PP directions
        // https://github.com/compiler-explorer/compiler-explorer/issues/4190
        let matches = [...code.matchAll(/^\s*\#\s*include\s+[\"|\<](\w+\:\/\/.*?)[\"|\>]/gm)].reverse()

        let promises_map = matches.map(async function(match) {

            let downloaded_file_content = await ce_API.#remote_files_cache.get(match[1])
            console.log(`>>>>> fetched downloaded_file_content length == ${downloaded_file_content.length}`)
            let match_0_token = match[0].replaceAll('\n', '')
            code = code.replace(match[0], `// download[${match_0_token}]::begin\n${downloaded_file_content}\n// download[${match_0_token}]::end`)
        })

        // Build & send the request
        let fetch_result = async () => {

            let body = {
                "source": code,
                "compiler": ce_options.compiler_id,
                "options": {
                    "userArguments": ce_options.compilation_options,
                    "executeParameters": {
                        "args": ce_options.execute_parameters_args || [],
                        "stdin": ce_options.execute_parameters_stdin || ""
                    },
                    "compilerOptions": {
                        "executorRequest": true
                    },
                    "filters": {
                        "execute": true
                    },
                    "tools": [],
                    "libraries": ce_options.libs || []
                },
                "lang": ce_options.language,
                "allowStoreCodeDebug": true
            }
            const options = {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json;charset=utf-8'
                },
                body: JSON.stringify(body)
            };

            console.log(body)

            return await fetch(`https://godbolt.org/api/compiler/${ce_options.compiler_id}/compile`, options)
                .then(response => response.text())
        }

        return await Promise.all(promises_map).then(() => {
            return fetch_result()
        })
    }
}

// ============
// HTMLElements

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

class SendToGodboltButton extends HTMLButtonElement {

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

                this.onClickSend()

                window.setTimeout(() => {
                    this.innerHTML = SendToGodboltButton.icon
                    this.style.fill = 'black'
                }, 1000);
            }
        );
    }

    onClickSend() {
        let codeSectionElement = this.parentElement.parentElement

        console.log(codeSectionElement)

        if (codeSectionElement === undefined
        ||  codeSectionElement.tagName.match(`\w+${CodeSection.HTMLElement_name.toUpperCase()}`) === '')
            console.error("awesome-doc-code-sections.js:SendToGodboltButton::onClickSend: unexpected parent.parent element (must be an - optionaly Basic - CodeSection)")
        console.log('awesome-doc-code-sections.js:SendToGodboltButton::onClickSend: : sending ...')
    
        var get_configuration = function() {
            
            let configuration = awesome_doc_code_sections.configuration.GodboltLanguages.get(codeSectionElement.hljs_language)
            if (configuration === undefined)
                console.error(`awesome-doc-code-sections.js:SendToGodboltButton::onClickSend: missing configuration for hljs language [${codeSectionElement.hljs_language}]`)
            return configuration
        }
        var get_ce_options = function() {
            return typeof codeSectionElement.ce_options === 'undefined' || codeSectionElement.ce_options === undefined
                ? get_configuration()
                : codeSectionElement.ce_options
        }
        var get_language = function() {
        //      hljs    https://github.com/highlightjs/highlight.js/blob/main/SUPPORTED_LANGUAGES.md
        //  vs. godbolt https://godbolt.org/api/languages
            return ce_API.languages.includes(get_ce_options().language)
                ? get_ce_options().language
                : get_configuration().language
        }
        var get_code = function() {
            let result = codeSectionElement.ce_code || codeSectionElement.code
            if (result === undefined)
                console.error(`awesome-doc-code-sections.js:SendToGodboltButton::onClickSend: missing code`)
            return result
        }

        // build request as JSon
        let data = {
            "sessions": [{
                "id": 1,
                "language": get_language(),
                "source": get_code(),
                "compilers":  [
                    {
                        "id": get_ce_options().compiler_id || get_configuration().compiler_id,
                        "libs": get_ce_options().libs || [ ],
                        "options": get_ce_options().compilation_options || get_configuration().default_options
                    }
                ],
                "executors": [{
                    "compiler":
                    {
                        "id": get_ce_options().compiler_id || get_configuration().compiler_id,
                        "libs": get_ce_options().libs || [ ],
                        "options": get_ce_options().compilation_options || get_configuration().default_options
                    }
                    // TODO: exec
                }]
            }]
        };
        // CE /clientstate API
        ce_API.open_in_new_tab(data)
    }
}
customElements.define(SendToGodboltButton.HTMLElement_name, SendToGodboltButton, {extends: 'button'});

class BasicCodeSection extends HTMLElement {
// Basic code section (meaning, no metadata parsing), with synthax-coloration provided by highlightjs
// Additionaly, the language code language can be forced (`code_language` parameter, or `language` attributes),
// otherwise it is automatically detected based on fetched code content
//
// <BasicCodeSection language='cpp'>[some code here]</BasicCodeSection>
    
    static HTMLElement_name = 'awesome-doc-code-sections_basic-code-section'
    
    constructor(code, language) {
        super();

        try {
            // arguments
            if (code === undefined && this.textContent !== undefined)
                code = this.textContent
            if (! language)
                language = this.getAttribute('language') || undefined
            if (language !== undefined && !language.startsWith("language-"))
                language = `language-${language}`

            this.language = language
            this.code = code

            if (this.code === undefined || this.code.length == 0)
                throw 'invalid or empty code'
            this.load();
        }
        catch (error) {
            this.innerHTML = `<p style="color:red; border-style: solid; border-color: red;">awesome-doc-code-sections:BasicCodeSection: error : ${error}</p>`
        }
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

        // TODO: don't add CE button if language is not supported (ex: json, bash, console output, etc...)
        // let configuration = awesome_doc_code_sections.configuration.GodboltLanguages.get(codeSectionElement.hljs_language)
        // if (configuration !== undefined)

        let CE_button = new SendToGodboltButton
            CE_button.style.zIndex = code_node.style.zIndex + 1
        code_node.appendChild(CE_button)

        this.innerHTML = code_node.outerHTML;
    }

    static Initialize_DivHTMLElements() {
    // expected formats :
    //
    // <div class='awesome-doc-code-sections_basic-code-section' [language='cpp']>
    //  one line of code here
    // </div>
    //
    // <div class='awesome-doc-code-sections_basic-code-section' [language='cpp']>
    //   <pre>
    //      <code>
    //  mutiples
    //  lines of
    //  code here
    //      </code>
    //   </pre>
    // </div>

        let replace_by_HTMLElement = (index, value) => {
    
            let language = value.getAttribute('language')
            let code = value.textContent
                        .replace(/^\s+/g, '').replace(/\s+$/g, '') // remove enclosing empty lines
            let node = new BasicCodeSection(code, language);
                node.setAttribute('language', language)
            value.replaceWith(node);
        };

        let elements = $('body').find(`div[class=${BasicCodeSection.HTMLElement_name}]`)
        console.log(`awesome-doc-code-sections.js:CodeSection : Initialize_DivHTMLElements : replacing ${elements.length} element(s) ...`)
        elements.each(replace_by_HTMLElement)
    }

    get hljs_language() {
        let code = $(this).find("pre code")
        if (code.length == 0)
            console.error(`awesome-doc-code-sections.js:CodeSection::hljs_language(get): ill-formed element`)
        
        let result = code[0].classList.toString().replace(/hljs language-/g, '')
        if (result.indexOf(' ') !== -1)
            console.error(`awesome-doc-code-sections.js:CodeSection::hljs_language(get): ill-formed code hljs classList`)
        return result
    }
}
customElements.define(BasicCodeSection.HTMLElement_name, BasicCodeSection);

class CodeSection extends BasicCodeSection {
// Code section, with synthax-coloration provided by highlightjs
// Additionaly, the language code language can be forced (`code_language` parameter, or `language` attributes),
// otherwise it is automatically detected based on fetched code content
//
// <CodeSection language='cpp'>[some code here]</CodeSection>

    static HTMLElement_name = 'awesome-doc-code-sections_code-section'

    get ce_code() {
        return this.parsed_code.ce_code
    }
    get ce_options() {
        return this.parsed_code.ce_options
    }

    constructor(code, language) {
        let parsed_code = undefined
        try             { parsed_code = new ParsedCode(code) }
        catch (error)   {
            this.innerHTML = `<p style="color:red; border-style: solid; border-color: red;">awesome-doc-code-sections:CodeSection: error : ${error}</p>`
            return
        }
        super(parsed_code.code, language);
        this.parsed_code = parsed_code

        if (this.parsed_code.ce_options.add_in_doc_execution)
            this.#add_execution_panel()
    }

    #add_execution_panel() {

        // todo: loading panel
        let code_node = document.createElement('pre');
            code_node.style.zIndex = 1;
            code_node.style.position = 'relative'
        let code = code_node.appendChild(document.createElement('code'));
            code.textContent = this.code

        ce_API.fetch_execution_result(this.ce_options, this.ce_code)
            .then((result) => {
                console.log('fetched: ' + result)
            })
    }

    static Initialize_DivHTMLElements() {
    // expected formats :
    //
    // <div class='awesome-doc-code-sections_code-section' [language='cpp']>
    //  one line of code here
    // </div>
    //
    // <div class='awesome-doc-code-sections_code-section' [language='cpp']>
    //   <pre>
    //      <code>
    //  mutiples
    //  lines of
    //  code here
    //      </code>
    //   </pre>
    // </div>

        let replace_by_HTMLElement = (index, value) => {
    
            let language = value.getAttribute('language')
            let code = value.textContent
                        .replace(/^\s+/g, '').replace(/\s+$/g, '') // remove enclosing empty lines
            let node = new CodeSection(code, language);
                node.setAttribute('language', language)
            value.replaceWith(node);
        };

        let elements = $('body').find(`div[class=${CodeSection.HTMLElement_name}]`)
        console.log(`awesome-doc-code-sections.js:CodeSection : Initialize_DivHTMLElements : replacing ${elements.length} element(s) ...`)
        elements.each(replace_by_HTMLElement)
    }
}
customElements.define(CodeSection.HTMLElement_name, CodeSection);

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
            // super.code = code;
            super.parsed_code = new ParsedCode(code)
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
//
// Example:
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

            let theme_color = (html_node.classList.contains('dark-mode') ? 'dark' : 'light')
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
        console.log('awesome-doc-code-sections.js:onHighlightjsHrefChange: Switching highlighthjs stylesheet \n from : ' + mutation.oldValue + '\n to   : ' + code_stylesheet.href)

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

// ============

awesome_doc_code_sections.HTML_elements = {}
awesome_doc_code_sections.HTML_elements.CopyToClipboardButton = CopyToClipboardButton
awesome_doc_code_sections.HTML_elements.SendToGodboltButton   = SendToGodboltButton
awesome_doc_code_sections.HTML_elements.BasicCodeSection      = BasicCodeSection
awesome_doc_code_sections.HTML_elements.CodeSection           = CodeSection
awesome_doc_code_sections.HTML_elements.RemoteCodeSection     = RemoteCodeSection
awesome_doc_code_sections.ThemeSelector = ThemeSelector

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

        // WIP: keep doc ref links
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

    // WIP: documentation reference links
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

            if (awesome_doc_code_sections.options.toggle_dark_mode) {
                if (undefined === awesome_doc_code_sections.ToggleDarkMode)
                    console.error(
                        'awesome-doc-code-sections.js:initialize: options toggle_dark_mode set to true, but awesome_doc_code_sections.ToggleDarkMode is undefined\n' +
                        'Did you forget to include awesome-doc-code-sections_dark-mode.js ?'
                    )
                else
                    awesome_doc_code_sections.ToggleDarkMode.initialize()
            }

            // replace <select/> with proper HTML elements
            awesome_doc_code_sections.ThemeSelector.Initialize_SelectHTMLElements();
            // replace <div/> with proper HTML elements
            // TODO: Make generic
            awesome_doc_code_sections.HTML_elements.BasicCodeSection.Initialize_DivHTMLElements();
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