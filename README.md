# BTHSHub
A server for Brooklyn Tech's app hub!

# Why make this?
- Lack of information on PSAL tryouts and a central place to find clubs, interests and sports for Freshmen
<img width="1025" height="598" alt="image" src="https://github.com/user-attachments/assets/f3281740-1bda-45d2-ba66-421e99fef934" />
- Lack of guides about what is and how to join the Math Team class, science olympiad, FRC 334
- Confusing usage of Microsoft Teams for robotics but SportsYou for PSAL teams (which both have problems in themselves)

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
7. Run ```openssl rand -hex 32``` in order to get a private key. Then, put in a .env file like so:

```
JWT_SECRET=xxx...
```
8. Follow steps below in "Development"
# Development
1. Ensure you have Cmake extension in Vscode or run cmake in the CLI
2. Run the redis server
```
redis-server &
```
3. Use the run button at the bottom in vscode or use the Cmake CLI
# Production notes
Make sure to rotate JWT_SECRET every x days. Maybe write a cron job?
