## berxel sdk  
v2.0.122 library 

## compile  
'g++ main.cpp -I Include -L libs -lberxel -o color_depth' or 'make'

## how to use
check main.cpp for example

## depend (only need view debug)
sudo apt-get install freeglut3-dev

## notice
Execute 'bash install.sh' to install udev rules.   
Execute 'bash env.sh' to set environment library.
Maybe you should do by yourself like  
'CUR_DIR=$PWD; export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CUR_DIR/libs_arm64'  
in the shell.
