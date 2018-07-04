mpi := stencil_mpi.c
orig := stencil_serial.c

opts :=
opts += -march=native
opts += -O3
opts += -D N=64
opts += -D NT=1000
opts += -D NTICKS=3

CC := mpiicc
CFLAGS := $(opts)
all : mpi orig
run : src go

mpi :
	$(CC) $(filter %.c,$(mpi)) -o stencil_mpi $(CFLAGS)

orig :
	gcc $(filter %.c,$(orig)) -o stencil_serial $(CFLAGS)

clean :
	rm -f stencil_mpi stencil_serial MPI_PingPong.*

rm :
	rm -f MPI_PingPong.*

go :
	qsub test.sh