#!/bin/bash

cd util/Miracl
bash linux64
cd -
mkdir -p ../lib
cp util/Miracl/miracl.a ../lib/libmiracl.a
