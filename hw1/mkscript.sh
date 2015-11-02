make clean > /dev/null;
rm -rf compile.time.txt run.time.txt

/usr/bin/time make flag=$1 2>> compile.time.txt;
# make clean > /dev/null;
# /usr/bin/time make flag=$1 2>> compile.time.txt 1> /dev/null;
# make clean > /dev/null;
# /usr/bin/time make flag=$1 2>> compile.time.txt 1> /dev/null;
# make clean > /dev/null;
# /usr/bin/time make flag=$1 2>> compile.time.txt 1> /dev/null;
# make clean > /dev/null;
# /usr/bin/time make flag=$1 2>> compile.time.txt 1> /dev/null;

/usr/bin/time ./vpr iir1.map4.latren.net k4-n10.xml place.out route.out -nodisp -place_only -seed 0 2>> run.time.txt 1> /dev/null;
/usr/bin/time ./vpr iir1.map4.latren.net k4-n10.xml place.out route.out -nodisp -place_only -seed 0 2>> run.time.txt 1> /dev/null;
/usr/bin/time ./vpr iir1.map4.latren.net k4-n10.xml place.out route.out -nodisp -place_only -seed 0 2>> run.time.txt 1> /dev/null;
/usr/bin/time ./vpr iir1.map4.latren.net k4-n10.xml place.out route.out -nodisp -place_only -seed 0 2>> run.time.txt 1> /dev/null;
/usr/bin/time ./vpr iir1.map4.latren.net k4-n10.xml place.out route.out -nodisp -place_only -seed 0 2>> run.time.txt 1> /dev/null;

# /usr/bin/time make -j $1 2>> compile.time.txt;
# make clean > /dev/null;
# /usr/bin/time make -j $1 2>> compile.time.txt 1> /dev/null;
# make clean > /dev/null;
# /usr/bin/time make -j $1 2>> compile.time.txt 1> /dev/null;
# make clean > /dev/null;
# /usr/bin/time make -j $1 2>> compile.time.txt 1> /dev/null;
# make clean > /dev/null;
# /usr/bin/time make -j $1 2>> compile.time.txt 1> /dev/null;

# ./vpr iir1.map4.latren.net k4-n10.xml place.out route.out -nodisp -place_only -seed 0;
