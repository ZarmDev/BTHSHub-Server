# THIS SCRIPT WILL CLEAR EVERYTHING AND START ANEW! (For debugging)
import requests
import subprocess

def adminSetup():
    url = "http://localhost:4221/adminsetup"
    data = ""
    response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
    print(response.text)

def check_server():
    try:
        requests.get("http://localhost:4221")
    except:
        print("\nYour server is not running!\n")
        exit()

check_server()
subprocess.run(["redis-cli", "FLUSHALL"])
adminSetup()