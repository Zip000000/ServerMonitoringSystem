CC := gcc
SHDIR := ./0.Headfile
CFLAGS := -pthread
MOBJS := x.Master.o $(SHDIR)/Sock.o $(SHDIR)/Common.o $(SHDIR)/ClntList.o $(SHDIR)/Epoll.o
COBJS := z.Client.o $(SHDIR)/Sock.o $(SHDIR)/Common.o $(SHDIR)/ClntList.o $(SHDIR)/Epoll.o
# test

all: Zip.pihealthd Zip.pihealthd.Master
	@echo "OK!"

Zip.pihealthd.Master: $(MOBJS)
	@$(CC) -o $@ $(CFLAGS) $(MOBJS)

$(MOBJS) : %.o : %.c
	@$(CC) -c $(CFLAGS) $< -o $@

Zip.pihealthd: $(COBJS)
	@$(CC) -o $@ $(CFLAGS) $(COBJS)

$(COBJS) : %.o : %.c
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean 

clean:
	@rm -f ./0.Headfile/*.o
	@rm -f ./*.o
	@rm -f 
	@rm -f config.sample
	@echo Done cleaning

install: Zip.pihealthd
	@cp -r ./1.ShellStuff/ /tmp/
	@test -f config.sample || cp config config.sample
	@cp ./Zip.pihealthd /usr/local/bin
	@cp ./config /etc/Zip.pihealth.config

Minstall: Zip.pihealthd.Master
	@test -f config.sample || cp config config.sample
	@cp ./Zip.pihealthd.Master /usr/local/bin
	@cp ./config /etc/Zip.pihealth.config

