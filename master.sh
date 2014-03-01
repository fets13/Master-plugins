there=$PWD
cd bin
export LD_LIBRARY_PATH=.
./ydlemaster -c ../ydle.conf
cd $there
