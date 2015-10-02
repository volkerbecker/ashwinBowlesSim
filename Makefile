HOST=$(shell hostname)
#CFLAGS FOR MEITNER AND PRIVATE PC
#CLFLAGS = -DCL_USE_DEPRECATED_OPENCL_2_0_APIS -D__CL_ENABLE_EXCEPTIONS -DCL_ENABLE_EXCEPTIONS -std=c++11 -O3 -g
#CFLAGS FOR FER

CLFLAGS = -DCL_USE_DEPRECATED_OPENCL_2_0_APIS -D__CL_ENABLE_EXCEPTIONS -DCL_ENABLE_EXCEPTIONS -std=c++11
#optimization
CLFLAGS += -O3

ifneq (${HOST},fermi)
INCLUDE =-I${HOME}/workspace/openGLvisualizer 
endif
ifeq (${HOST},becker-amazing-pc)
INCLUDE += -I${AMDAPPSDKROOT}/include/
endif

OBJS =	ashwinBowles.o ParticleSystem.o Walls.o	AshwinBowlesSystem.o Setup.o


LIBS =	-lOpenCL
ifeq (${HOST},becker-amazing-pc) 
LIBS += -L${AMDAPPSDKROOT}/lib/x86_64 
endif
ifneq (${HOST},fermi)
LIBS += -L${HOME}/workspace/openGLvisualizer -lminglvisualizer -lsfml-system -lsfml-window -lGLEW -lSOIL -lGL -lsfml-graphics -lX11 -pthread
endif 

TARGET = ashwinBowles

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
	
%.o:	%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(CLFLAGS) -c $*.cpp -o $*.o
	$(CXX) -MM $(CXXFLAGS) $(INCLUDE) $(CLFLAGS) $*.cpp > $*.d

-include $(OBJS:.o=.d)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) *.d

