import requests
import subprocess
import json
from dotenv import load_dotenv
import os

def pf(text):
  print(f"\033[93m{text}\033[0m")


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
data = "moderator\npassword\nemail"

response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
print(response.text)

url = "http://localhost:4221/createuser"
data = "regularuser\npassword\nemail2"

response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
print(response.text)

url = "http://localhost:4221/login"
data = "admin\n" + os.getenv("ADMINPASS")

response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
print(response.text)
adminToken = response.text

url = "http://localhost:4221/login"
data = "regularuser\n" + "password"

response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
print(response.text)
regularusertoken = response.text

# Make moderator (the user) a moderator
pf("mmaking moderator a moderator" + adminToken)
url = "http://localhost:4221/admin/updateotheruseradminlevel"
data = "moderator\n" + "1"

response = requests.post(url, data=data, headers={"Authorization": adminToken,"Content-Type": "text/plain"})
print(response.text)

# Relogin with moderator and password and check if token works later on as a moderator
url = "http://localhost:4221/login"
data = "moderator\n" + "password"

response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
print(response.text)
moderatorToken = response.text

url = "http://localhost:4221/mod/createteam"
data = "FRC 334\n0"

response = requests.post(url, data=data, headers={"Authorization": moderatorToken, "Content-Type": "text/plain"})
print(response.text)

# url = "http://localhost:4221/api/addusertoteam"
# data = "FRC 334\n0"

# response = requests.post(url, data=data, headers={"Authorization": adminToken, "Content-Type": "text/plain"})
# print(response.text)

url = "http://localhost:4221/api/getteaminfo"
data = "FRC 334"

response = requests.post(url, data=data, headers={"Authorization": adminToken, "Content-Type": "text/plain"})
print(response.text)

url = "http://localhost:4221/api/getallteams"
data = ""

response = requests.get(url, data=data, headers={"Authorization": adminToken, "Content-Type": "text/plain"})
print(response.text)

url = "http://localhost:4221/api/getteammembers"
data = "FRC 334"

response = requests.get(url, data=data, headers={"Authorization": adminToken, "Content-Type": "text/plain"})
print(response.text)

url = "http://localhost:4221/api/getteamcoaches"
data = "FRC 334"

response = requests.get(url, data=data, headers={"Authorization": adminToken, "Content-Type": "text/plain"})
print(response.text)

url = "http://localhost:4221/api/createannoucement"
for i in range(20):
  print("sending " + str(i))
  data = {
    "teamName": "FRC 334",
    "content": str(i),
    "mentions": ["programmers", "test"]
  }

  obj = json.dumps(data)

  response = requests.post(url, obj, headers={"Authorization": adminToken, "Content-Type": "text/plain"})

print(response.text)

url = "http://localhost:4221/api/getannoucements"
data = "FRC 334"

response = requests.post(url, data=data, headers={"Authorization": adminToken, "Content-Type": "text/plain"})
print(response.text)

url = "http://localhost:4221/mod/addotherusertoteam"
data = "FRC 334\nregularuser"

response = requests.post(url, data=data, headers={"Authorization": adminToken, "Content-Type": "text/plain"})
print(response.text)

url = "http://localhost:4221/api/getteaminfo"
data = "FRC 334"

response = requests.post(url, data=data, headers={"Authorization": adminToken, "Content-Type": "text/plain"})
print(response.text)

url = "http://localhost:4221/api/getteammembers"
data = "FRC 334"

response = requests.get(url, data=data, headers={"Authorization": adminToken, "Content-Type": "text/plain"})
print(response.text)

def upload_pdf_test():
    print("\n--- Testing PDF Upload ---")
    
    # URL for PDF upload endpoint
    url = "http://localhost:4221/api/uploadpdf"

    # Get the directory of the current script
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # Construct the relative path to the PDF file
    pdf_path = os.path.join(script_dir, "ProgramCard.pdf")
    
    try:
        # Open the PDF file in binary mode
        with open(pdf_path, 'rb') as pdf_file:
            # Create the multipart/form-data request with the PDF file
            files = {'file': (os.path.basename(pdf_path), pdf_file, 'application/pdf')}
            
            # Send the request with authentication token
            response = requests.post(
                url, 
                files=files,
                headers={"Authorization": adminToken}  # Use the admin token for authorization
            )
            
            # Print the response status and content
            print(f"Status Code: {response.status_code}")
            print(f"Response: {response.text}")
            
            # Check if upload was successful
            if response.status_code == 200:
                print("PDF uploaded successfully!")
            else:
                print(f"PDF upload failed with status {response.status_code}")
                
    except FileNotFoundError:
        print(f"Error: PDF file not found at {pdf_path}")
    except Exception as e:
        print(f"Error uploading PDF: {str(e)}")

# Add this to the end of your test script
upload_pdf_test()

url = "http://localhost:4221/api/getschedule"
data = ""

response = requests.get(url, data=data, headers={"Authorization": adminToken, "Content-Type": "text/plain"})
print(response.text)

# url = "http://localhost:4221/uploadschedule"
# pdf_path = '../src/ProgramCard.pdf'

# # This automatically sets the correct Content-Type and formats the request properly
# with open(pdf_path, 'rb') as pdf_file:
#   files = {'file': ('./ProgramCard.pdf', pdf_file, 'application/pdf')}
#   response = requests.post(url, files=files)
    
# print(response.text)