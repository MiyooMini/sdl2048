
TARGET = sdl2048

CC = arm-linux-gnueabihf-gcc
CXX = arm-linux-gnueabihf-g++
STRIP = arm-linux-gnueabihf-strip

SOURCES = . 
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.cpp))
OFILES = $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)

CFLAGS = -I$(shell pwd)/../dependency/release/include/cust_inc/ -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -O3 -march=armv7ve -ffast-math -fomit-frame-pointer -fno-strength-reduce
CXXFLAGS = $(CFLAGS)
LDFLAGS = -L$(shell pwd)/../dependency/release/nvr/i2m/common/glibc/8.2.1/cust_libs/dynamic/ -lSDL -lSDL_ttf -lSDL_image -lSDL_mixer
LDFLAGS += -L$(shell pwd)/../dependency/release/nvr/i2m/common/glibc/8.2.1/mi_libs/dynamic/ -lmi_common -lmi_sys -lmi_disp -lmi_panel -lmi_gfx -lmi_divp -lmi_ao -lmad -lfreetype

$(TARGET): $(OFILES)
	$(CXX) $(OFILES) -o $@ $(LDFLAGS)
	#$(STRIP) $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OFILES)


install:
	echo do nothing for install
