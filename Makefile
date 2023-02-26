CC=g++
CFLAGS=-c -Wall -g
C_OBJS=Client_Main.o Client.o Packet.o
S_OBJS=Server_Main.o Server.o Packet.o

all:ClientApp ServerApp

ClientApp:$(C_OBJS)
	$(CC) $^ -o ClientApp

ServerApp:$(S_OBJS)
	$(CC) $^ -o ServerApp -lpthread

%.o:%.c
	$(CC) $^ $(CFLAGS)  -o $@

clean:
	rm *.o ClientApp ServerApp -rf