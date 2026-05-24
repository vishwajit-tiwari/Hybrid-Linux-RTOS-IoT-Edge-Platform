CC=gcc

CFLAGS=-Wall

TARGET=telemetry_service

all:
	$(CC) $(CFLAGS) telemetry_service.c -o $(TARGET) -lpaho-mqtt3cs

clean:
	rm -f $(TARGET)

