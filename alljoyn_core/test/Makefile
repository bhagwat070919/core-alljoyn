# Copyright AllSeen Alliance. All rights reserved.
#
#    Permission to use, copy, modify, and/or distribute this software for any
#    purpose with or without fee is hereby granted, provided that the above
#    copyright notice and this permission notice appear in all copies.
#
#    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
#    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
#    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
#    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
#    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
#    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
#    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
# 

.PHONY: all clean

all: progs

LIBS += -lalljoyn
PROG_BINS =  \
        bignum \
        socktest \
        autochat \
        remarshal \
        unpack \
        srp \
        aes_ccm \
        keystore \
        bbservice \
        bbsig \
        bbclient \
        bbjoin \
        bbjitter \
        marshal \
        names \
        compression \
        rawclient \
        rawservice \
        sessions

# Test Programs
progs : $(PROG_BINS)

%:%.cc
	$(CC) $(CXXFLAGS) $(CPPDEFINES) $(INCLUDE) $(LINKFLAGS) -o $@ $< $(LIBS)
	cp $@ $(INSTALLDIR)/dist/bin

clean:
	@rm -f $(PROG_BINS) *~


