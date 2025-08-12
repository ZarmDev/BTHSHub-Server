# Send post request to server (with data)
curl -X POST -d "" localhost:4221/
# Create an account
curl -X POST -d "username\npassword\nemail" localhost:4221/createuser
# Login (generates token)
curl -X POST -d "username\npassword" localhost:4221/login
# Clear redis DB
redis-cli FLUSHALL