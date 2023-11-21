#export LD_LIBRARY_PATH=:/home/jim/workspace/db01/berxel/V2.0.88/Linux/x86/BerxelSDK-Linux-2.0.88/libs/:~/workspace/db01/berxel/sdk/libs 
#!/bin/bash

get_arch=`arch`
CUR_DIR=$PWD
echo $CUR_DIR
if [[ $get_arch =~ "x86_64" ]];then
    echo "this is x86_64"
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CUR_DIR/libs
elif [[ $get_arch =~ "aarch64" ]];then
    echo "this is arm64"
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CUR_DIR/libs_arm64
elif [[ $get_arch =~ "mips64" ]];then
    echo "this is mips64"
else
    echo "unknown!!"
fi
