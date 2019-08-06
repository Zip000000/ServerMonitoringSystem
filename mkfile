













#.PHONY: clean
clean:
    @rm -f *.o pihealthd
    @rm -f ../common/*.o
    @echo Done cleaning

install: pihealthdhealthd install.sh pihealthd.conf.sample
    @test -f /etc/pihealthd.conf || cp pihealthd.conf.sample /etc/pihealthd.conf.sample
    @bash install.sh
    cp -f pihealthd /usr/bin/@echo "PiHealthd isntalled"
