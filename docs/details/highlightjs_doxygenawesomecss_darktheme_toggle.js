// highlightjs + doxygen-awesome-css + dark/light-mode == <3
//  prerequisites : doxygen-awesome-css-darkmode-toggle
//  prerequisites : highlightjs

function onHTMLClassChange(mutationsList, observer) {
    mutationsList.forEach(mutation => {
        if (mutation.attributeName !== 'class')
            return;

        let html_node = document.getElementsByTagName('html')[0];

        if (mutation.oldValue !== null &&
            mutation.oldValue !== "dark-mode" &&
            mutation.oldValue !== "light-mode")
            return;
        if (mutation.oldValue === html_node.className)
            return;

        let code_stylesheet = document.getElementById('code_theme_stylesheet');

        console.log('onHTMLClassChange: Switching theme from : ' + mutation.oldValue + ' to ' + html_node.className);
        
        if (html_node.className === 'dark-mode') {
            code_stylesheet.href = code_stylesheet.href.replace('-light', '-dark')
        }
        else {
            code_stylesheet.href = code_stylesheet.href.replace('-dark', '-light')
        }
    })
}

const html_class_mutationObserver = new MutationObserver(onHTMLClassChange);
html_class_mutationObserver.observe(
    document.getElementsByTagName('html')[0],
    { 
        attributes: true,
        attributeFilter: [ 'class' ],
        attributeOldValue: true
    }
);

function onHighlightjsHrefChange(mutationsList, observer) {

    mutationsList.forEach(mutation => {
        if (mutation.attributeName !== 'href')
            return;

        let code_stylesheet = document.getElementById('code_theme_stylesheet');
        if (mutation.oldValue === code_stylesheet.href)
            return
        console.log('onHighlightjsHrefChange: Switching highlighths stylesheet \n from : ' + mutation.oldValue + '\n to   : ' + code_stylesheet.href)

        hljs.highlightAll();
    })
}

const highlightjs_stylesheet_href_mutationObserver = new MutationObserver(onHighlightjsHrefChange);
highlightjs_stylesheet_href_mutationObserver.observe(
    document.getElementById('code_theme_stylesheet'),
    { 
        attributes: true,
        attributeFilter: [ 'href' ],
        attributeOldValue: true
    }
)
