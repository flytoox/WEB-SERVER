//<?php
//    // Get the values submitted in the form
//    $firstName = isset($_POST['firstName']) ? $_POST['firstName'] : '';
//    $lastName = isset($_POST['lastName']) ? $_POST['lastName'] : '';
//    $name = isset($_POST['name']) ? $_POST['name'] : '';
//
//    // Output the body
//    echo "<!DOCTYPE html>\n";
//    echo "<html>\n";
//    echo "<head>\n";
//    echo "    <title>CGI Example</title>\n";
//    echo "</head>\n";
//    echo "<body>\n";
//    echo "    <h1>Hello, CGI!</h1>\n";
//    echo "    <p>First Name: $firstName</p>\n";
//    echo "    <p>Last Name: $lastName</p>\n";
//    echo "    <p>Name: $name</p>\n";
//    echo "</body>\n";
//    echo "</html>\n";
//?>

<?php
// Debugging script to print received POST data

// Check Content-Type
$contentType = isset($_SERVER["CONTENT_TYPE"]) ? $_SERVER["CONTENT_TYPE"] : "Not specified";
echo "Content-Type: " . $contentType . PHP_EOL;

// Check Content-Length
$contentLength = isset($_SERVER["CONTENT_LENGTH"]) ? $_SERVER["CONTENT_LENGTH"] : "Not specified";
echo "Content-Length: " . $contentLength . PHP_EOL;

// Read raw POST data from php://input
$rawPostData = file_get_contents('php://input');

// Print raw POST data
echo "Raw POST data: " . $rawPostData . PHP_EOL;
?>

