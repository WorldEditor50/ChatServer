CC=gcc
TARGET=client
SRC= main.c client.c
CFLAG= -g -Wall
LIBS += ../log/log.c
LIBS += ../message/message.c
LIBS += -lpthread
all:
	$(CC) -o $(TARGET) $(SRC) $(LIBS) $(CFLAG)
