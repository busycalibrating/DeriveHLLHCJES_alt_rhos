#!/bin/sh
exec=data_fit.exe
code=example_code/FitZjetDB.C

flags=$($ROOTSYS/bin/root-config --cflags --glibs)
includes="-I."

echo ; echo "Will compile $code"
g++ -o $exec $code $flags $includes && {
    echo "Compilation successful!"
    echo
    echo "Will now run..."
    ./$exec
    ./$exec poisson
} || {
    echo "Compilation failed :("
    echo
}
