// highlightjs + doxygen-awesome-css + dark/light-mode == <3
//  prerequisites : doxygen-awesome-css-darkmode-toggle
//  prerequisites : highlightjs

// TODO: list of (few) themes in doxygen footer bar ?

function switch_code_theme_to(integrity, href) {
    let code_stylesheet = document.getElementById('code_theme_stylesheet');
    code_stylesheet.integrity = integrity;
    code_stylesheet.href = href;
}

function callback(mutationsList, observer) {
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

        console.log('Switching highlightjs theme from : ' + mutation.oldValue + ' to ' + html_node.className);
        
        // hard-coded tokyo-night theme
        if (html_node.className === 'dark-mode') {
            switch_code_theme_to(
                "sha512-dSQLLtgaq2iGigmy9xowRshaMzUHeiIUTvJW/SkUpb1J+ImXOPNGAI7ZC8V5/PiN/XN83B8uIk4qET7AMhdC5Q==",
                "https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.6.0/styles/tokyo-night-dark.min.css"
            );
        }
        else {
            switch_code_theme_to(
                "sha512-XPIePliMtoEozJ99t+gktFvC8YVLKHzQH7T0RBtzEzNitkNh4IgQ5UAOdPT52ypYGjOHWkKSC4W77ZelU42LeA==",
                "https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.6.0/styles/tokyo-night-light.min.css"
            );
        }
        hljs.highlightAll();
    })
} // dark-mode

const mutationObserver = new MutationObserver(callback);
mutationObserver.observe(
    document.getElementsByTagName('html')[0],
    { 
        attributes: true,
        attributeFilter: [ 'class' ],
        attributeOldValue: true
    }
);
