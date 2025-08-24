import requests
import subprocess

subprocess.run(["redis-cli", "FLUSHALL"])  # Flush server, clear data

input("Press enter when the server is running...")

try:
  requests.get("http://localhost:4221")
except:
  print("\nYour server is not running!\n")
  exit()

url = "http://localhost:4221/createuser"
data = "username\npassword\nemail"

response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
print(response.text)

url = "http://localhost:4221/login"
data = "username\npassword"

response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
print(response.text)

url = "http://localhost:4221/login"
data = "u\n123"

response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
print(response.text)

url = "http://localhost:4221/api/createteam"
data = "username\npassword\nFRC 334"

response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
print(response.text)

# url = "http://localhost:4221/uploadschedule"
# pdf_path = '../src/ProgramCard.pdf'

# # This automatically sets the correct Content-Type and formats the request properly
# with open(pdf_path, 'rb') as pdf_file:
#   files = {'file': ('./ProgramCard.pdf', pdf_file, 'application/pdf')}
#   response = requests.post(url, files=files)
    
# print(response.text)