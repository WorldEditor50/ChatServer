CC=gcc
TARGET=client
CFLAG= -g -Wall
LIBS += ../log/log.c
LIBS += ../message/message.c
LIBS += -lpthread
all:
	$(CC) -o $(TARGET) main.c client.c $(LIBS) $(CFLAG)
