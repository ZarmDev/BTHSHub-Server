import requests
import subprocess
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
data = f"admin\n{os.getenv("ADMINPASS")}"

response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
print(response.text)
token = (response.text)

url = "http://localhost:4221/createteam"
data = "FRC 334"

response = requests.post(url, data=data, headers={"Authorization": token, "Content-Type": "text/plain"})
print(response.text)

url = "http://localhost:4221/getallteams"
data = ""

response = requests.get(url, data=data, headers={"Authorization": token, "Content-Type": "text/plain"})
print(response.text)

# url = "http://localhost:4221/uploadschedule"
# pdf_path = '../src/ProgramCard.pdf'

# # This automatically sets the correct Content-Type and formats the request properly
# with open(pdf_path, 'rb') as pdf_file:
#   files = {'file': ('./ProgramCard.pdf', pdf_file, 'application/pdf')}
#   response = requests.post(url, files=files)
    
# print(response.text)

print("-----------------------ERROR TESTING-----------------------")
exit()

url = "http://localhost:4221/login"
data = "u\n123"

response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})

url = "http://localhost:4221/createteam"
data = "FRC 334"

response = requests.post(url, data=data, headers={"Authorization": token, "Content-Type": "text/plain"})
print(response.text)