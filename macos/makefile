VERSION = 1.0
IDENTIFIER = com.bosa.eidlink
PROGRAM = eidlink
BUILDDIR = ./build
BINARIES = /tmp/eIDLink.dst



.PHONY : all
all: $(BINARIES)


$(BINARIES):
	xcodebuild install -project eIDLink.xcodeproj

.PHONY : clean
clean:
	-rm -rf $(BINARIES)
	-rm -rf $(BUILDDIR)

test: all
	python ../test/test.py
