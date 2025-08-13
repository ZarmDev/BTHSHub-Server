# BTHSHub
A server for Brooklyn Tech's app hub!

# Why make this?
- Lack of information on PSAL tryouts and a central place to find clubs, interests and sports for Freshmen

https://www.reddit.com/r/BTHS/comments/1mm0aef/when_are_girls_varsity_tennis_tryouts/

https://www.reddit.com/r/BTHS/comments/1m6w5f9/kids_on_the_math_team_do_you_wanna_slide_over/

https://www.reddit.com/r/BTHS/comments/1jia3lr/what_are_scioly_tryouts_like_at_btech/

- Lack of guides about what is and how to join the Math Team class, science olympiad, FRC 334
- Confusing usage of Microsoft Teams (which is a really bad app) for robotics but SportsYou for PSAL teams (which both have problems in themselves)
- I personally missed most of my tryouts in Freshmen year because I didn't have the SportsYou code... (which apparently is given by the coach?)

# Installation
## Linux
### Install vcpkg
```
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh --disableMetrics
export VCPKG_ROOT=/workspaces/BTHSHub-Server/vcpkg
```
### Install ninja
> Debian based
```
sudo apt update
sudo apt install -y ninja-build build-essential
```
### Install redis
> Debian based
```
sudo apt install redis-server
```
> All linux distributions
You may have to enable it as system service
```
sudo systemctl enable redis-server
sudo systemctl start redis-server
```
### Install libsodium
> Debian based
```
sudo apt install -y libsodium-dev
```
> Arch (already installed)
### Follow steps in "First time" and then "Development"

# First time (development)
1. Ensure you have Cmake extension in Vscode or run cmake in the CLI
2. Run ```openssl rand -hex 32``` in order to get a private key. Set it to JWT_SECRET in .env
3. Optionally add an admin password if you are creating an admin account
```
JWT_SECRET=xxx
ADMINPASS=xxx
```
# Development
1. Run the redis server
```
redis-server &
```
2. Use the run button at the bottom in vscode or use the Cmake CLI commands below:
```
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[put-your-vcpkg-root]/scripts/buildsystems/vcpkg.cmake
```
and then
```
cmake --build ./build
```
# Plan
1. Host on Heroku (for 24 months)
2. Use name.com or namecheap github student offer
3. It could also be hosted on AppWrite
# TODO
For more ideas, see ideas.md
## Server: (base goals)
✅ The website will be made in C++ to be extremely fast and reliable
✅ User authentication with JWT Tokens
❌ Add rate limiting
❌ Store BTHS daily announcements and send in a nice format to phones
❌ Add admins/moderators like teachers

## Future: (AFTER APP IS MADE!)
❌ Implement club credit system and club directory

# Production notes
## Optimizations
### Shorten routes
For security and performance, you can write a script that replaces all the routes in server.cpp to be replaced with one letter.
## Security
### Rotation
Try to rotate JWT_SECRET every x days using a cronjob.
