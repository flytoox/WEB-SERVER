#!/usr/bin/env python3

import cgi
import cgitb
import os

# Enable CGI error reporting
cgitb.enable()

# Set the path to store uploaded files
UPLOAD_DIR = "/path/to/upload/directory/"

# HTML template for the response
html_template = """Content-Type: text/html\r\n\r\n
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>File Upload</title>
</head>
<body>
    <h1>File Upload</h1>
    <form enctype="multipart/form-data" action="upload_file.py" method="post">
        <input type="file" name="file">
        <input type="submit" value="Upload">
    </form>
</body>
</html>
"""

# Print the HTML template
print(html_template)

# Get the form data
form = cgi.FieldStorage()

# Check if the form has a file field
if "file" in form:
    # Get the uploaded file
    uploaded_file = form["file"]

    # Get the filename
    filename = os.path.basename(uploaded_file.filename)

    # Save the file to the upload directory
    with open(os.path.join(UPLOAD_DIR, filename), "wb") as f:
        f.write(uploaded_file.file.read())

    print("<p>File uploaded successfully!</p>")
else:
    print("<p>No file selected!</p>")
