import requests
import subprocess
import json
from dotenv import load_dotenv
import os

def pf(text):
    print(f"\033[93m{text}\033[0m")

os.system("clear")
load_dotenv()
subprocess.run(["redis-cli", "FLUSHALL"])

def check_server():
    try:
        requests.get("http://localhost:4221")
    except:
        print("\nYour server is not running!\n")
        exit()

def adminSetup():
    url = "http://localhost:4221/adminsetup"
    data = ""
    response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
    print(response.text)

def createUser(user, password, email):
    url = "http://localhost:4221/createuser"
    data = f"{user}\n{password}\n{email}"
    response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
    print(response.text)

def login(username, password):
    url = "http://localhost:4221/login"
    data = f"{username}\n{password}"
    response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
    if response.text != "Invalid password or username":
        print("Logged in to " + username)
        print(response.text)
    else:
        print(response.text)
    return response.text

def makeModerator(adminToken, username):
    pf("Making " + username + " a moderator")
    url = "http://localhost:4221/admin/updateotheruseradminlevel"
    data = f"{username}\n1"
    response = requests.post(url, data=data, headers={"Authorization": adminToken, "Content-Type": "text/plain"})
    print(response.text)

def createTeam(moderatorToken, team_name, team_type):
    url = "http://localhost:4221/mod/createteam"
    data = f"{team_name}\n{team_type}"
    response = requests.post(url, data=data, headers={"Authorization": moderatorToken, "Content-Type": "text/plain"})
    print(response.text)

def getTeamInfo(token, team_name):
    url = "http://localhost:4221/api/getteaminfo"
    data = team_name
    response = requests.post(url, data=data, headers={"Authorization": token, "Content-Type": "text/plain"})
    print(response.text)

def getAllTeams(token):
    url = "http://localhost:4221/api/getallteams"
    data = ""
    response = requests.get(url, data=data, headers={"Authorization": token, "Content-Type": "text/plain"})
    print(response.text)

def getTeamMembers(token, team_name):
    url = "http://localhost:4221/api/getteammembers"
    data = team_name
    response = requests.post(url, data=data, headers={"Authorization": token, "Content-Type": "text/plain"})
    print(response.text)

def getTeamCoaches(token, team_name):
    url = "http://localhost:4221/api/getteamcoaches"
    data = team_name
    response = requests.post(url, data=data, headers={"Authorization": token, "Content-Type": "text/plain"})
    print(response.text)

def createAnnouncements(token, team_name):
    url = "http://localhost:4221/api/createannoucement"
    for i in range(20):
        data = {
            "teamName": team_name,
            "content": str(i),
            "mentions": ["programmers", "test"]
        }
        obj = json.dumps(data)
        response = requests.post(url, obj, headers={"Authorization": token, "Content-Type": "text/plain"})
        print(response.text)

def getAnnouncements(token, team_name):
    url = "http://localhost:4221/api/getannoucements"
    data = team_name
    response = requests.post(url, data=data, headers={"Authorization": token, "Content-Type": "text/plain"})
    print(response.text)

def addUserToTeam(token, team_name):
    url = "http://localhost:4221/api/addusertoteam"
    data = team_name
    response = requests.post(url, data=data, headers={"Authorization": token, "Content-Type": "text/plain"})
    print(response.text)

def addOtherUserToTeam(token, team_name, username):
    url = "http://localhost:4221/mod/addotherusertoteam"
    data = f"{team_name}\n{username}"
    response = requests.post(url, data=data, headers={"Authorization": token, "Content-Type": "text/plain"})
    print(response.text)

def uploadPdfTest(token):
    print("\n--- Testing PDF Upload ---")
    url = "http://localhost:4221/api/uploadschedule"
    script_dir = os.path.dirname(os.path.abspath(__file__))
    pdf_path = os.path.join(script_dir, "ProgramCard.pdf")
    try:
        with open(pdf_path, 'rb') as pdf_file:
            files = {'file': (os.path.basename(pdf_path), pdf_file, 'application/pdf')}
            response = requests.post(
                url,
                files=files,
                headers={"Authorization": token}
            )
            print(f"Status Code: {response.status_code}")
            print(f"Response: {response.text}")
            if response.status_code == 200:
                print("PDF uploaded successfully!")
            else:
                print(f"PDF upload failed with status {response.status_code}")
    except FileNotFoundError:
        print(f"Error: PDF file not found at {pdf_path}")
    except Exception as e:
        print(f"Error uploading PDF: {str(e)}")

def getSchedule(token):
    url = "http://localhost:4221/api/getschedule"
    data = ""
    response = requests.get(url, data=data, headers={"Authorization": token, "Content-Type": "text/plain"})
    print(response.text)

# --- Main execution order ---
check_server()
adminSetup()
createUser("moderator", "pass", "p")
createUser("regularuser", "pass", "p2")

adminToken = login("admin", os.getenv("ADMINPASS"))
regularUserToken = login("regularuser", "pass")
makeModerator(adminToken, "moderator")
moderatorToken = login("moderator", "pass")

createTeam(moderatorToken, "FRC 334", 0)
getTeamInfo(adminToken, "FRC 334")
getAllTeams(adminToken)
getTeamCoaches(adminToken, "FRC 334")
createAnnouncements(adminToken, "FRC 334")
getAnnouncements(adminToken, "FRC 334")
addUserToTeam(regularUserToken, "FRC 334")
getTeamMembers(adminToken, "FRC 334")
# addOtherUserToTeam(adminToken, "FRC 334", "regularuser")
getTeamInfo(adminToken, "FRC 334")
getTeamMembers(adminToken, "FRC 334")
# uploadPdfTest(adminToken)
# getSchedule(adminToken)