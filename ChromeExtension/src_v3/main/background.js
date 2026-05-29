var NATIVE_HOST = "be.bosa.beidconnect";

console.log("BeIDConnect event page activated");

// When message is received from page send it to native
chrome.runtime.onMessage.addListener(function (request, sender, sendResponse) {
    if (sender.id !== chrome.runtime.id) {
        console.log('Ignoring message not originating from our extension');
        return undefined;
    }

    if (request.src === "beidconnect.page" && sender.tab) {
        request.tab = sender.tab.id;
        request.sendResponse = sendResponse;
        _forward(request);
        return true;
    }

    console.log('Sender Tab not defined');
    return false;
});

// Send the message back to the originating tab
function _reply(tab, msg) {
    msg.src = "beidconnect.background";
    msg.extensionVersion = chrome.runtime.getManifest().version;
    msg.extensionBrowser = "EXTENSIONPLATFORM";
    return chrome.tabs.sendMessage(tab, msg);
}

function _fail_with(msg, result, error) {
    var resp = {};
    resp.correlationId = msg.correlationId;
    resp.result = result;
    if (error) {
        resp.extensionError = error;
    }
    return _reply(msg.tab, resp);
}

function _forward(message) {
    var tabid = message.tab;
    //this will only copy simple objects,no functions whatsoever, doesn't matter if we filter out here
    var messageCopy = JSON.parse(JSON.stringify(message));

    if (message.pin) {
        message.pin = "********";
    }
    console.log("SEND " + tabid + ": " + JSON.stringify(message));
    return chrome.runtime.sendNativeMessage(NATIVE_HOST, messageCopy, function (response) {
        if (response) {
            console.log("RECV " + tabid + ": " + JSON.stringify(response));
            message.sendResponse({});
            return _reply(tabid, response);
        } else {
            console.log("ERROR " + tabid + ": " + JSON.stringify(chrome.runtime.lastError));
            if (chrome.runtime.lastError.message && chrome.runtime.lastError.message.indexOf("native messaging host not found") != -1) {
                message.sendResponse({});
                return _fail_with(message, "timeout", chrome.runtime.lastError.message);
                // } else if (chrome.runtime.lastError.message && chrome.runtime.lastError.message.indexOf("native messaging host was disabled") != -1) {
                //     // This error is thrown when the native messaging host is disabled in the registry
                //     // on Windows. This is not a problem with the extension itself, but rather
                //     // a configuration issue on the user's machine.
                //     // We can handle this error gracefully by informing the user that the native messaging host
                //     // is disabled and that they need to enable it in the registry.
                //     // the full message is: "Access to the native messaging host was disabled by the system administrator."
                //     message.sendResponse({});
                //     return _fail_with(message, "technical_error", chrome.runtime.lastError.message);
            } else {
                message.sendResponse({});
                return _fail_with(message, "technical_error", chrome.runtime.lastError.message);
            }
        }
    });
}
