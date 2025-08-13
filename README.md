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

# TODO
## Client:
- On the first startup, show the freshman guide. There is a google doc of things you wrote for freshmen. Then, tell them also about NHS tutoring and afterschool tutoring. 
- Finally, show check marks for interests like band, orchestra, sci Olympiad, robotics, track and then based on that show a guide and steps for freshmen. 
- Add a checklist for each year and what you should be doing (also add the college google classroom stuff)
- A way for Brooklyn Tech coaches to add data to create teams (like SportsYou) except it immediately shows every team and the tryouts and discussions
- It gives you notifications for tryouts and includes the daily announcement which can be updated by visiting the website
- Discord-like way to chat about college and other stuff. Also automatic NHS pairing/senior buddy pairing with ability to text them.
- There will also be guides like the locker policy, the requirements for gyms/clubs/advanced diplomas, how to tryout for science Olympiad, how to get into robotics (be charismatic/leader), people's advice, comments (If time permit) and this huge guide/youtube video: BTHS GUIDE VIDEO - Google Docs
- Contains directory for clubs and teachers
- Contains a way to share schedules with other students (if time permits) and a calendar like Saturn
- Add discussion groups for classes that you can join using a code or just freely join but ensure that teachers CANNOT join it. Allow users to be anonymous but also allow a moderator who can perma ban accounts
- Add a group called proposals, crushes, confessions and moderators. Proposals is where you can vote like reddit on proposals to do
- Add discussions/guides part of the site and course discussions/FAQ. I think the best way is if we had a google classroom like UI which has different groups (not to be confused with teams) that had all the courses listed, the teachers there and FAQs about it
- Add events
- Add a teacher appreciation part where you can nominate teachers and they get awards
## Server:
- The website will be a route accepting a username and passcode which will be rate limited heavily (3 attempts per week)
- The website will be made in C++ to be extremely fast and reliable
- Add admins/moderates like teachers
- Store BTHS daily announcements and send in a nice format to phones
- Implement club credit system and club directory

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
