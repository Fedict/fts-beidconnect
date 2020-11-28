
Some remarks with Safari Extensions
====================================

At the end of the installation the app is launched and the Sarari Preferences can be opened for extension with ID=be.bosa.eidapp-extension
When installing, sometimes (especially when reïnstalling from downloaded package), extension is not added to Safari for some reason.
Re-Installing can help. In debug from XCode this never seems to happen but from an installed app, this seems to be the case. (tested anyway)

App needs to be notarised.
Signing of app and extension is done in XCode project file when building.
When trying to overwrite the signature in makefile in cl build, this has some weird effect afterwards when installing
(app as well as appex needs to be signed for the notarisation to accept)

A "Developer ID Installer" certificate is needed to distribute the package outside of the mac app store !!!


