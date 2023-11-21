g++ -o libberxel.so core.cpp Common/BerxelCommonFunc.cpp Common/BerxelImageRender.cpp  -shared -fPIC -I. -I Include  -I Common -Llibs -lBerxelHawk -lglut -lGL 
