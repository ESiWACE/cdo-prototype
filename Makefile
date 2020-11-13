all: cdow1

# Set the environment before to point path to nc-config
# export PATH=/home/kunkel/ur-git/esiwace/ESD-Middleware/install/bin/:$PATH
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/kunkel/ur-git/esiwace/ESD-Middleware/install/lib/

clean:
	rm cdow1

cdow1: cdow1.cpp
	g++ -g3 -std=c++17 cdow1.cpp -o cdow1 -O3 $(shell nc-config --cflags --libs)

run: cdow1
	./cdow1 /home/kunkel/ur-git/esiwace/io-training/lab-files/snow/snowcover.mon.mean.nc result.nc

run.esdm: cdow1
	# How to use ESDM/NetCDF with this example:
	mkfs.esdm -g -l --create  --remove --ignore-errors
	nccopy /home/kunkel/ur-git/esiwace/io-training/lab-files/snow/snowcover.mon.mean.nc esdm://snowcover.nc
	./cdow1 esdm://snowcover.nc esdm://result.nc
