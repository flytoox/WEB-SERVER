<?php
// Get the POST data
$postData = $_POST['data'];
$mail = $_POST['mail'];

// Output HTTP headers
header("Content-Type: text/html");

// Output the HTML response
echo "<html><body>";
echo "<h1>CGI Test (PHP)</h1>";

if (!empty($postData)) {
    echo "<p>Received POST data: " . htmlspecialchars($postData) . "</p>";
    echo "<p>Received POST mail: " . htmlspecialchars($mail) . "</p>";
} else {
    echo "<p>No POST data received.</p>";
}

echo "</body></html>";
?>

