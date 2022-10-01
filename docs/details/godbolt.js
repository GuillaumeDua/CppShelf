
class CopyToClipboardButton extends HTMLButtonElement {

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

            let text = this.previousSibling.textContent
            navigator.clipboard.writeText(text).then(
                function() {
                    console.log('CopyToClipboardButton: success');
                },
                function(err) {
                    console.error('CopyToClipboardButton: failed: ', err);
                }
            );
            window.setTimeout(() => {
                this.innerHTML = CopyToClipboardButton.copyIcon
            }, CopyToClipboardButton.successDuration);    

        }
    }
}
customElements.define('copy-to-clipboard-button', CopyToClipboardButton, {extends: 'button'});

function send_to_godbolt(element) {
    console.log('sending : ' + element)
}

class SendToGodboltButton extends HTMLButtonElement {

    static title = 'Try this on godbolt.org (compiler-explorer)'
    static icon = 
        `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="32" height="32">
        <switch><g><path d="M58.6 46.5c-.3-.5-.3-1.2 0-1.7.3-.6.7-1.3 1-2 .2-.5-.1-1-.7-1h-5.8c-.6 0-1.2.3-1.4.8-.7 1.1-1.6 2.2-2.6 3.2-3.7 3.7-8.6 5.7-13.9 5.7-5.3 0-10.2-2-13.9-5.7-3.8-3.7-5.8-8.6-5.8-13.9s2-10.2 5.8-13.9c3.7-3.7 8.6-5.7 13.9-5.7 5.3 0 10.2 2 13.9 5.7 1 1 1.9 2.1 2.6 3.2.3.5.9.8 1.4.8h5.8c.5 0 .9-.5.7-1-.3-.7-.6-1.3-1-2-.3-.5-.3-1.2 0-1.7l1.9-3.5c.4-.7.3-1.5-.3-2.1l-4.9-4.9c-.6-.6-1.4-.7-2.1-.3l-3.6 2c-.5.3-1.2.3-1.7 0-1.7-.9-3.5-1.7-5.4-2.2-.6-.2-1-.6-1.2-1.2l-1.1-3.9C40.1.5 39.5 0 38.7 0h-6.9C31 0 30.2.5 30 1.3l-1.1 3.9c-.2.6-.6 1-1.2 1.2-1.9.6-3.6 1.3-5.3 2.2-.5.3-1.2.3-1.7 0l-3.6-2c-.7-.4-1.5-.3-2.1.3l-4.9 4.9c-.6.6-.7 1.4-.3 2.1l2 3.6c.3.5.3 1.2 0 1.7-.9 1.7-1.7 3.5-2.2 5.3-.2.6-.6 1-1.2 1.2l-3.9 1.1c-.7.2-1.3.9-1.3 1.7v6.9c0 .8.5 1.5 1.3 1.7l3.9 1.1c.6.2 1 .6 1.2 1.2.5 1.9 1.3 3.6 2.2 5.3.3.6.3 1.2 0 1.7l-2 3.6c-.4.7-.3 1.5.3 2.1L15 57c.6.6 1.4.7 2.1.3l3.6-2c.6-.3 1.2-.3 1.7 0 1.7.9 3.5 1.7 5.3 2.2.6.2 1 .6 1.2 1.2l1.1 3.9c.2.7.9 1.3 1.7 1.3h6.9c.8 0 1.5-.5 1.7-1.3l1.1-3.9c.2-.6.6-1 1.2-1.2 1.9-.6 3.6-1.3 5.4-2.2.5-.3 1.2-.3 1.7 0l3.6 2c.7.4 1.5.3 2.1-.3l4.9-4.9c.6-.6.7-1.4.3-2.1l-2-3.5z" fill="#67c52a"/><path d="M23.5 37.7v4.4h23.8v-4.4H23.5zm0-7.8v4.4h19.6v-4.4H23.5zm0-7.9v4.4h23.8V22H23.5z" fill="#3c3c3f"/>
        </g></switch></svg>`;

    connectedCallback() {
        console.log("SendToGodboltButton : connectedCallback")
    }
    constructor() {
        console.log("SendToGodboltButton : constructor")

        super();
        this.title = SendToGodboltButton.title;
        this.innerHTML = SendToGodboltButton.icon;
        this.style.zIndex = 2;
        this.style.position = 'absolute';
        this.style.top = 10 + 'px';
        this.style.right = 10 + 'px';
        this.addEventListener('click', () => console.log("SendToGodboltButton clicked !")); // TODO: godbolt CE API (and inject csl::ag header include<raw_github_path.hpp>)
    }
}
customElements.define('send-to-godbolt-button', SendToGodboltButton, {extends: 'button'});

// Todo: Remove xhr (or make another component for remove ressources)
//  local code had better to be injected
//  - godbolt_snippet
//  - godbolt_remote_snippet (or a factory for remote ressources that generates a godbolt_snippet)
class godbolt_snippet extends HTMLElement {

    connectedCallback() {
        // console.log('godbolt_snippet: connectedCallback with url attribute : ' + this.getAttribute('url'));
    }

    constructor(code_url) {
        super();

        if (code_url === undefined && this.getAttribute('url') != undefined)
            code_url = this.getAttribute('url')

        // TODO: parse for specific format (or use regex + specific tags ?)
        // - expected output tag
        // - {code}{separator}{expected_output}
       
        this.code_url = code_url;
        this.load();
    }

    load() {

        let _this = this;

        let xhr = new XMLHttpRequest();
        xhr.open('GET', this.code_url); // TODO: async
        xhr.onerror = function() {
            console.log(`godbolt_snippet: Network Error`);
        };
        xhr.onload = function() {
            console.log(`godbolt_snippet: Loaded with status: ${xhr.status}`);
            if (xhr.status != 200) {
                return;
            }

            _this.code = xhr.responseText

            let code_node = document.createElement('pre');
                code_node.style.zIndex = 1;
                code_node.style.position = 'relative'
            let code = code_node.appendChild(document.createElement('code'));
                code.textContent = _this.code
//                 code.textContent = `auto i = int{ 42 };
// ++i; // comment
// return generate_fun() + unicorns_everywhere<42>();`

            let copy_button = new CopyToClipboardButton()
                copy_button.style.zIndex = code_node.style.zIndex + 1
            code_node.appendChild(copy_button)

            let CE_button = new SendToGodboltButton
                CE_button.style.zIndex = code_node.style.zIndex + 1
            code_node.appendChild(CE_button)
            _this.innerHTML = code_node.outerHTML;

            hljs.highlightAll(); // TODO: only this one
        };
        xhr.send();
    }

    static init() {
        $(function() {
            $(document).ready(function() {
          
            })
        })
    }
}
window.customElements.define('godbolt-snippet', godbolt_snippet);

// TODO : same with old/previous table/tr/th/td ?

function inject_examples() {
// expected format : <div class='code_example' url='path/to/code'></div>
    var place_holders = $('body').find('div[class=code_example]');
    place_holders.each((index, value) => {

        if (value.getAttribute('url') === undefined) {
            console.log('godbolt.js: warning: code_example is missing an url attribute')
            return true; // ill-formed, skip this element but continue iteration
        }
        console.log('processing example ' + index + ' with index ' + value.getAttribute('url') + ' ...')

        // const example_url = 'https://raw.githubusercontent.com/GuillaumeDua/CppShelf/gh-pages/examples/' + value.id;
        const example_url = 'https://raw.githubusercontent.com/GuillaumeDua/CppShelf/main/.gitignore';
        // const example_url = document.URL.split('/').slice(0, -1).join('/') + '/../../../' + value.getAttribute('url')

        let example_element = new godbolt_snippet(example_url);
            example_element.setAttribute('url', example_url);
        value.appendChild(example_element);
    });
}

function godbolt_js_initialize() {

    $(function() {
        $(document).ready(function() {
            console.log('initializing code sections ...')
            inject_examples();
        })
    })
}

// TODO : highlightjs theme picker
// - Tokyo Night Dark
// - Base16/Google Dark