PLDFLAGS = $(shell pkg-config --libs libpcsclite libcrypto)
PCFLAGS = $(shell pkg-config --cflags libpcsclite libcrypto)
eidlink: asn1.o BEIDCard.o Card.o CardFactory.o CardReader.o CertChainRequestHandler.o comm.o hash.o InfoRequestHandler.o log.o main.o native.o ReaderList.o Request.o RequestHandler.o SCard.o SignRequestHandler.o UserCertsRequestHandler.o util.o VersionRequestHandler.o VirtualCard.o VirtualReader.o x509Util.o crypto.o
	$(CXX) $(PLDFLAGS) $(LDFLAGS) -o $@ $^

%.o: ../common/%.cpp
	$(CXX) -c -O2 $(CXXFLAGS) $(PCFLAGS) -I../common -o $@ $<
%.o: ../common/%.c
	$(CC) -c -O2 $(CFLAGS) $(PCFLAGS) -I../common -o $@ $<

install:
	install -d $(DESTDIR)/usr/bin
	install -g root -o root -m 755 eidlink $(DESTDIR)/usr/bin/

clean:
	rm -f *.o eidlink

check: eidlink
	cd ..; python test/test.py
