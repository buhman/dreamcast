rm -rf ftdi_transfer_source
mkdir -p ftdi_transfer_source

cp -rL \
   align.hpp \
   crc32.c \
   crc32.h \
   ftdi_maple.cpp \
   ftdi_maple.hpp \
   ftdi_transfer.1 \
   ftdi_transfer.cpp \
   ftdi_transfer.hpp \
   ftdi_transfer.sh \
   Makefile \
   maple \
   serial_protocol.hpp \
   ftdi_transfer_source/

#tar cvzf ftdi_transfer_source.tar.gz ftdi_transfer_source
