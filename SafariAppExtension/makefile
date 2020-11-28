VERSION = 1.7.0
APPIDENTIFIER = be.bosa.eidapp
APPEXIDENTIFIER = be.bosa.eidapp-extension
PROGRAM = Bosa\ eID\ App\ Extension
DISTDIR = ./install/dist
DEPSDIR = ./install/deps
BUILDDIR = ./build
#EXE = ./Build/Release/eIDLink
APP = ./Build/Release/Bosa\ eID\ App\ Extension.app
APPEX = $(APP)/Contents/PlugIns/BOSA\ eID\ App\ Extension.appex/Contents/MacOS/BOSA\ eID\ App\ Extension
BINARIES = /tmp/BOSA\ eID\ App.dst
DMGFILE = Bosa_eID_App_Extension_$(VERSION).dmg
INSTALL_PRODUCT = $(DISTDIR)/$(PROGRAM)-install.pkg
INSTALL_RESOURCES = ./install/install_resources
INSTALL_COMPONENT = $(DEPSDIR)/eIDAppComponent.pkg
COMPONENT_PFILE = ./install/eIDAppComponent.plist
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
	hdiutil create -fs JHFS+ -volname "Bosa eID App Extension Installer" -srcfolder $(DISTDIR) -ov $(DMGFILE)
	codesign -s "Developer ID Application: Zetes Sa" --timestamp -i $(APPIDENTIFIER) $(DMGFILE)
	cp $(DMGFILE) $(PROGRAM).dmg

$(INSTALL_PRODUCT) : $(BINARIES) $(REQUIREMENTS) $(DEPENDENCY) $(COMPONENT_PFILE) $(INSTALL_COMPONENT) $(INSTALL_DISTRIBUTION_FILE)
	productbuild --distribution $(INSTALL_DISTRIBUTION_FILE) \
	--sign "Developer ID Installer: Zetes Sa" \
	--timestamp \
	--resources $(INSTALL_RESOURCES) \
	--package-path $(DEPSDIR) \
	$(INSTALL_PRODUCT)

$(BINARIES):
	xcodebuild install -project "BOSA eID App.xcodeproj"
	codesign -f --options=runtime -s "Developer ID Application: Zetes Sa" --identifier -v ./Build/Release/Bosa\ eID\ App\ Extension.app/Contents/PlugIns/BOSA\ eID\ App\ Extension.appex/Contents/MacOS/BOSA\ eID\ App\ Extension
	codesign -f --options=runtime -s "Developer ID Application: Zetes Sa" -v ./Build/Release/Bosa\ eID\ App\ Extension.app
	
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
	xcrun altool --notarize-app --primary-bundle-id $(APPIDENTIFIER) --username "@keychain:AC_USER" --password "@keychain:AC_PASSWORD" --asc-provider ZetesSa --file $(PROGRAM).dmg

staple: 
	xcrun stapler staple $(PROGRAM).dmg



.PHONY : clean
clean:
	-rm -f $(DMGFILE) $(INSTALL_PRODUCT) $(INSTALL_COMPONENT)
	-rm -rf $(BINARIES)
	-rm -rf $(DISTDIR)
	-rm -rf $(BUILDDIR)
