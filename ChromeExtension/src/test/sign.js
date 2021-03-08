
function sign() {

  if (!( "EIDChromeExt" in window)) {
    error("Extension not found");
     showExtensionLink();
    return;
  }

  console.log("Using extension to sign");

  var ext = new EIDChromeExt();
  var lang = $("#lang").val();
  var algo = $("#digestAlgo").val();
  var mac = $("#mac").val();  //should be generated on server
  var pin = $("#pin").val();  //should be generated on server
  var digestValue = $("#digestValue").val();

  console.log("get info...");
  ext.getInfo().then(function(infoResponse) {
	 console.log("extVersion: " + infoResponse.extensionVersion + " nativeVersion: " + infoResponse.nativeVersion);
	console.log("host app up_to_date?: " + infoResponse.uptodate);
	if (infoResponse.uptodate === false) {
		error("native host app not up to date. Please install latest version. ");
		showExtensionHostLink();
		return;
	}
	info("getCertificateChain...");
	ext.getCertificateChain(lang, mac).then(function(certResponse) {
	//var digest = { algo: algo, value: digestValue };
	//for example,use same mac, on server this should be new value
	info("sign...");
	ext.sign(lang, mac, certResponse.certificateChain.signing, algo, digestValue, pin).then(function(sigResponse) {
		success(sigResponse.signature);
	}, function(err) {
		error("sign() returned " + err);
	});
	}, function(err) {
		error("getCertificateChain() returned " + err);
	});
  }, function (err) {
  	  if (err.message === "timeout") {
  	  	error("getInfo(): timeout => no native host app installed")
		showExtensionHostLink();
  	  	}
  	  else {
        error("getInfo() returned " + err);
  	  }
  });
}

function error(msg) {
  $("#feedback").removeClass().addClass("alert alert-danger").text(msg);
}

function info(msg) {
  $("#feedback").removeClass().addClass("alert alert-info").text(msg);
}

function success(signature) {
  $("#feedback").removeClass().addClass("alert alert-success").text("Successfully signed!");
  $("#signature").text(signature);
}

function showExtensionLink() {
   $("#feedback").append(" <a href='#' onclick='chrome.webstore.install()' title='Install BeIDConnect Extension'><span class='glyphicon glyphicon-download-alt'></span></a>");
}

function showExtensionHostLink() {
   $("#feedback").append(" <a href='#' onclick=window.open('https://www.eazysign.be/chrome') title='Install BeIDConnect Extension Host app'><span class='glyphicon glyphicon-download-alt'></span></a>");
}

$("#sign-btn").on("click", sign);
