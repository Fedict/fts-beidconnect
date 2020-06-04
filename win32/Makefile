#
# eidlink
#


# This is the Makefile for Windows NMake. See GNUmakefile for OSX/Linux.

!IF !DEFINED(BUILD_NUMBER)
BUILD_NUMBER=0
!ENDIF

MAJOR_VERSION=1
MINOR_VERSION=2
#RELEASE_VERSION=1
VERSION=$(MAJOR_VERSION).$(MINOR_VERSION)

SIGN = signtool sign /v /n "Zetes nv/sa" /fd SHA256 /du http://eazysign.be /t http://timestamp.globalsign.com/scripts/timestamp.dll
#SIGN = signtool sign /v /n "Zetes nv/sa" /fd SHA256 /du http://bosa.be /t http://timestamp.globalsign.com/scripts/timestamp.dll
WIX = C:\Program Files (x86)\WiX Toolset v3.10

PROGRAM = VS_2019\Release\eidlink.exe
BINARIES = VS_2019\Release
INSTALL_PRODUCT = eidlink_$(VERSION).msi
BUILDCONSTANTS="LOG_INFO=1;WIN32;_WIN32_WINNT=0x0501;NDEBUG;_CRT_SECURE_NO_WARNINGS;_CONSOLE"

binaries:
	msbuild -m /p:Configuration=Release;Platform=x86 /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=0 /property:BUILD_NUMBER=$(BUILD_NUMBER) VS_2019\eidlink.sln
   
virtualcards:
   msbuild /p:Configuration=virtual_cards;Platform=x86 /property:MAJOR_VERSION=$(MAJOR_VERSION) /property:MINOR_VERSION=$(MINOR_VERSION) /property:RELEASE_VERSION=$(RELEASE_VERSION) /property:BUILD_NUMBER=$(BUILD_NUMBER) VS_2019\eidlink.sln
   
pkg: binaries
	$(SIGN) $(PROGRAM)
	"$(WIX)\bin\candle.exe" eidlink.wxs -dVERSION=$(VERSION)
	"$(WIX)\bin\light.exe" -out $(INSTALL_PRODUCT) eidlink.wixobj -v -ext WixUIExtension -dWixUILicenseRtf=install/LICENSE.rtf -dWixUIDialogBmp=install/install_dlg.bmp
	$(SIGN) $(INSTALL_PRODUCT)
   copy $(INSTALL_PRODUCT) eidlink.msi
   
pkg-unsigned: binaries
	"$(WIX)\bin\candle.exe" eidlink.wxs -dVERSION=$(VERSION)
	"$(WIX)\bin\light.exe" -out $(INSTALL_PRODUCT) eidlink.wixobj -ext WixUIExtension -dWixUILicenseRtf=install/LICENSE.rtf -dWixUIDialogBmp=install/install_dlg.bmp
	
clean:
   del /Q $(BINARIES)
   del /Q $(INSTALL_PRODUCT)

test: binaries
	python ../test/test.py
