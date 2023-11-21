#!/bin/bash

get_arch=`arch`
if [[ $get_arch =~ "x86_64" ]];then
    echo "this is x86_64"
    g++ -o libberxel.so core/core.cpp core/BerxelCommonFunc.cpp core/BerxelImageRender.cpp  -shared -fPIC -I. -I include  -I core -Llibs -lBerxelHawk -lglut -lGL 
    mv libberxel.so libs/
elif [[ $get_arch =~ "aarch64" ]];then
    echo "this is arm64"
    g++ -o libberxel.so core/core.cpp core/BerxelCommonFunc.cpp core/BerxelImageRender.cpp  -shared -fPIC -I. -I include  -I core -Llibs_arm64 -lBerxelHawk -lglut -lGL 
    mv libberxel.so libs_arm64/
elif [[ $get_arch =~ "mips64" ]];then
    echo "this is mips64"
else
    echo "unknown!!"
fi

