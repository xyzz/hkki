
objects = main.o action.o stcm2l_file.o text_entity.o
CXX = g++
CFLAGS = `pkg-config --cflags gtk+-2.0 gmodule-2.0 glib-2.0` -Wall 
LDFLAGS = `pkg-config --libs gtk+-2.0 gmodule-2.0 glib-2.0`

.PHONY: all clean

all: hkki

hkki: $(objects)
	$(CXX) -o hkki $(objects)  $(LDFLAGS)

$(objects): %.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@
	
clean:
	rm *.o
