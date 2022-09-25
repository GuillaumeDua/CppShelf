
class SendToGodboltButton extends HTMLButtonElement {
    constructor() {
        super();
        this.addEventListener('click', () => alert("SendToGodboltButton!")); //TODO
    }
}
customElements.define('send-to-godbolt-button', SendToGodboltButton, {extends: 'button'});
// <button is="send-to-godbolt-button">Click me</button>

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

            this.parent = document.createElement('h2');

    //         let content = document.createElement('table');
    //         let content_first_row  = content.appendChild(document.createElement('tr'));
    //         let content_second_row = content.appendChild(document.createElement('tr'));
    //         let content_first_row_th = content_first_row.appendChild(document.createElement('th'));
    //         content_first_row_th.textContent = "C++ code";
            
    //             .setAttribute('href', 'https://godbolt.org')
    //             .textContent = 'Try me on compiler-explorer'
    //         ;
    //         content_first_row_th.appendChild(document.createElement('a'))

    //         this.parent.innerHTML = content;
            
            
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

        // let example_element = new godbolt_snippet(example_url);
        // example_element.setAttribute('url', example_url);
        // value.appendChild(example_element);

        // value.innerHTML = `<h2>TEST</h2><godbolt_snippet url="${example_url}"></godbolt_snippet>`;

        // TODO: try me on godbolt button (or iframe)
        let code = document.createElement('code');
        code.className = 'language-cpp';
        code.textContent = `
        auto i = 42;\n
        ++i;
        using type = std::string;
        `;
        // Tokyo Night Dark
        // Base16/Google Dark
        value.appendChild(code);

        hljs.highlightAll();
    });
}

function godbolt_js_initialize() {

    $(function() {
        $(document).ready(function() {
            inject_examples();
        })
    })
}
