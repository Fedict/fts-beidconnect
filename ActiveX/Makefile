#
# BeIDConnect Active X component
#



# This is the Makefile for Windows NMake. See GNUmakefile for OSX/Linux.
!IF !DEFINED(BUILD_NUMBER)
BUILD_NUMBER=0
!ENDIF

MAJOR_VERSION=2
MINOR_VERSION=7
RELEASE_VERSION=0
VERSION=$(MAJOR_VERSION).$(MINOR_VERSION)

SIGN = signtool sign /v /n "Federale Overheidsdienst Beleid en Ondersteuning" /fd SHA256 /tr http://rfc3161timestamp.globalsign.com/advanced /td SHA256
WIX = C:\Program Files (x86)\WiX Toolset v3.11

SOLUTION_X = BeIDConnectX.sln
PROGRAM_X = beidconnectX\Release\beidconnect.dll
PROGRAM_X_ADMIN = beidconnectX\Release\beidconnectA.dll
PROGRAM = ..\win32\VS_2019\Release\beidconnect.exe
BINARIES = beidconnectX\Release
INSTALL_PRODUCT = beidconnectX_$(VERSION).msi
INSTALL_ADMIN_PRODUCT = beidconnectX_admin_$(VERSION).msi
HEAT_OUPUT_WXS = beidconnectX\install\heat_output.wxs
HEAT_OUPUT_ADMIN_WXS = beidconnectX\install\heat_output_admin.wxs

all: pkg pkgadmin

binaries:
   msbuild /p:Configuration=Release;Platform="Any CPU" /property:AssemblyName=BeIDConnect /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=0 /property:BUILD_NUMBER=$(BUILD_NUMBER) $(SOLUTION_X)
   msbuild /p:Configuration=ReleaseAdmin;Platform="Any CPU" /property:AssemblyName=BeIDConnectA /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=0 /property:BUILD_NUMBER=$(BUILD_NUMBER) $(SOLUTION_X)

pkgadmin: binaries
 	$(SIGN) $(PROGRAM_X_ADMIN)
	$(SIGN) $(PROGRAM)
	"$(WIX)\bin\candle.exe" beidconnectX_admin.wxs -dVERSION=$(VERSION) -ext WixUtilExtension
	"$(WIX)\bin\light.exe" -out $(INSTALL_ADMIN_PRODUCT) beidconnectX_admin.wixobj -v -ext WixUtilExtension -ext WixUIExtension -dWixUILicenseRtf=beidconnectX/install/LICENSE.rtf -dWixUIDialogBmp=beidconnectX/install/install_dlg.bmp
	$(SIGN) $(INSTALL_ADMIN_PRODUCT)

pkg: binaries
	$(SIGN) $(PROGRAM_X)
	$(SIGN) $(PROGRAM)
	"$(WIX)\bin\candle.exe" beidconnectX.wxs -dVERSION=$(VERSION) -ext WixUtilExtension
	"$(WIX)\bin\light.exe" -out $(INSTALL_PRODUCT) beidconnectX.wixobj -v -ext WixUtilExtension -ext WixUIExtension -dWixUILicenseRtf=beidconnectX/install/LICENSE.rtf -dWixUIDialogBmp=beidconnectX/install/install_dlg.bmp
	$(SIGN) $(INSTALL_PRODUCT)

#the heat command generates a wixtoolset install file that can be included in the main installer
#use this command to get the proper registry entries to add
#be careful not to recreate this with every release, it will also generate a new guid each time
genwix: binaries
   "$(WIX)\bin\Heat.exe" file $(PROGRAM_X) -dr INSTALLFOLDER -srd -gg -sfrag -suid -out $(HEAT_OUPUT_WXS)
   "$(WIX)\bin\Heat.exe" file $(PROGRAM_X_ADMIN) -dr INSTALLFOLDER -srd -gg -sfrag -suid -out $(HEAT_OUPUT_ADMIN_WXS)

clean:
   del /Q $(INSTALL_ADMIN_PRODUCT)
   del /Q $(INSTALL_PRODUCT)
   del /Q $(HEAT_OUPUT_WXS)
   del /Q $(HEAT_OUPUT_ADMIN_WXS)
   del /Q $(BINARIES)
   del /Q *.wixobj
   del /Q *.wixpdb
   
install:
	msiexec /i $(INSTALL_PRODUCT) /qn /L*V install.log
	
installadmin:
	msiexec /i $(INSTALL_ADMIN_PRODUCT) /qn /L*V installadmin.log
