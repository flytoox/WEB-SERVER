#!/usr/bin/env python

# Importing necessary modules
import cgi
import requests
# Set content type to HTML
print("Content-type:text/html\r\n\r\n")

# Start HTML
print("<html>")
print("<head>")
print("<title>CGI Example</title>")
print("</head>")
print("<body>")
print("<h2>This is a CGI Example</h2>")
print("<p>Hello from Python CGI!</p>")
print("</body>")
print("</html>")

session = requests.Session()
session.get('http://localhost:1111/cgiTests/get.py')
session.close()
