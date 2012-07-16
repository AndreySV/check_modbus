#!/bin/bash

#aclocal
#autoconf
#touch README AUTHORS NEWS ChangeLog
# autoheader
#automake -a
#./configure
./configure LDFLAGS=-L/usr/local/lib CPPFLAGS=-I/usr/local/include
# make
# make dist
