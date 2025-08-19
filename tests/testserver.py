import requests
import subprocess

def expect(text):
  if (not response.text == text):
    exit()

subprocess.run(["redis-cli", "FLUSHALL"])  # Flush server, clear data
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
expect("Expect: Invalid password or username")

url = "http://localhost:4221/uploadschedule"
pdf_path = '../src/ProgramCard.pdf'

# This automatically sets the correct Content-Type and formats the request properly
with open(pdf_path, 'rb') as pdf_file:
  files = {'file': ('./ProgramCard.pdf', pdf_file, 'application/pdf')}
  response = requests.post(url, files=files)
    
print(response.text)