CC=gcc
TARGET=server
CFLAG= -g -Wall
LIBS += ../container/list/list.c
LIBS += ../message/message.c
LIBS += ../log/log.c
LIBS += ../user/user.c
LIBS += ../threadpool/threadpool.c
LIBS += -lpthread
all:
	$(CC) -o $(TARGET) main.c server.c $(LIBS) $(CFLAG)
