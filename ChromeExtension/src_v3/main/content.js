window.addEventListener("message", function (event) {
    // We only accept messages from ourselves
    if (event.source !== window) {
        return;
    }

    if (event.data.src && (event.data.src === "beidconnect.page")) {
        event.data.origin = location.origin;
        chrome.runtime.sendMessage(event.data, function (response) { });
    }
}, false);

chrome.runtime.onMessage.addListener(function (request, sender, sendResponse) {
    window.postMessage(request, '*');
});

// inject page.js to the DOM of every page
var s = document.createElement('script');
s.src = chrome.runtime.getURL('page.js');

// remove script tag after script itself has loaded
s.onload = function () { this.parentNode.removeChild(this); };
(document.head || document.documentElement).appendChild(s);
