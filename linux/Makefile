PLDFLAGS = $(shell pkg-config --libs libpcsclite)
PCFLAGS = $(shell pkg-config --cflags libpcsclite)
beidconnect: asn1.o BEIDCard.o CardFactory.o CardReader.o CertChainRequestHandler.o comm.o hash.o InfoRequestHandler.o log.o main.o native.o ReaderList.o Request.o RequestHandler.o SCard.o SignRequestHandler.o UserCertsRequestHandler.o util.o VersionRequestHandler.o VirtualCard.o VirtualReader.o x509Util.o crypto.o setup.o test.o IDRequestHandler.o
	$(CXX) $^ $(PLDFLAGS) $(LDFLAGS) -o $@

%.o: ../common/%.cpp
	$(CXX) -c -O2 -std=gnu++11 $(CXXFLAGS) $(PCFLAGS) -fPIE -I../common -o $@ $<
%.o: ../common/%.c
	$(CC) -c -O2 -std=gnu99 $(CFLAGS) $(PCFLAGS) -fPIE -I../common -o $@ $<

install:
	install -d $(DESTDIR)/usr/bin
	install -m 755 beidconnect $(DESTDIR)/usr/bin/
	mkdir -p /etc/chromium/native-messaging-hosts
	mkdir -p /etc/opt/chrome/native-messaging-hosts
	mkdir -p /etc/opt/edge/native-messaging-hosts
	mkdir -p /usr/lib/mozilla/native-messaging-hosts
	mkdir -p /usr/lib64/mozilla/native-messaging-hosts
	$(DESTDIR)/usr/bin/beidconnect -setup $(DESTDIR)/usr/bin/ /etc/chromium/native-messaging-hosts/ /usr/lib/mozilla/native-messaging-hosts/
	cp --update=none /etc/chromium/native-messaging-hosts/be.bosa.beidconnect.json /etc/opt/chrome/native-messaging-hosts/
	cp --update=none /etc/chromium/native-messaging-hosts/be.bosa.beidconnect.json /etc/opt/edge/native-messaging-hosts/
	cp --update=none /usr/lib/mozilla/native-messaging-hosts/be.bosa.beidconnect.json /usr/lib64/mozilla/native-messaging-hosts/

clean:
	rm -f *.o beidconnect

#check: beidconnect
#	cd ..; python3 test/test.py
