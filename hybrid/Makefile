# Stib, hybrid: the game's C compiled natively from ../source (a
# reconstruction of the RISC OS source), the rest of !RunImage - its
# assembler, DeskLib and the C start-up - translated (gen/, from
# ../tools/images.py: make regen), linked with libroger (the RISC OS
# runtime). See game.c.
ROGER   ?= $(HOME)/stuff/libroger
SS      ?= $(HOME)/stuff/superspider
CC      ?= gcc
CFLAGS  ?= -O2 -g
SRC     := ../source
LIBS    := $(shell pkg-config --libs sdl3) -ljpeg -lm -lpthread

GAME_C  := play between sat levels global userint
NATIVE  := $(addprefix build/obj/,$(GAME_C:=.o))
GEN     := $(wildcard gen/*/code*.c) $(wildcard gen/*/entries.c)
OBJ     := $(GEN:.c=.o) build/native.o hyrt.o game.o

# the DeskLib headers the source names ("DeskLib:wimp.h"), all inc/desklib.h
DESKLIB_H := core error event eventmsg file handler icon kbd menu resource screen \
             sprite swi template wimp wimpswis window

# the game's C as Acorn C compiled it: char unsigned, float arithmetic in
# single precision (no contraction); hybrid.h first
GAMEFLAGS := $(CFLAGS) -std=gnu11 -funsigned-char -ffp-contract=off -fno-strict-aliasing \
             -Ibuild/inc -Iinc -include hybrid.h -w

stib: $(OBJ) $(ROGER)/libroger.a
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(ROGER)/libroger.a $(LIBS)

$(ROGER)/libroger.a: FORCE
	$(MAKE) -C $(ROGER) libroger.a

gen/%.o: gen/%.c $(ROGER)/include/roger_cpu.h
	$(CC) $(CFLAGS) -w -fno-strict-aliasing -I$(ROGER)/include -c -o $@ $<

hyrt.o: hyrt.c hyrt.h inc/kernel.h $(ROGER)/include/roger.h
	$(CC) $(CFLAGS) -Wall -fno-strict-aliasing -I$(ROGER)/include -c -o $@ $<

game.o: game.c hyrt.h $(ROGER)/include/roger.h
	$(CC) $(CFLAGS) -Wall -I$(ROGER)/include -c -o $@ $<

# the game's C as one object exporting only hy_native, hy_fnaddr and
# hy_finish, so
# that its names (game, lev, drop...) and libroger's don't meet
build/native.o: $(NATIVE) build/obj/shims.o
	$(LD) -r -o $@ $^
	objcopy -G hy_native -G hy_fnaddr -G hy_finish $@

build/obj/shims.o: shims.c hyrt.h build/.src hybrid.h inc/desklib.h
	@mkdir -p build/obj
	$(CC) $(GAMEFLAGS) -c -o $@ $<

# userint.c's main is called by the C start-up (shims.c)
build/obj/userint.o: GAMEFLAGS += -Dmain=stib_main

build/obj/%.o: build/.src hybrid.h inc/kernel.h inc/swis.h inc/desklib.h
	@mkdir -p build/obj
	$(CC) $(GAMEFLAGS) -c -o $@ build/src/$*.c

# the sources with stib.patch applied; the headers by name
build/.src: stib.patch $(wildcard $(SRC)/*.c $(SRC)/*.h)
	rm -rf build/src build/inc
	mkdir -p build/src build/inc
	cp $(SRC)/*.c $(SRC)/*.h build/src/
	patch -s -d build/src -p1 < stib.patch
	for f in build/src/*.h; do ln -sf ../src/$$(basename $$f) build/inc/; done
	for h in $(DESKLIB_H); do ln -sf ../../inc/desklib.h "build/inc/DeskLib:$$h.h"; done
	touch $@

# make full: the whole of !RunImage translated (build/fullgen), as a full
# recompilation, for make compare
FULLGEN := $(wildcard build/fullgen/code*.c) build/fullgen/entries.c
full: stib-full
stib-full: build/fullgen/.done game.c $(ROGER)/libroger.a
	$(MAKE) $(patsubst %.c,%.o,$(wildcard build/fullgen/code*.c) build/fullgen/entries.c)
	$(CC) $(CFLAGS) -Wall -DFULL -I$(ROGER)/include -c -o build/game-full.o game.c
	$(CC) $(CFLAGS) -o $@ build/fullgen/*.o $(filter gen/flexisound/%,$(GEN:.c=.o)) \
	    build/game-full.o $(ROGER)/libroger.a $(LIBS)
build/fullgen/.done: ../analysis/runimage.json
	@mkdir -p build/fullgen
	$(SS)/_build/default/bin/main.exe gen "../stib/!Stib/!RunImage" 8000 ../analysis/runimage.json \
	    --c build/fullgen --prefix Full --tabprefix full --files 4 --no-smc
	touch $@
build/fullgen/%.o: build/fullgen/%.c $(ROGER)/include/roger_cpu.h
	$(CC) $(CFLAGS) -w -fno-strict-aliasing -I$(ROGER)/include -c -o $@ $<

# make test: a headless game (Play, three shots, Escape, a hiscore, back to
# the title, Desktop; then Quit from the icon's menu), seeded; make compare:
# the same with the full recompilation, a screenshot every half second
# (../tools/compare.py). SCRIPT, LENGTH (ms) and SEED may be given:
#   make compare SCRIPT=... LENGTH=...
SCRIPT ?= 9000:click:160:284,9200:release,14000:click:400:512,14100:release,17000:click:900:512,17100:release,20000:click:640:512,20100:release,26000:key:113:300,35500:type:106,35700:type:117,36100:type:13,40000:click:160:124,40200:release
LENGTH ?= 41000
SEED ?= 1996
RUN = --seed=$(SEED) --frame-lock --headless-mips=50 --limit=$(shell echo $$(( ($(LENGTH) + 4000) * 50000 ))) \
      --script=$(shell (echo $(SCRIPT) | tr , '\n'; seq 500 500 $(LENGTH) | sed 's/$$/:shot/') | \
      sort -s -n -t: -k1,1 | paste -sd,)
test: stib
	rm -rf build/test && mkdir -p build/test/run && ln -s ../../../stib build/test/stib
	cd build/test/run && ../../../stib $(RUN) | tail -1
compare: stib stib-full
	rm -rf build/cmp && mkdir -p build/cmp/hybrid build/cmp/full && ln -s ../../../stib build/cmp/stib
	cd build/cmp/hybrid && ../../../stib $(RUN) | tail -1
	cd build/cmp/full && ../../../stib-full $(RUN) | tail -1
	python3 ../tools/compare.py build/cmp/hybrid build/cmp/full

# make regen: analyse and translate the images again
regen:
	SUPERSPIDER=$(SS) python3 ../tools/images.py analyse
	SUPERSPIDER=$(SS) python3 ../tools/images.py gen

clean:
	rm -rf build gen/*/*.o hyrt.o game.o stib stib-full

.PHONY: FORCE regen clean full test compare
