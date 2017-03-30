#!/bin/sh
exec=make_truth_response_plots.exe
code=example_code/DrawTruthResponse.C

flags=$($ROOTSYS/bin/root-config --cflags --glibs)
includes="-I."

echo ; echo "Will compile $code"
g++ -o $exec $code $flags $includes && {
    echo "Compilation successful!"
    echo
    echo "Will now run..."
    ./$exec
} || {
    echo "Compilation failed :("
    echo
}
