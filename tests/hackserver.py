# script to attempt to find any exploits in the server
import requests
import json
import string
import random

print("-----------------------FINDING BUGS-----------------------")

url = "http://localhost:4221/login"
data = "u\n123"

response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})

print(response.text)

url = "http://localhost:4221/mod/createteam"
data = "FRC 334\n0"

response = requests.post(url, data=data, headers={"Authorization": "ddddwdawdwddaw", "Content-Type": "text/plain"})
print(response.text)

# GENERATED WITH AI ALL BELOW

def test_header(name, description):
    print(f"\n{'='*20} {name} {'='*20}")
    print(f"Testing: {description}")
    print("-" * (42 + len(name)))

# Test 1: Invalid login attempts
test_header("INVALID LOGIN", "Testing login with non-existent users")

url = "http://localhost:4221/login"
data = "nonexistent_user\nwrong_password"
response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
print(f"Non-existent user: {response.text}")

# SQL injection attempts in login
data = "'; DROP TABLE users; --\npassword"
response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
print(f"SQL injection attempt: {response.text}")

# Test 2: Authorization bypass attempts
test_header("AUTH BYPASS", "Testing with invalid/malformed tokens")

fake_tokens = [
    "fake_jwt_token",
    "",
    "Bearer fake_token",
    "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.fake.token",
    "a" * 8193,  # Very long token
    "a" * 1000,  # Very long token
    None
]

for i, token in enumerate(fake_tokens):
    print(f"\nTest {i+1}: Token = {token[0:3]}...")
    headers = {"Content-Type": "text/plain"}
    if token is not None:
        headers["Authorization"] = token
    
    # Try protected routes
    response = requests.post("http://localhost:4221/mod/createteam", 
                           data="HackTeam\n0", headers=headers)
    print(f"  Create team: {response.status_code} - {response.text[:50]}")
    
    response = requests.post("http://localhost:4221/admin/updateotheruseradminlevel", 
                           data="target\n2", headers=headers)
    print(f"  Admin action: {response.status_code} - {response.text[:50]}")

# Test 3: Buffer overflow attempts
test_header("BUFFER OVERFLOW", "Testing with extremely long requests")

# Generate very long strings
long_string = "A" * 10000
very_long_string = "B" * 100000

test_cases = [
    ("Long username", f"{long_string}\npassword"),
    ("Long password", f"user\n{long_string}"),
    ("Both long", f"{long_string}\n{very_long_string}"),
    ("Long team name", f"{long_string}\n0"),
]

for name, data in test_cases:
    print(f"\n{name}:")
    try:
        response = requests.post("http://localhost:4221/login", 
                               data=data, headers={"Content-Type": "text/plain"}, 
                               timeout=5)
        print(f"  Status: {response.status_code}, Length: {len(response.text)}")
    except requests.exceptions.RequestException as e:
        print(f"  Exception: {e}")

# Test 4: Malformed Content-Length header
test_header("MALFORMED HEADERS", "Testing with wrong Content-Length")

malformed_requests = [
    {"Content-Length": "999999", "data": "short"},
    {"Content-Length": "-1", "data": "test"},
    {"Content-Length": "abc", "data": "test"},
    {"Content-Length": "0", "data": "actually_has_data"},
]

for test_case in malformed_requests:
    print(f"\nTesting Content-Length: {test_case['Content-Length']}")
    headers = {
        "Content-Type": "text/plain",
        "Content-Length": test_case["Content-Length"]
    }
    try:
        response = requests.post("http://localhost:4221/login", 
                               data=test_case["data"], headers=headers, timeout=5)
        print(f"  Response: {response.status_code} - {response.text[:50]}")
    except requests.exceptions.RequestException as e:
        print(f"  Exception: {e}")

# Test 5: Special characters and encoding attacks
test_header("SPECIAL CHARS", "Testing with special characters and encoding")

special_payloads = [
    "user\x00admin\npassword",  # Null byte injection
    "user\r\nadmin\npassword",  # CRLF injection
    "user\\nadmin\\npassword",  # Backslash injection (your code checks for this)
    "user\"\npassword",         # Quote injection
    "user'\npassword",          # Single quote injection
    "user<script>\npassword",   # XSS attempt
    "user/../admin\npassword",  # Path traversal attempt
    "用户\n密码",               # Unicode characters
]

for payload in special_payloads:
    print(f"\nTesting payload: {repr(payload)}")
    try:
        response = requests.post("http://localhost:4221/login", 
                               data=payload, headers={"Content-Type": "text/plain"}, 
                               timeout=5)
        print(f"  Response: {response.status_code} - {response.text[:50]}")
    except requests.exceptions.RequestException as e:
        print(f"  Exception: {e}")

# Test 6: JSON injection in announcement creation
test_header("JSON INJECTION", "Testing malformed JSON in announcements")

# First, let's try to get a valid token (you'll need to adjust this)
url = "http://localhost:4221/login"
data = "admin\nactual_admin_password"  # Replace with real admin creds
response = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
admin_token = response.text if response.status_code == 200 else "fake_token"

json_payloads = [
    '{"teamName":"test","content":"normal"}',  # Missing mentions
    '{"teamName":"","content":"test","mentions":[]}',  # Empty team name
    '{"teamName":null,"content":"test","mentions":[]}',  # Null team name
    '{"teamName":"' + "A" * 1000 + '","content":"test","mentions":[]}',  # Very long team name
    '{malformed json',  # Invalid JSON
    '{"teamName":"test","content":"' + "A" * 10000 + '","mentions":[]}',  # Very long content
    '{"teamName":"../admin","content":"test","mentions":[]}',  # Path traversal in team name
]

for payload in json_payloads:
    print(f"\nTesting JSON: {payload[:50]}...")
    try:
        response = requests.post("http://localhost:4221/api/createannoucement", 
                               data=payload, 
                               headers={"Authorization": admin_token, "Content-Type": "text/plain"}, 
                               timeout=5)
        print(f"  Response: {response.status_code} - {response.text[:50]}")
    except requests.exceptions.RequestException as e:
        print(f"  Exception: {e}")

# Test 7: Race condition testing
test_header("RACE CONDITIONS", "Testing concurrent requests")

import threading
import time

def make_request(url, data, headers, results, index):
    try:
        response = requests.post(url, data=data, headers=headers, timeout=5)
        results[index] = f"Status: {response.status_code}, Text: {response.text[:30]}"
    except Exception as e:
        results[index] = f"Exception: {e}"

# Try to create the same team multiple times simultaneously
results = [""] * 10
threads = []

for i in range(10):
    thread = threading.Thread(target=make_request, args=(
        "http://localhost:4221/mod/createteam",
        f"RaceTeam{i}\n0",
        {"Authorization": admin_token, "Content-Type": "text/plain"},
        results,
        i
    ))
    threads.append(thread)

# Start all threads at roughly the same time
for thread in threads:
    thread.start()

# Wait for all to complete
for thread in threads:
    thread.join()

for i, result in enumerate(results):
    print(f"Thread {i}: {result}")

# Test 8: Memory exhaustion
test_header("MEMORY EXHAUSTION", "Testing multiple large requests")

print("Sending multiple large requests...")
for i in range(5):
    large_data = f"user{i}\n" + "A" * 50000
    try:
        response = requests.post("http://localhost:4221/login", 
                               data=large_data, 
                               headers={"Content-Type": "text/plain"}, 
                               timeout=5)
        print(f"Request {i}: {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"Request {i} failed: {e}")

print("\n" + "="*60)
print("SECURITY TESTING COMPLETE")
print("="*60)