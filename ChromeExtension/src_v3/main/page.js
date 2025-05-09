// The test if the function is already existing resolve the case
// if an end user install both the Microsoft Edge browser extension
// and the Google Chrome browser extension.
// Both will inject the code, but only the first instanciated will be
// usable.
if (typeof EIDChromeExt !== 'function') {
	window.EIDChromeExt = function () {
		var pendingPromises = {};

		var ExtensionError = function (eventData) {
            Object.assign(this, eventData);
            // message is there for backward compatibility. Use result instead.
			this.message = eventData.result;
		};

		ExtensionError.prototype.toString = function () {
			return JSON.stringify(this);
		};

		function guid() {
			function s4() {
				return Math.floor((1 + Math.random()) * 0x10000)
					.toString(16)
					.substring(1);
			}
			return s4() + s4() + '-' + s4() + '-' + s4() + '-' +
				s4() + '-' + s4() + s4() + s4();
		}

		function messagePromise(msg) {
			return new Promise(function (resolve, reject) {
				msg.correlationId = guid();
				msg.src = 'beidconnect.page';

				window.postMessage(msg, "*");

				pendingPromises[msg.correlationId] = {
					resolve: resolve,
					reject: reject
				};
			});
		}

		window.addEventListener("message", function (event) {
			if (event.source !== window) return;
			if (event.data.src && (event.data.src === "beidconnect.background")) {
				console.log("Page received: ");
				console.log(event.data);

				if (event.data.correlationId) {
					var p = pendingPromises[event.data.correlationId];
					if (p === undefined) {
						console.log("No pending promise found, ignoring native reply");
						return;
					}

					delete pendingPromises[event.data.correlationId];

					if (event.data.result === "OK") {
						p.resolve(event.data);
					} else {
						p.reject(new ExtensionError(event.data));
					}
				} else {
					console.log("No correlationId in event msg");
				}
			}
		}, false);

		function isUptodate(minimumVersion, installedVersion) {
			var expected = minimumVersion.split(".");
			var actual = installedVersion.split(".");
			return (actual[0] > expected[0]) || (actual[0] === expected[0] && actual[1] >= expected[1]);
		}

		// [API]
		this.checkVersion = function (minimumVersion, onSuccess, onNotInstalled, onNeedsUpdate) {
			var currentInstance = this;
			this.getVersion().then(
				function (msg) {
					var installedVersion = msg.version;
					console.log("beidconnect version is " + installedVersion);

					if (isUptodate(minimumVersion, installedVersion)) {
						onSuccess(installedVersion);
					} else {
						onNeedsUpdate(installedVersion);
					}
				},
				function (err) {
					onNotInstalled();
				});
		};

		this.getVersion = function () {
			console.log("Getting version");
			return messagePromise({ operation: 'VERSION' });
		};

		this.sign = function (language, mac, cert, algo, digest, pin) {
			console.log("Signing");
			return messagePromise({ operation: 'SIGN', cert: cert, algo: algo, digest: digest, pin: pin, language: language, mac: mac });
		};

		this.auth = function (language, mac, cert, algo, digest, pin) {
			console.log("Authentication");
			return messagePromise({ operation: 'AUTH', cert: cert, algo: algo, digest: digest, language: language, mac: mac, pin: pin });
		};

		this.getUserCertificates = function (language, mac, keyusage) {
			console.log("Reading user certificates");
			keyusage = typeof keyusage === 'undefined' ? "" : keyusage;
			return messagePromise({ operation: 'USERCERTS', keyusage: keyusage, language: language, mac: mac });
		};

		this.getUserCertificateChain = function (language, mac, cert) {
			console.log("Getting certificate chain for specific cert");
			return messagePromise({ operation: 'CERTCHAIN', language: language, mac: mac, cert: cert });
		};

		this.getId = function (language, mac, idflags) {
			console.log("ID");
			return messagePromise({ operation: 'ID', idflags: idflags.toString(), language: language, mac: mac });
		};

		this.suspend = function () {
			console.log("Suspending pending promises");
			pendingPromises = {};
		};
		// [/API]

		console.log("EIDChromeExt interface initialized");
	};
}
else {
	console.log("EIDChromeExt function already defined");
}