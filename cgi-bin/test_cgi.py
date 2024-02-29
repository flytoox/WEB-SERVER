#!/usr/bin/env python3

import cgi

print("Content-Type: text/html\n")
print("<html><body>")
print("<h1>CGI Test</h1>")

form = cgi.FieldStorage()

if "data" in form:
    print("<p>Received POST data: {}</p>".format(form["data"].value))
else:
    print("<p>No POST data received.</p>")

print("</body></html>")
