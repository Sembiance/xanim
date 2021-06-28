############### BCBASE

include ../global_config

OBJS = \
	atom.o \
	codecs.o \
	ctab.o \
	dinf.o \
	dref.o \
	edts.o \
	elst.o \
	hdlr.o \
	ima4.o \
	jdatasrc.o \
	jpeg.o \
	matrix.o \
	mdat.o \
	mdhd.o \
	mdia.o \
	minf.o \
	moov.o \
	mvhd.o \
	quicktime.o \
	raw.o \
	smhd.o \
	stbl.o \
	stco.o \
	stsc.o \
	stsd.o \
	stsdtable.o \
	stss.o \
	stsz.o \
	stts.o \
	tkhd.o \
	trak.o \
	udta.o \
	ulaw.o \
	util.o \
	vmhd.o \
	wmx1.o \
	yuv2.o \
	yuv4.o

OUTPUT = quicktime.a

CC = gcc



$(OUTPUT): $(OBJS)
	ar rc $(OUTPUT) $(OBJS)
	ranlib $(OUTPUT)

dump: dump.o quicktime.a
	$(CC) -o dump dump.o quicktime.a -ljpeg

make_streamable: make_streamable.o quicktime.a
	$(CC) -o make_streamable make_streamable.o quicktime.a -ljpeg

util:  dump make_streamable
	

quicktime.o: quicktime.h  quicktime.c

.c.o:
	$(CC) -c $(CFLAGS) $*.c

clean:
	rm -f core
	rm -f *.o
	rm -f $(OUTPUT)
	rm -f dump make_streamable a.out

wc:
	cat *.c *.h | wc

install:
