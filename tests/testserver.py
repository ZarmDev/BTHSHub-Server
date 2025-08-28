import requests
import subprocess
import json
from dotenv import load_dotenv
import os

os.system("clear")

load_dotenv()  # Loads variables from .env into environment

subprocess.run(["redis-cli", "FLUSHALL"])  # Flush server, clear data

try:
  requests.get("http://localhost:4221")
except:
  print("\nYour server is not running!\n")
  exit()

url = "http://localhost:4221/adminsetup"
data = ""

response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
print(response.text)

url = "http://localhost:4221/createuser"
data = "username\npassword\nemail"

response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
print(response.text)

url = "http://localhost:4221/login"
data = "admin\n" + os.getenv("ADMINPASS")
print(os.getenv("ADMINPASS"))

response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
print(response.text)
token = (response.text)

url = "http://localhost:4221/mod/createteam"
data = "FRC 334\n0"

response = requests.post(url, data=data, headers={"Authorization": token, "Content-Type": "text/plain"})
print(response.text)

# url = "http://localhost:4221/api/addusertoteam"
# data = "FRC 334\n0"

# response = requests.post(url, data=data, headers={"Authorization": token, "Content-Type": "text/plain"})
# print(response.text)

url = "http://localhost:4221/api/getteaminfo"
data = "FRC 334"

response = requests.get(url, data=data, headers={"Authorization": token, "Content-Type": "text/plain"})
print(response.text)

url = "http://localhost:4221/api/getallteams"
data = ""

response = requests.get(url, data=data, headers={"Authorization": token, "Content-Type": "text/plain"})
print(response.text)

url = "http://localhost:4221/api/createannoucement"
for i in range(20):
  data = {
    "teamName": "FRC 334",
    "content": str(i),
    "mentions": ["programmers", "test"]
  }

  obj = json.dumps(data)

  response = requests.post(url, data=obj, headers={"Authorization": token, "Content-Type": "text/plain"})

print(response.text)

url = "http://localhost:4221/api/getannoucements"
data = "FRC 334"

response = requests.get(url, data=data, headers={"Authorization": token, "Content-Type": "text/plain"})
print(response.text)

# url = "http://localhost:4221/uploadschedule"
# pdf_path = '../src/ProgramCard.pdf'

# # This automatically sets the correct Content-Type and formats the request properly
# with open(pdf_path, 'rb') as pdf_file:
#   files = {'file': ('./ProgramCard.pdf', pdf_file, 'application/pdf')}
#   response = requests.post(url, files=files)
    
# print(response.text)