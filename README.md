# BTHSHub
A server for Brooklyn Tech's app hub!

Copyright (c) 2025 ZarmDev. All rights reserved.

# Why make this?
- Lack of information on PSAL tryouts and a central place to find clubs, interests and sports for Freshmen

https://www.reddit.com/r/BTHS/comments/1mm0aef/when_are_girls_varsity_tennis_tryouts/

https://www.reddit.com/r/BTHS/comments/1m6w5f9/kids_on_the_math_team_do_you_wanna_slide_over/

https://www.reddit.com/r/BTHS/comments/1jia3lr/what_are_scioly_tryouts_like_at_btech/

- Lack of guides about what is and how to join teams or classes like the Math Team class, Weston Scholars, science olympiad, FRC 334
- Confusing usage of Microsoft Teams (which is a really bad app) for robotics but SportsYou for PSAL teams (which both have problems in themselves)
- I personally missed most of my tryouts in Freshmen year because I didn't have the SportsYou code and recently I went to a track tryout and there wasn't a single freshmen for a school of 6000 kids
- Lack of club credit system

# Installation
## Linux
### Install vcpkg
> All linux distributions
```
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh --disableMetrics
nano ~./bashrc
```
add this to ~./bashrc
```
export VCPKG_ROOT=[your-vcpkg-root]
```
then run
```
source ~/.bashrc
```
> IF THIS DOES NOT WORK THEN EDIT CMakePresets.json and add your root rather than it having ```$env{VCPKG_ROOT}```
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
> Arch
```
sudo pacman -S redis
```
> All linux distributions
You may have to enable it as system service (personally, I didn't have to)
```
sudo systemctl enable redis-server
sudo systemctl start redis-server
```
### Install redis-plus-plus seperately
```
git clone https://github.com/sewenew/redis-plus-plus.git
cd redis-plus-plus
mkdir build && cd build
cmake ..
make
sudo make install
```
### Install libsodium
> Debian based
```
sudo apt install -y libsodium-dev
```
> Arch
```
sudo pacman -S libsodium
```
### Follow steps in "First time" and then "Development"

# First time
1. Ensure you have Cmake extension in Vscode or run cmake in the CLI
2. Run ```openssl rand -hex 32``` in order to get a private key. Set it to JWT_SECRET in .env
3. Optionally add an admin password if you are creating an admin account
```
JWT_SECRET=xxx
ADMINPASS=xxx
```
4. Add the path to your vcpkg root to CMakePresets.json at "CMAKE_TOOLCHAIN_FILE". There is already a value there so just replace it.
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
# Deploy to Heroku
```
heroku create bthshub-server
heroku config:set JWT_SECRET=$(openssl rand -hex 32)
heroku config:set ADMINPASS=your_secure_admin_password

```
# TODO
For more ideas, see ideas.md
## Server: (base goals)
✅ The website will be made in C++ to be extremely fast and reliable
✅ User authentication with JWT Tokens
✅ Add team creation
✅ Private or public teams
✅ Add yourself or others to teams (if you own the team or are admin)
✅ Add ability to post annoucements
✅ Add way to get all annoucements
✅ Add admins and moderator permissions/roles
✅ Admin panel
✅ Add error handling, meaning the server will always run unless a severe error occurs
❌ Add rate limiting

## Future: (AFTER APP IS MADE!)
❌ Implement club credit system and club directory
❌ Store BTHS daily announcements and send in a nice format to phones
❌ Add rate limiting
❌ Add friend functionality
❌ Storing schedules and being able to get other user's schedule (if they are friends)
❌ Ability to delete your own account
❌ Delete and edit annoucements
❌ More team actions like deletion or removing users from teams

## Common errors
### Illegal instruction        (core dumped)
Likely means you forgot to return a string in your route

Example:
```
string updateOtherUserAdminLevel(HttpRequest &req) {
    grantAdminLevel(parsed[0], parsed[1]);
    // Should have returned sendString or something like:
    // return sendString("200 OK", "Changed permission successfully!");
}
server.post("/api/test", updateOtherUserAdminLevel);
```

## Issues I am aware of 
⚠️In userdatabase, teamdatabase, you can easily screw up some parts of the server by inputting backslashes. I will look into a efficient way to parse data from req.data without compromising for security
# Production notes
## Optimizations
### Shorten routes
For security and performance, you can write a script that replaces all the routes in server.cpp to be replaced with one letter.
## Security
Make sure to log everything! Remove the system("clear") line
### Rotation
Try to rotate JWT_SECRET every x days using a cronjob.