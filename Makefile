CXXFLAGS = $(shell root-config --cflags) -fPIC
ROOTLIBS = $(shell root-config --libs)
OBJ = Analysis.o Dict.o
HDR = Analysis.h
SRC = Analysis.cc  
Event.so : $(OBJ) 
	g++ $(ROOTLIBS) -shared -o libAnalysis.so $(OBJ) 
Event.o : $(SRC)
	g++ $(CXXFLAGS) -c $(SRC)
Dict.cc : $(HDR)
	 rootcint -f Dict.cc -c $(HDR) LinkDef.h

clean :
	rm *.o *.so Dict.cc *.pcm
