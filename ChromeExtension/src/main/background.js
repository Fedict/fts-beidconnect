var NATIVE_HOST = "be.bosa.beidconnect";

console.log("BeIDConnect event page activated");

// When message is received from page send it to native
chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) {
    if (sender.id !== chrome.runtime.id) {
        console.log('Ignoring message not originating from our extension');
        return;
    }

    if (sender.tab) {
        request.tab = sender.tab.id;
        _forward(request);
    }
});

// Send the message back to the originating tab
function _reply(tab, msg) {
    msg.src = "beidconnect.background";
    msg.extensionVersion = chrome.runtime.getManifest().version;
    msg.extensionBrowser = "EXTENSIONPLATFORM";
    chrome.tabs.sendMessage(tab, msg);
}

function _fail_with(msg, result) {
    var resp = {};
    resp.correlationId = msg.correlationId;
    resp.result = result;
    _reply(msg.tab, resp);
}

function _forward(message) {
    var tabid = message.tab;
    //this will only copy simple objects,no functions whatsoever, doesn't matter if we filter out here
    var messageCopy = JSON.parse(JSON.stringify(message));

    if (message.pin) {
        message.pin = "********";
    }
    console.log("SEND " + tabid + ": " + JSON.stringify(message));
    browser.runtime.sendNativeMessage(NATIVE_HOST, messageCopy)
        .then((response) => {
            if (response) {
                console.log("RECV " + tabid + ": " + JSON.stringify(response));
                _reply(tabid, response);
            } else {
                console.log("ERROR " + tabid + ": " + JSON.stringify(chrome.runtime.lastError));
                if (chrome.runtime.lastError.message && chrome.runtime.lastError.message.indexOf("native messaging host not found") != -1) {
                    _fail_with(message, "timeout");
                } else {
                    _fail_with(message, "technical_error");
                }
            }
        })
        .catch((reason) => {
            console.log("ERROR CATCH " + tabid, reason);
            if (reason && reason.message && reason.message.startsWith("No such native application")) {
                _fail_with(message, "timeout");
            } else {
                _fail_with(message, "technical_error");
            }
        });
}
