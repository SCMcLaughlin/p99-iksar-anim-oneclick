
CFLAGS= 
COPT= -O2 -fomit-frame-pointer -ffast-math -std=gnu89
CWARN= -Wall -Wextra -Wredundant-decls
CWARNIGNORE= -Wno-unused-result -Wno-strict-aliasing
CINCLUDE= -Isrc/
CDEF=

ifdef debug
CFLAGS+= -O0 -g -Wno-format -fno-omit-frame-pointer
CDEF+= -DDEBUG
else
CFLAGS+= -DNDEBUG
endif

_OBJECTS=               \
 bit                    \
 crc                    \
 hash                   \
 main                   \
 pfs                    \
 util_array             \
 util_buffer            \
 util_hash_tbl          \
 wld                    \
 virtual_wld

OBJECTS= $(patsubst %,build/%.o,$(_OBJECTS))

##############################################################################
# Core Linker flags
##############################################################################
LFLAGS= 
LDYNAMIC= -lz
LSTATIC= 

##############################################################################
# Util
##############################################################################
Q= @
E= @echo -e
RM= rm -f 

##############################################################################
# Build rules
##############################################################################
.PHONY: default all clean

default all: p99-iksar-anim-oneclick

p99-iksar-anim-oneclick: bin/p99-iksar-anim-oneclick

bin/p99-iksar-anim-oneclick: $(OBJECTS)
	$(E) "Linking $@"
	$(Q)$(CC) -o $@ $^ $(LSTATIC) $(LDYNAMIC) $(LFLAGS)

build/%.o: src/%.c $($(CC) -M src/%.c)
	$(E) "\e[0;32mCC     $@\e(B\e[m"
	$(Q)$(CC) -c -o $@ $< $(CDEF) $(COPT) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE)

clean:
	$(Q)$(RM) build/*.o
	$(E) "Cleaned build directory"
