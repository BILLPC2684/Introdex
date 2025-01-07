read -p "Build [C]lient or [S]erver: " building
if [[ $building == [sS]	]]; then
 echo "Building Server..."
 if gcc src/server.c -o Introdex-Server -pg -I./src/include -lcjson -lm -lpthread -ldl -lrt -g -luv -Wformat-overflow=0 --debug; then
  echo -e "\n\e[32;1m[✓] \e[33mBuild for Introdex-Server was \e[32;1mSuccessful\e[33m!\e[m"
 else
  echo -e "\n\e[31;1m[X] \e[33mBuild for Introdex-Server was \e[31;1mUnsuccessful\e[33m...\e[m"
  echo "Please make sure the following libraries are installed:"
  echo "\\cJSON:                      https://github.com/DaveGamble/cJSON"
  echo "\\libuv:                      https://github.com/libuv/libuv"
 fi
else
 echo "Building Client..."
 if gcc src/client.c -o Introdex-Client -pg -I./src/include -lraylib -lcjson -lGL -lm -lpthread -ldl -lrt -lX11 -g -I./bin/raylib/src/ -Wformat-overflow=0 --debug; then
  echo -e "\n\e[32;1m[✓] \e[33mBuild for Introdex-Server was \e[32;1mSuccessful\e[33m!\e[m"
 else
  echo -e "\n\e[31;1m[X] \e[33mBuild for Introdex-Server was \e[31;1mUnsuccessful\e[33m...\e[m"
  echo "Please make sure the following libraries are installed:"
  echo "\\raylib:                     https://github.com/raysan5/raylib"
  echo "\\cJSON:                      https://github.com/DaveGamble/cJSON"
  echo "\\libuv:                      https://github.com/libuv/libuv"
 fi
fi
