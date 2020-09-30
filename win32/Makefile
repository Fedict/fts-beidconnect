#
# native app: eidlink.exe (for all browsers needing a native app)
# activeX DLL: eidlink.dll (for internet Explorer)
# axtiveX DLL: eidlinkA.dll (for installation by Admins, to install in programfiles, looks for native eidlink.exe in programfiles)
#


# This is the Makefile for Windows NMake. See GNUmakefile for OSX/Linux.

!IF !DEFINED(BUILD_NUMBER)
BUILD_NUMBER=0
!ENDIF

MAJOR_VERSION=1
MINOR_VERSION=6
#RELEASE_VERSION=1
VERSION=$(MAJOR_VERSION).$(MINOR_VERSION)

SIGN = signtool sign /v /n "Zetes nv/sa" /fd SHA256 /du http://bosa.belgium.be /t http://timestamp.globalsign.com/scripts/timestamp.dll
WIX = C:\Program Files (x86)\WiX Toolset v3.10

HEAT_OUPUT_WXS = heat_output_admin.wxs

SOLUTION_X = ..\ActiveX\eIDLinkX.sln
PROGRAM_X = ..\ActiveX\eidlinkX\Release\eidlink.dll
PROGRAM_X_ADMIN = ..\ActiveX\eidlinkX\Release\eidlinkA.dll
BINARIES_X = ..\ActiveX\eidlinkX\Release
SOLUTION = VS_2019\eidlink.sln
PROGRAM = VS_2019\Release\eidlink.exe
BINARIES = VS_2019\Release
INSTALL_PRODUCT = eidlink_$(VERSION).msi
INSTALL_ADMIN_PRODUCT = eidlink_admin_$(VERSION).msi
BUILDCONSTANTS="LOG_INFO=1;WIN32;_WIN32_WINNT=0x0501;NDEBUG;_CRT_SECURE_NO_WARNINGS;_CONSOLE"

binaries:
	msbuild -m /p:Configuration=Release;Platform=x86 /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=0 /property:BUILD_NUMBER=$(BUILD_NUMBER) $(SOLUTION)
  	msbuild /p:Configuration=Release;Platform="Any CPU" /property:AssemblyName=eIDLink /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=0 /property:BUILD_NUMBER=$(BUILD_NUMBER) $(SOLUTION_X)
   msbuild /p:Configuration=ReleaseAdmin;Platform="Any CPU" /property:AssemblyName=eIDLinkA /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=0 /property:BUILD_NUMBER=$(BUILD_NUMBER) $(SOLUTION_X)
   
virtualcards:
   msbuild /p:Configuration=virtual_cards;Platform=x86 /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=$(RELEASE_VERSION) /property:BUILD_NUMBER=$(BUILD_NUMBER) VS_2019\eidlink.sln
   
pkg: binaries
   $(SIGN) $(PROGRAM_X)
	$(SIGN) $(PROGRAM)
	"$(WIX)\bin\candle.exe" eidlink.wxs -dVERSION=$(VERSION) -ext WixUtilExtension
	"$(WIX)\bin\light.exe" -out $(INSTALL_PRODUCT) eidlink.wixobj -v -ext WixUtilExtension -ext WixUIExtension -dWixUILicenseRtf=install/LICENSE.rtf -dWixUIDialogBmp=install/install_dlg.bmp
	$(SIGN) $(INSTALL_PRODUCT)
   copy $(INSTALL_PRODUCT) eidlink.msi
   
pkgadmin: binaries
   $(SIGN) $(PROGRAM_X_ADMIN)
	$(SIGN) $(PROGRAM)
	"$(WIX)\bin\candle.exe" eidlink_admin.wxs -dVERSION=$(VERSION) -ext WixUtilExtension
	"$(WIX)\bin\light.exe" -out $(INSTALL_ADMIN_PRODUCT) eidlink_admin.wixobj -v -ext WixUtilExtension -ext WixUIExtension -dWixUILicenseRtf=install/LICENSE.rtf -dWixUIDialogBmp=install/install_dlg.bmp
	$(SIGN) $(INSTALL_ADMIN_PRODUCT)
   
pkg-unsigned: binaries
	"$(WIX)\bin\candle.exe" eidlink.wxs -dVERSION=$(VERSION) -ext WixUtilExtension
	"$(WIX)\bin\light.exe" -out $(INSTALL_PRODUCT) eidlink.wixobj -ext WixUtilExtension -ext WixUIExtension -dWixUILicenseRtf=install/LICENSE.rtf -dWixUIDialogBmp=install/install_dlg.bmp
	
#the heat command (wix toolset bin folder) generates a wixtoolset install file that can be included in the main installer
#use this command to get the proper registry entries to add for the ActiveX component
#be careful not to recreate this with every release, it will also generate a new guid each time
genwix: binaries
   "$(WIX)\bin\Heat.exe" file $(PROGRAM_X_ADMIN) -dr INSTALLFOLDER -srd -gg -sfrag -suid -out $(HEAT_OUPUT_WXS)

clean:
   del /Q $(BINARIES_X)
   del /Q $(BINARIES)
   del /Q $(INSTALL_PRODUCT)
   del /Q $(INSTALL_ADMIN_PRODUCT)
   del /Q $(HEAT_OUPUT_WXS)


test: binaries
	python ../test/test.py
