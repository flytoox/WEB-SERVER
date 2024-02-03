//<?php
//    // Read the raw input for POST requests
//    $rawPostData = file_get_contents('php://input');
//
//    // Parse the raw input for POST data
//    parse_str($rawPostData, $_POST);
//
//    // Get the values submitted in the form using $_POST
//    $firstName = isset($_POST['firstName']) ? urldecode($_POST['firstName']) : '';
//    $lastName = isset($_POST['lastName']) ? urldecode($_POST['lastName']) : '';
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
//
//    // Display the contents of $_POST
//    echo "<p>POST data:</p>\n";
//    echo "<pre>";
//    print_r($_POST);
//    echo "</pre>";
//
//    echo "</body>\n";
//    echo "</html>\n";
//?>

<?php
echo '<pre>POST data:' . print_r($_POST, true) . '</pre>';
?>

