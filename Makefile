all: cdow1

# Set the environment before to point path to nc-config
# export PATH=$PATH:/home/kunkel/ur-git/esiwace/ESD-Middleware/install/bin/
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/kunkel/ur-git/esiwace/ESD-Middleware/install/lib/

clean:
	rm cdow1

cdow1: cdow1.cpp
	g++ -g3 -std=c++17 cdow1.cpp -o cdow1 -O3 $(shell nc-config --cflags --libs)

run: cdow1
	./cdow1 /home/kunkel/ur-git/esiwace/io-training/lab-files/snow/snowcover.mon.mean.nc
