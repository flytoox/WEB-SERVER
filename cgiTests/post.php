<?php
// Get the POST data
$postData = $_POST['data'];

// Output HTTP headers
header("Content-Type: text/html");

// Output the HTML response
echo "<!DOCTYPE html>\n";
echo "<html>\n";
echo "<head>\n";
echo "    <title>CGI Example</title>\n";
echo "    <link rel=\"stylesheet\" href=\"/Desktop/WebServ/html/styles.css\">\n";
echo "</head>\n";
echo "<body>\n";
echo "<div class=\"header\">\n";
echo "    <ul class=\"headerLinks\">\n";
echo "        <li class=\"headerItem\" ><a href=\"/\">Root</a></li>\n";
echo "        <li class=\"headerItem\" ><a href=\"/Desktop/WebServ/html/upload.html\">Upload</a></li>\n";
echo "        <li class=\"headerItem\" ><a href=\"/Desktop/WebServ/cgiTests/example.php\">Get</a></li>\n";
echo "        <li class=\"headerItem\" ><a href=\"/Desktop/WebServ/html/test_cgi.html\">Post</a></li>\n";
echo "        <li class=\"headerItem\" ><a href=\"/Desktop/WebServ/cgiTests/login.php\">Login</a></li>\n";
echo "    </ul>\n";
echo "</div>\n";
echo "<div class=\"container\">\n";
echo "    <div class=\"head\">\n";
if (!empty($postData)) {
    echo "<h1 >Received POST data: <font class=\"special\">" . htmlspecialchars($postData) . "</font></h1>";
} else {
    echo "<h1 >No POST data received</h1>";
}
echo "    </div>\n";
echo "    <div>\n";
echo "</div>";
echo "</body>\n";
echo "</html>\n";
?>
