
c_files := $(wildcard *.c)
exes := $(patsubst %.c,%,$(c_files))
plot := $(patsubst %.c,%.plot,$(c_files))
asms := $(patsubst %.c,%.s,$(c_files))

opts := 
opts += -march=native 
opts += -O3
opts += -D CPUSPEED_MHZ=2900
#opts += -fopt-info-vec-optimized 
#opts += -fopt-info-vec-missed
#opts += -std=gnu99
#opts += -fopenmp
#opts += -fcilkplus
#opts += -funroll-loops
#opts += --param max-unroll-times=2
#opts += -Wall
#opts += -Wextra

#CC := ~/parallel2/sys/inst/g/gcc/bin/gcc
CC := gcc-8
CFLAGS := $(opts)
all : $(exes) $(asms)

bin : $(exes)

plot : $(plot)

asm : $(asms)

$(exes) : % : %.c Makefile
	$(CC) -o $@ $(CFLAGS) $<
$(asms) : %.s : %.c Makefile
	$(CC) -S $(CFLAGS) $<
$(plot) : %.plot : %.c Makefile
	$(CC) -o $@ $(CFLAGS) -D PLOT $<

clean :
	rm -f $(asms) $(exes) $(plot)