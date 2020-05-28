Apple Notarisation of apps and disk images
================================================
Starting with Catalina, no apps can run without notarisation of the app or installation by Apple.
When notarising an app or bundle, some special tools provided by Apple need to be used to send the app to Apple for the notarisation process.
Luckily, this can be done for the .dmg file we built for eidlink in one step.
Apple will hierarchically step in the DMG and subsequent installation packages to notarise all internal installation packages, components and the eazylink binary itself

For the notarising, this needs to be done by a Apple developer (just like signing the app) and an app specific password.
This app specific password needs to be generated online when logging in into the dev account.
This password is then stored in the local keychain of the developer's PC.
Notarising takes up to half an hour or longer and the resulting uid (and email) can be used to see the logfile online to see if there are any problems with the notarisation process.
The resulting ticket can be stapled to the app or bundle. The result of the notarising is kept by Apple online.
Also for existing applications (and installed versions) the notarisation can still be done. 


	- Login Apple ID => https://appleid.apple.com/#!&page=signin

	- Create app specific password (just creates a password here (probably linked to account)
	- Store this pw In the local keychain (to avoid to have to put it directly in a script)
	
	xcrun altool --store-password-in-keychain-item "AC_PASSWORD" -u appleid@iCloud.com -p <password> cdwt-hsrb-gmcf-kvaq

	- use altool to get a table of providers, provider short names, and team IDs associated with a given username and password 

	xcrun altool --list-providers -u appleid@iCloud.Com -p "@keychain:AC_PASSWORD"

	- Notarize app (send app to Apple for notarisation)

	xcrun altool --notarize-app --primary-bundle-id be.bosa.eidlink --username appleid@iCloud.com --password "@keychain:AC_PASSWORD" --asc-provider BOSA --file eidlink_1.1.dmg

	- Wait for some time ....

	No errors uploading 'eidlink_1.1.dmg'.
	=> RequestUUID = aae0890f-dbdd-4824-811f-2313e8b8b054

	xcrun altool --notarization-history 0 -u appleid@iCloud.com -p "@keychain:AC_PASSWORD" --asc-provider companySA

	xcrun altool --notarization-info aae0890f-dbdd-4824-811f-2313e8b8b054 -u appleid@iCloud.com -p "@keychain:AC_PASSWORD"

	Staple the Ticket to Your Distribution
	Notarization produces a ticket that tells Gatekeeper that your app is notarized. 
	After notarization completes successfully, the next time any user attempts to run your app on macOS 10.14 or later, Gatekeeper finds the ticket online. This includes users who 	downloaded your app before notarization.
	You should also attach the ticket to your software using the stapler tool, so that future distributions include the ticket. This ensures that Gatekeeper can find the ticket even 	when a network connection isn't  available. To attach a ticket to your app, use the stapler tool:

	xcrun stapler staple "eidlink_1.1.dmg"

