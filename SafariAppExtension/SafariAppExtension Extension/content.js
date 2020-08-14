//console.log("injection content script on " + document.location.href);

//to keep extension alive when debugging in safari, deactivate for production
setInterval(() => safari.extension.dispatchMessage("ping", {}), 3000);

window.addEventListener("message", function(event) {
    // We only accept messages from ourselves
    if (event.source !== window) {
      return;
    }

    if (event.data.src && (event.data.src === "page.js")) {
        event.data.origin = location.origin;
        //safari.runtime.sendMessage(event.data, function(response) {});
       safari.extension.dispatchMessage("message_toapp", event.data );
    }
}, false);

//must be "message" event here (answer from AppExtension)
safari.self.addEventListener("message", function(event) {
   console.log(event.name + event.message.response);
   window.postMessage(event.message, '*');
});

// inject page.js to the DOM of every page
var s = document.createElement('script');
s.src = safari.extension.baseURI + "page.js";
// remove script tag after script itself has loaded
s.onload = function() {this.parentNode.removeChild(this);};
(document.head || document.documentElement).appendChild(s);

