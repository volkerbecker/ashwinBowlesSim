CLFLAGS = -DCL_USE_DEPRECATED_OPENCL_2_0_APIS -D__CL_ENABLE_EXCEPTIONS -DCL_ENABLE_EXCEPTIONS -std=c++11 -O0 -g

INCLUDE =-I${HOME}/workspace/openGLvisualizer -I${AMDAPPSDKROOT}/include/

OBJS =	ashwinBowles.o ParticleSystem.o Walls.o	AshwinBowlesSystem.o Setup.o

#For amazing-sax
LIBS =	-lOpenCL -L${AMDAPPSDKROOT}/lib/x86_64 -L${HOME}/workspace/openGLvisualizer -lminglvisualizer -lsfml-system -lsfml-window -lGLEW -lSOIL -lGL -lsfml-graphics -lX11 -pthread
#For Meitner
#LIBS =	-lOpenCL -L${AMDAPPSDKROOT}/lib/x86_64 -L${HOME}/workspace/openGLvisualizer -lminglvisualizer -lsfml-system -lsfml-window -lGLEW -lSOIL -lGL -lsfml-graphics -lX11 -pthread

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

