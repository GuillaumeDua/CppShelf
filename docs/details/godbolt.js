class SendToGodboltButton extends HTMLElement {

    static icon = 
        `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="64" height="64">
        <switch><g><path d="M58.6 46.5c-.3-.5-.3-1.2 0-1.7.3-.6.7-1.3 1-2 .2-.5-.1-1-.7-1h-5.8c-.6 0-1.2.3-1.4.8-.7 1.1-1.6 2.2-2.6 3.2-3.7 3.7-8.6 5.7-13.9 5.7-5.3 0-10.2-2-13.9-5.7-3.8-3.7-5.8-8.6-5.8-13.9s2-10.2 5.8-13.9c3.7-3.7 8.6-5.7 13.9-5.7 5.3 0 10.2 2 13.9 5.7 1 1 1.9 2.1 2.6 3.2.3.5.9.8 1.4.8h5.8c.5 0 .9-.5.7-1-.3-.7-.6-1.3-1-2-.3-.5-.3-1.2 0-1.7l1.9-3.5c.4-.7.3-1.5-.3-2.1l-4.9-4.9c-.6-.6-1.4-.7-2.1-.3l-3.6 2c-.5.3-1.2.3-1.7 0-1.7-.9-3.5-1.7-5.4-2.2-.6-.2-1-.6-1.2-1.2l-1.1-3.9C40.1.5 39.5 0 38.7 0h-6.9C31 0 30.2.5 30 1.3l-1.1 3.9c-.2.6-.6 1-1.2 1.2-1.9.6-3.6 1.3-5.3 2.2-.5.3-1.2.3-1.7 0l-3.6-2c-.7-.4-1.5-.3-2.1.3l-4.9 4.9c-.6.6-.7 1.4-.3 2.1l2 3.6c.3.5.3 1.2 0 1.7-.9 1.7-1.7 3.5-2.2 5.3-.2.6-.6 1-1.2 1.2l-3.9 1.1c-.7.2-1.3.9-1.3 1.7v6.9c0 .8.5 1.5 1.3 1.7l3.9 1.1c.6.2 1 .6 1.2 1.2.5 1.9 1.3 3.6 2.2 5.3.3.6.3 1.2 0 1.7l-2 3.6c-.4.7-.3 1.5.3 2.1L15 57c.6.6 1.4.7 2.1.3l3.6-2c.6-.3 1.2-.3 1.7 0 1.7.9 3.5 1.7 5.3 2.2.6.2 1 .6 1.2 1.2l1.1 3.9c.2.7.9 1.3 1.7 1.3h6.9c.8 0 1.5-.5 1.7-1.3l1.1-3.9c.2-.6.6-1 1.2-1.2 1.9-.6 3.6-1.3 5.4-2.2.5-.3 1.2-.3 1.7 0l3.6 2c.7.4 1.5.3 2.1-.3l4.9-4.9c.6-.6.7-1.4.3-2.1l-2-3.5z" fill="#67c52a"/><path d="M23.5 37.7v4.4h23.8v-4.4H23.5zm0-7.8v4.4h19.6v-4.4H23.5zm0-7.9v4.4h23.8V22H23.5z" fill="#3c3c3f"/></g></switch></svg>
        </svg>`;

    connectedCallback() {
        console.log("SendToGodboltButton : connectedCallback")
    }
    constructor() {
        console.log("SendToGodboltButton : constructor")

        super();
        this.innerHTML = icon + `Try me using Godbolt's compiler-explorer`;
        this.addEventListener('click', () => console.log("SendToGodboltButton clicked !")); // TODO: godbolt CE API
    }
}
customElements.define('send-to-godbolt-button', SendToGodboltButton, {extends: 'button'});


class godbolt_snippet extends HTMLElement {

    connectedCallback() {
        console.log('godbolt_snippet: connectedCallback with url attribute : ' + this.getAttribute('url'));
    }

    constructor(code_url) {
        console.log('godbolt_snippet: constructor: ' + code_url)
        super();
        // this.onclick = this.open_in_godbolt // TBD
        
        this.code_url = code_url;
        // this.short_link_hash = 'x1dGTWddK';

        this.load();
    }

    load() {

        let _this = this;

        let xhr = new XMLHttpRequest();
        xhr.open('GET', this.code_url); // TODO: async
        xhr.onerror = function() {
            console.log(`Network Error`);
        };
        xhr.onload = function() {
            console.log(`Loaded with status: ${xhr.status}`);
            if (xhr.status != 200) {
                return;
            }

            let code_node = document.createElement('pre');
            let code = code_node.appendChild(document.createElement('code'));
            code.textContent = `auto i = int{ 42 };
++i;
return generate_fun() + unicorns_everywhere<42>();`

            // WIP : remove the table, make the button overlap the code section
            _this.innerHTML = `
                <table class="doxtable"><tbody>
                    <tr>
                        <th>
                            Try me
                            <send-to-godbolt-button></send-to-godbolt-button>
                        </th>
                    </tr>
                    <tr><td>
                        {code}
                    </td></tr>
                </tbody></table>
            `.replace('{code}', code_node.outerHTML);
            

            hljs.highlightAll();

    //         `
    //     <table>
    //     <tr><th>
    //         C++ code (
    //         <a href="https://godbolt.org/z/` + this.short_link_hash + `">
    //         Try me on compiler-explorer
    //         <img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> </a>
    //         )
    //     </th><th> Console output </th></tr>
    //     <tr><td>
    
    // \`\`\`cpp
    // ` + xhr.response + `
    // \`\`\`
    
    // </td><td>
    
    // \`\`\`
    // ` + "TODO: TBD (split using comments with special token ?)" + `
    // \`\`\`
    // </td></tr></table>
    // `
    //     ;
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

function inject_examples() {
    var place_holders = $('body').find('div[class=code_example]');
    place_holders.each((index, value) => {

        if (value.id === undefined) {
            console.log('godbolt.js: warning: code_example is missing an id')
            return true; // ill-formed, skip this element but continue iteration
        }
        console.log('processing example ' + index + ' with index ' + value.id + ' ...')

        // const example_url = 'https://raw.githubusercontent.com/GuillaumeDua/CppShelf/gh-pages/examples/' + value.id;
        const example_url = 'https://raw.githubusercontent.com/GuillaumeDua/CppShelf/main/.gitignore';

        // TODO: trim-off first comment (or use regex + specific tags ?)

        // value = new godbolt_snippet(example_url);

        // value.textContent

        let example_element = new godbolt_snippet(example_url);
        example_element.setAttribute('url', example_url);
        value.appendChild(example_element);

        // value.innerHTML = `<h2>TEST</h2><godbolt_snippet url="${example_url}"></godbolt_snippet>`;

        // TODO: try me on godbolt button (or iframe)


//         let pre = document.createElement('pre');
//         let code = document.createElement('code');
//         code.className = 'language-cpp hljs';
//         code.textContent = `auto i = 42;
// ++i;
// using type = std::string;`;
//         // Tokyo Night Dark
//         // Base16/Google Dark
//         pre.appendChild(code);
//         hljs.highlightElement(code);
//         value.appendChild(pre);
    });
}

function godbolt_js_initialize() {

    $(function() {
        $(document).ready(function() {
            inject_examples();
            
        })
    })
}
