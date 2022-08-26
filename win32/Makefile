#
# native app: beidconnect.exe (for all browsers needing a native app)
# activeX DLL: beidconnect.dll (for internet Explorer)
# axtiveX DLL: beidconnectA.dll (for installation by Admins, to install in programfiles, looks for native beidconnect.exe in programfiles)
#


# This is the Makefile for Windows NMake. See GNUmakefile for OSX/Linux.

!IF !DEFINED(BUILD_NUMBER)
BUILD_NUMBER=0
!ENDIF

MAJOR_VERSION=2
MINOR_VERSION=7
VERSION=$(MAJOR_VERSION).$(MINOR_VERSION)

#SIGN = signtool sign /v /n "Federale Overheidsdienst Beleid en Ondersteuning" /fd SHA256 /tr http://rfc3161timestamp.globalsign.com/advanced /td SHA256
SIGN = signtool sign /fd SHA256 /a /f D:\cpe\BoSa\bosa_self_signed_test_cert.pfx /p bosa
WIX = C:\Program Files (x86)\WiX Toolset v3.11

HEAT_OUPUT_WXS = heat_output.wxs
HEAT_OUPUT_ADMIN_WXS = heat_output_admin.wxs

SOLUTION_X = ..\ActiveX\BeIDConnectX.sln
PROGRAM_X = ..\ActiveX\BeIDConnectX\bin\x86\Release\beidconnect.dll
PROGRAM_X_X64 = ..\ActiveX\BeIDConnectX\bin\x64\Release\beidconnect_x64.dll
PROGRAM_X_ADMIN = ..\ActiveX\BeIDConnectX\bin\x86\ReleaseAdmin\beidconnectA.dll
PROGRAM_X_ADMIN_X64 = ..\ActiveX\BeIDConnectX\bin\x64\ReleaseAdmin\beidconnectA_x64.dll
BINARIES_X = ..\ActiveX\BeIDConnectX\Release
SOLUTION = VS_2019\BeIDConnect.sln
PROGRAM = VS_2019\Release\beidconnect.exe
PROGRAM_X64 = VS_2019\x64\Release\beidconnect.exe
BINARIES = VS_2019\Release
INSTALL_PRODUCT = beidconnect_$(VERSION).msi
INSTALL_PRODUCT_NL = beidconnect_$(VERSION)_nl.msi
INSTALL_PRODUCT_FR = beidconnect_$(VERSION)_fr.msi
INSTALL_PRODUCT_DE = beidconnect_$(VERSION)_de.msi
INSTALL_PRODUCT_x64 = beidconnect_x64_$(VERSION).msi
INSTALL_PRODUCT_x64_NL = beidconnect_x64_$(VERSION)_nl.msi
INSTALL_PRODUCT_x64_FR = beidconnect_x64_$(VERSION)_fr.msi
INSTALL_PRODUCT_x64_DE = beidconnect_x64_$(VERSION)_de.msi
INSTALL_ADMIN_PRODUCT = beidconnect_admin_$(VERSION).msi
BUILDCONSTANTS="LOG_INFO=1;WIN32;_WIN32_WINNT=0x0501;NDEBUG;_CRT_SECURE_NO_WARNINGS;_CONSOLE"

all: pkg pkgadmin

binaries:
	msbuild -m /p:Configuration=Release;Platform=x86 /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=0 /property:BUILD_NUMBER=$(BUILD_NUMBER) $(SOLUTION)
	msbuild -m /p:Configuration=ReleaseAdmin;Platform=x86 /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=0 /property:BUILD_NUMBER=$(BUILD_NUMBER) $(SOLUTION)
	msbuild -m /p:Configuration=Release;Platform=x64 /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=0 /property:BUILD_NUMBER=$(BUILD_NUMBER) $(SOLUTION)
	msbuild /p:Configuration=Release;Platform=x86 /property:AssemblyName=BeIDConnect /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=0 /property:BUILD_NUMBER=$(BUILD_NUMBER) $(SOLUTION_X)
	msbuild /p:Configuration=ReleaseAdmin;Platform=x86 /property:AssemblyName=BeIDConnectA /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=0 /property:BUILD_NUMBER=$(BUILD_NUMBER) $(SOLUTION_X)
	msbuild /p:Configuration=Release;Platform=x64 /property:AssemblyName=BeIDConnect_x64 /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=0 /property:BUILD_NUMBER=$(BUILD_NUMBER) $(SOLUTION_X)
	msbuild /p:Configuration=ReleaseAdmin;Platform=x64 /property:AssemblyName=BeIDConnectA_x64 /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=0 /property:BUILD_NUMBER=$(BUILD_NUMBER) $(SOLUTION_X)
   
virtualcards:
	msbuild /p:Configuration=virtual_cards;Platform=x86 /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=$(RELEASE_VERSION) /property:BUILD_NUMBER=$(BUILD_NUMBER) VS_2019\beidconnect.sln
   
pkg: binaries
	$(SIGN) $(PROGRAM_X)
	$(SIGN) $(PROGRAM_X_X64)
	$(SIGN) $(PROGRAM)
	$(SIGN) $(PROGRAM_X64)
	"$(WIX)\bin\candle.exe" beidconnect.wxs -dPlatform=win32 -dVERSION=$(VERSION) -ext WixUtilExtension
	"$(WIX)\bin\light.exe" -out $(INSTALL_PRODUCT) beidconnect.wixobj -v -ext WixUtilExtension -ext WixUIExtension -cultures:en-us -dWixUILicenseRtf=install/LICENSE.rtf -dWixUIDialogBmp=install/install_dlg.bmp
	"$(WIX)\bin\light.exe" -out $(INSTALL_PRODUCT_NL) beidconnect.wixobj -v -ext WixUtilExtension -ext WixUIExtension -cultures:nl-nl; -dWixUILicenseRtf=install/LICENSE_NL.rtf -dWixUIDialogBmp=install/install_dlg.bmp
	"$(WIX)\bin\light.exe" -out $(INSTALL_PRODUCT_FR) beidconnect.wixobj -v -ext WixUtilExtension -ext WixUIExtension -cultures:fr-fr; -dWixUILicenseRtf=install/LICENSE_FR.rtf -dWixUIDialogBmp=install/install_dlg.bmp
	"$(WIX)\bin\light.exe" -out $(INSTALL_PRODUCT_DE) beidconnect.wixobj -v -ext WixUtilExtension -ext WixUIExtension -cultures:de-de -dWixUILicenseRtf=install/LICENSE_DE.rtf -dWixUIDialogBmp=install/install_dlg.bmp
	"$(WIX)\bin\candle.exe" beidconnect.wxs -dPlatform=x64 -dVERSION=$(VERSION) -ext WixUtilExtension -arch x64
	"$(WIX)\bin\light.exe" -out $(INSTALL_PRODUCT_x64) beidconnect.wixobj -v -ext WixUtilExtension -ext WixUIExtension -cultures:en-us -dWixUILicenseRtf=install/LICENSE.rtf -dWixUIDialogBmp=install/install_dlg.bmp
	"$(WIX)\bin\light.exe" -out $(INSTALL_PRODUCT_x64_NL) beidconnect.wixobj -v -ext WixUtilExtension -ext WixUIExtension -cultures:nl-nl; -dWixUILicenseRtf=install/LICENSE_NL.rtf -dWixUIDialogBmp=install/install_dlg.bmp
	"$(WIX)\bin\light.exe" -out $(INSTALL_PRODUCT_x64_FR) beidconnect.wixobj -v -ext WixUtilExtension -ext WixUIExtension -cultures:fr-fr; -dWixUILicenseRtf=install/LICENSE_FR.rtf -dWixUIDialogBmp=install/install_dlg.bmp
	"$(WIX)\bin\light.exe" -out $(INSTALL_PRODUCT_x64_DE) beidconnect.wixobj -v -ext WixUtilExtension -ext WixUIExtension -cultures:de-de -dWixUILicenseRtf=install/LICENSE_DE.rtf -dWixUIDialogBmp=install/install_dlg.bmp
	$(SIGN) $(INSTALL_PRODUCT)
	$(SIGN) $(INSTALL_PRODUCT_NL)
	$(SIGN) $(INSTALL_PRODUCT_FR)
	$(SIGN) $(INSTALL_PRODUCT_DE)
	$(SIGN) $(INSTALL_PRODUCT_x64)
	$(SIGN) $(INSTALL_PRODUCT_x64_NL)
	$(SIGN) $(INSTALL_PRODUCT_x64_FR)
	$(SIGN) $(INSTALL_PRODUCT_x64_DE)
   
pkgadmin: binaries
	$(SIGN) $(PROGRAM_X_ADMIN)
#$(SIGN) $(PROGRAM)
	"$(WIX)\bin\candle.exe" beidconnect_admin.wxs -dVERSION=$(VERSION) -ext WixUtilExtension
	"$(WIX)\bin\light.exe" -out $(INSTALL_ADMIN_PRODUCT) beidconnect_admin.wixobj -v -ext WixUtilExtension -ext WixUIExtension -dWixUILicenseRtf=install/LICENSE.rtf -dWixUIDialogBmp=install/install_dlg.bmp
	$(SIGN) $(INSTALL_ADMIN_PRODUCT)
   
pkg-unsigned: binaries
	"$(WIX)\bin\candle.exe" beidconnect.wxs -dVERSION=$(VERSION) -ext WixUtilExtension
	"$(WIX)\bin\light.exe" -out $(INSTALL_PRODUCT) beidconnect.wixobj -ext WixUtilExtension -ext WixUIExtension -dWixUILicenseRtf=install/LICENSE.rtf -dWixUIDialogBmp=install/install_dlg.bmp
	
#the heat command (wix toolset bin folder) generates a wixtoolset install file that can be included in the main installer
#use this command to get the proper registry entries to add for the ActiveX component
#be careful not to recreate this with every release, it will also generate a new guid each time
genwix: binaries
	"$(WIX)\bin\Heat.exe" file $(PROGRAM_X_ADMIN) -dr INSTALLFOLDER -srd -gg -sfrag -suid -out $(HEAT_OUPUT_ADMIN_WXS)
	"$(WIX)\bin\Heat.exe" file $(PROGRAM_X) -dr INSTALLFOLDER -srd -gg -sfrag -suid -out $(HEAT_OUPUT_WXS)

clean:
	del /Q $(BINARIES_X)
	del /Q $(BINARIES)
	del /Q $(HEAT_OUPUT_WXS)
	del /Q $(HEAT_OUPUT_ADMIN_WXS)
	del /Q *.wixobj
	del /Q *.wixpdb
	del /Q *.msi

test: binaries
	python ../test/test.py

install:
	msiexec /i $(INSTALL_PRODUCT) /qn /L*V install.log
	
installadmin:
	msiexec /i $(INSTALL_ADMIN_PRODUCT) /qn /L*V installadmin.log
	
