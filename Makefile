
CROSS_COMPILE=/datadisk/tools/gcc-arm-10.2-2020.11-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
CC 		= ${CROSS_COMPILE}gcc
TARGET	= capzu5
SRC		= capzu5.c
TARGET_V2	= capzu5_v2
SRC_V2		= capzu5_v2.c
TARGET_V3	= capzu5_v3
SRC_V3		= capzu5_v3.c
CFLAGS	= -Wall
LDFLAGS	= 

all: $(TARGET) $(TARGET_V2) $(TARGET_V3)

$(TARGET): $(SRC)
	$(CC) -o $(TARGET) $(CFLAGS) $(LDFLAGS) $<

$(TARGET_V2): $(SRC_V2)
	$(CC) -o $(TARGET_V2) $(CFLAGS) $(LDFLAGS) $<

$(TARGET_V3): $(SRC_V3)
	$(CC) -o $(TARGET_V3) $(CFLAGS) $(LDFLAGS) $<

.PHONY: clean
clean:
	@rm -rf $(TARGET) $(TARGET_V2) $(TARGET_V3) *.o
