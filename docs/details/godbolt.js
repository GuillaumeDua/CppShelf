
class godbolt_snippet extends HTMLElement {

    constructor(id) {
        super();
        // this.onclick = this.open_in_godbolt // TBD
        
        this.id = id;
        // this.path = 

        this.code = 'SOME CODE LOLZ'
        this.short_link_hash = 'x1dGTWddK';
        this.left_panel = 'toto';

        this.load();
    }

    load() {
        this.innerHTML = `
        <table>
        <tr><th>
            C++ code (
            <a href="https://godbolt.org/z/` + this.short_link_hash + `">
            Try me on compiler-explorer
            <img src="https://github.com/GuillaumeDua/CppShelf/blob/main/docs/details/images/compiler-explorer.png?raw=true" alt="" align="left" width="20" height="20" style="Padding: 2px 4px 0px 0px"/> </a>
            )
        </th><th> Console output </th></tr>
        <tr><td>
    
    \`\`\`cpp
    ` + this.code + `
    \`\`\`
    
    </td><td>
    
    \`\`\`
    ` + this.left_panel + `
    \`\`\`
    </td></tr></table>
    `
        ;
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
        // value.textContent
        // value.innerHTML = "<h2>TEST</h2>";
    });
}

function godbolt_js_initialize() {

    $(function() {
        $(document).ready(function() {
            inject_examples();
        })
    })
}
