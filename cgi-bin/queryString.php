<?php
// Accessing QueryString parameters
$data = $_GET['data']; // This will contain 'adnane'
$name = $_GET['name']; // This will contain 'user'
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>QueryString Test</title>
</head>
<body>
    <h1>QueryString Test</h1>
    <p>Data: <?php echo $data; ?></p>
    <p>Name: <?php echo $name; ?></p>
</body>
</html>
