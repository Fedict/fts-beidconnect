#
# eIDLink Active X component
#



# This is the Makefile for Windows NMake. See GNUmakefile for OSX/Linux.

!IF !DEFINED(BUILD_NUMBER)
BUILD_NUMBER=0
!ENDIF

MAJOR_VERSION=1
MINOR_VERSION=2
RELEASE_VERSION=0
VERSION=$(MAJOR_VERSION).$(MINOR_VERSION)

SIGN = signtool sign /v /n "Zetes nv/sa" /fd SHA256 /du http://bosa.belgium.be /t http://timestamp.globalsign.com/scripts/timestamp.dll
WIX = C:\Program Files (x86)\WiX Toolset v3.10

PROGRAM_X = eidlinkX\bin\Release\eidlink.dll
PROGRAM = eidlinkX\install\eidlink.exe
BINARIES = eidlinkX\bin\Release
INSTALL_PRODUCT = eidlinkX_$(VERSION).msi
HEAT_OUPUT_WXS = eidlinkX\install\heat_output.wxs

binaries:
	msbuild /p:Configuration=Release;Platform="Any CPU" /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=$(RELEASE_VERSION) /property:BUILD_NUMBER=$(BUILD_NUMBER) eIDLinkX.sln
   
pkg: binaries
	$(SIGN) $(PROGRAM_X)
	$(SIGN) $(PROGRAM)
	"$(WIX)\bin\candle.exe" eidlink.wxs -dVERSION=$(VERSION) -ext WixUtilExtension
	"$(WIX)\bin\light.exe" -out eidlinkX_$(VERSION).msi eidlink.wixobj -v -ext WixUtilExtension -ext WixUIExtension -dWixUILicenseRtf=eidlinkX/install/LICENSE.rtf -dWixUIDialogBmp=eidlinkX/install/install_dlg.bmp
	$(SIGN) eidlinkX_$(VERSION).msi

pkguser: binaries
	$(SIGN) $(PROGRAM_X)
	$(SIGN) $(PROGRAM)
	"$(WIX)\bin\candle.exe" eidlinkuser.wxs -dVERSION=$(VERSION) -ext WixUtilExtension
	"$(WIX)\bin\light.exe" -out eidlinkXU_$(VERSION).msi eidlinkuser.wixobj -v -ext WixUtilExtension -ext WixUIExtension -dWixUILicenseRtf=eidlinkX/install/LICENSE.rtf -dWixUIDialogBmp=eidlinkX/install/install_dlg.bmp
	$(SIGN) eidlinkXU_$(VERSION).msi


signdll:
   $(SIGN) eidlinkX\bin\Release\eidlink.dll

pkg-unsigned: binaries
	"$(WIX)\bin\candle.exe" eazylink.wxs -dVERSION=$(VERSION) -ext WixUtilExtension
   "$(WIX)\bin\candle.exe" eazylink_db.wxs -dVERSION=$(VERSION) -ext WixUtilExtension
	"$(WIX)\bin\light.exe" -out eazylinkX_$(VERSION).msi eazylink.wixobj -v -ext WixUtilExtension -ext WixUIExtension -dWixUILicenseRtf=eazylink/install/LICENSE.rtf -dWixUIDialogBmp=eidlink/install/dlgbmp.bmp

#the heat command generates a wixtoolset install file that can be included in the main installer
#use this command to get the proper registry entries to add
#be careful not to recreate this with every release, it will also generate a new guid each time
genwix: binaries
   "$(WIX)\bin\Heat.exe" file $(PROGRAM_X) -dr INSTALLFOLDER -srd -gg -sfrag -suid -out $(HEAT_OUPUT_WXS)

clean:
   del /Q eidlinkXU_$(VERSION).msi
   del /Q $(HEAT_OUPUT_WXS)
   del /Q $(BINARIES)
   del /Q $(INSTALL_PRODUCT)
