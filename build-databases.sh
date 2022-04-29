#!/bin/sh

cd src/main/cpp
mkdir -p build-databases
cd build-databases
cmake -DBUILD_ONSEM_DATABASE=ON ../
cmake --build .
./onsem/databasegenerator/onsemdatabasegenerator ../../assets ../onsem/share
cd ..
cp -r onsem/share/semantic/dynamicdictionary ../assets/linguistic/
