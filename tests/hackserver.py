# script to attempt to find any exploits in the server
print("-----------------------FINDING BUGS-----------------------")
exit()

url = "http://localhost:4221/login"
data = "u\n123"

response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})

url = "http://localhost:4221/mod/createteam"
data = "FRC 334\n0"

response = requests.post(url, data=data, headers={"Authorization": token, "Content-Type": "text/plain"})
print(response.text)