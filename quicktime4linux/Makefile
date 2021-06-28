include global_config

all:
	make -C src

util:
	make -C src util

dist:
	make -C src clean && \
	cd .. && \
	tar ch $(INSTDIR) | gzip -9 > $(TARBALL)

clean:
	make -C src clean;
