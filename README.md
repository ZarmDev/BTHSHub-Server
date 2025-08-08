# BTHSHub
A server for Brooklyn Tech's app hub!

# Installation (Arch Linux)
1. Install redis
sudo pacman -S
2. Install hiredis
```
pacman -S hiredis
```
OR
```
git clone https://github.com/redis/hiredis.git
cd hiredis
make
sudo make install
```
3. Install redis-plus-plus
```
git clone https://github.com/sewenew/redis-plus-plus.git
cd redis-plus-plus
mkdir build && cd build
cmake ..
make
sudo make install
```
4. Install jwt-cpp
git clone https://github.com/Thalhammer/jwt-cpp.git
cd jwt-cpp
cmake .
cmake --build .
cmake --install .
5. Update linker cache
```
sudo ldconfig
```
6. Also remember to add vcpkg to the toolchain: cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkglocation]/scripts/buildsystems/vcpkg.cmake .

You only need to run this once, or whenever:
- You change your dependencies
- You change your toolchain
- You delete your build/ folder
7. Follow steps below in "Development"
# Development
1. Ensure you have Cmake extension in Vscode or run cmake in the CLI
2. Run the redis server
```
redis-server &
```
3. Run ```openssl rand -hex 32``` in order to get a private key. Then, put in a .env file like so:

```
JWT_SECRET=xxx...
```
4. Run the cmake extension or use the CLI
# Production notes
Make sure to rotate JWT_SECRET every x days. Maybe write a cron job?