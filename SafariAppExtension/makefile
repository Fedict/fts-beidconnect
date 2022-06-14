VERSION = 2.6
APPIDENTIFIER = be.bosa.beidconnect.safari
APPEXIDENTIFIER = be.bosa.beidconnect.safari-extension
PROGRAM = BeIDConnect\ Extension
DISTDIR = ./install/dist
DEPSDIR = ./install/deps
BUILDDIR = ./build
APP = ./Build/Release/BeIDConnect\ Extension.app
APPEX = $(APP)/Contents/PlugIns/BeIDConnect\ Extension.appex/Contents/MacOS/BeIDConnect\ Extension
BINARIES = /tmp/BeIDConnect.dst
DMGFILE = BeIDConnect_Extension_$(VERSION).dmg
INSTALL_PRODUCT = $(DISTDIR)/$(PROGRAM)-install.pkg
INSTALL_RESOURCES = ./install/install_resources
INSTALL_COMPONENT = $(DEPSDIR)/BeIDConnectComponent.pkg
COMPONENT_PFILE = ./install/BeIDConnectComponent.plist
SCRIPTS = ./install/scripts
INSTALL_DISTRIBUTION_FILE = ./install/distribution.plist
REQUIREMENTS =./install/requirements.plist


.PHONY : all
all: $(DISTDIR) $(DEPSDIR) $(DMGFILE)

$(DISTDIR):
	mkdir $(DISTDIR)
	cp $(INSTALL_RESOURCES)/README.html $(DISTDIR)/
	cp $(INSTALL_RESOURCES)/LICENSE.html $(DISTDIR)/

$(DEPSDIR) :
	mkdir $(DEPSDIR)

$(DMGFILE): $(INSTALL_PRODUCT)
	hdiutil create -fs JHFS+ -volname "BeIDConnect Extension Installer" -srcfolder $(DISTDIR) -ov $(DMGFILE)
	codesign -s "3rd Party Mac Developer Application: FOD BOSA (7682K6YL8X)" --timestamp -i $(APPIDENTIFIER) $(DMGFILE)

$(INSTALL_PRODUCT) : $(BINARIES) $(REQUIREMENTS) $(DEPENDENCY) $(COMPONENT_PFILE) $(INSTALL_COMPONENT) $(INSTALL_DISTRIBUTION_FILE)
	productbuild --distribution $(INSTALL_DISTRIBUTION_FILE) \
	--sign "Developer ID Installer: FOD BOSA (7682K6YL8X)" \
	--timestamp \
	--resources $(INSTALL_RESOURCES) \
	--package-path $(DEPSDIR) \
	$(INSTALL_PRODUCT)

$(BINARIES):
	xcodebuild install -project "BeIDConnect.xcodeproj"
#	codesign -f --options=runtime -s "3rd Party Mac Developer Application: FOD BOSA (7682K6YL8X)" --identifier -v ./Build/Release/BeIDConnect\ Extension.app/Contents/XPCServices/BeIDConnect\ Extension.appex/Contents/MacOS/BeIDConnect\ Extension
#	codesign -f --options=runtime -s "3rd Party Mac Developer Application: FOD BOSA (7682K6YL8X)" -v ./Build/Release/BeIDConnect\ Extension.app
	
$(COMPONENT_PFILE) :
	@echo "Error: Missing component pfile."
	@echo "Create a component pfile with make compfiles."
	@exit 1

$(INSTALL_COMPONENT) : $(BINARIES) $(COMPONENT_PFILE)
	pkgbuild --root $(BINARIES) \
	--scripts $(SCRIPTS) \
	--component-plist $(COMPONENT_PFILE) \
	$(INSTALL_COMPONENT) \
	--identifier $(APPIDENTIFIER) \
	--version $(VERSION)

$(INSTALL_DISTRIBUTION_FILE) :
	@echo "Error: Missing distribution file."
	@echo "Create a distribution file with make distfiles."
	@exit 1

.PHONY : distfiles
distfiles : $(INSTALL_COMPONENT)
	productbuild --synthesize \
	--product $(REQUIREMENTS) \
	--package $(INSTALL_COMPONENT) \
	$(DISTRIBUTION_FILE).new
	@echo "Edit the $(DISTRIBUTION_FILE).new template to create a suitable $(DISTRIBUTION_FILE) file."
	
.PHONY : compfiles
compfiles : $(BINARIES)
	pkgbuild --analyze --root $(BINARIES) $(COMPONENT_PFILE).new
	@echo "Edit the $(COMPONENT_PFILE).new template to create a suitable $(COMPONENT_PFILE) file."

notarise: $(DMGFILE)
	xcrun altool --notarize-app --primary-bundle-id $(APPIDENTIFIER) --username christophe.peerens@bosa.fgov.be --password "@keychain:AC_PASSWORD" --asc-provider 7682K6YL8X --file $(DMGFILE)

staple: 
	xcrun stapler staple $(DMGFILE)



.PHONY : clean
clean:
	-rm -f $(DMGFILE) $(INSTALL_PRODUCT) $(INSTALL_COMPONENT)
	-rm -rf $(BINARIES)
	-rm -rf $(DISTDIR)
	-rm -rf $(BUILDDIR)
