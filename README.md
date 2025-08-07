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
4. Update linker cache
```
sudo ldconfig
```
5. Follow steps below in "Development"
# Development
1. Ensure you have Cmake extension in Vscode or run cmake in the CLI
2. Run the redis server
```
redis-server &
```
3. Run the cmake extension or use the CLI