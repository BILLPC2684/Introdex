mkdir -p bin && cd bin
if [ ! -f /usr/local/include/uv.h ]; then
 git clone https://github.com/libuv/libuv
 cd libuv
 sh autogen.sh
 ./configure
 make j=16
 make check
 sudo make install
 rm -rf libuv
 cd ..
fi
if [ ! -f /usr/local/lib/libraylib.a ]; then
 sudo apt install build-essential git libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
 git clone https://github.com/raysan5/raylib
 git clone --depth 1 https://github.com/raysan5/raylib.git raylib
 cd raylib/src/
 make PLATFORM=PLATFORM_DESKTOP j=16 # To make the static version.
 sudo make install
 cd ../..
 rm -rf raylib
fi
if [ ! -f /usr/include/cjson/cJSON.h ]; then
 git clone https://github.com/DaveGamble/cJSON
 mkdir cJSON/build
 cd cJSON/build
 cmake .. -DENABLE_CJSON_UTILS=On -DENABLE_CJSON_TEST=Off -DCMAKE_INSTALL_PREFIX=/usr
 make
 sudo make DESTDIR=$pkgdir install
 cd ../..
 rm -rf cJSON
fi
cd ..
