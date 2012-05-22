CC=g++
OUTPUT=ocv
CCARGS=-I /opt/local/include/ -L /opt/local/lib -o $(OUTPUT) -l opencv_core -l opencv_highgui -l opencv_imgproc -l opencv_features2d
DYLD_LIBRARY_PATH=$(DYLD_LIBRARY_PATH):/opt/local/lib
LD_LIBRARY_PATH=/opt/local/lib

all: clean
	$(CC) $(CCARGS) main.cpp

clean:
	rm -f ocv
