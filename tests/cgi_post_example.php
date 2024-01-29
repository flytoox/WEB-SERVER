<?php
header('Content-Type: application/json');

// Read POST data
$postData = file_get_contents('php://input');
$parsedData = json_decode($postData, true);

if ($parsedData === null) {
    // JSON parsing failed
    http_response_code(400); // Bad Request
    echo json_encode(['error' => 'Invalid JSON data']);
} else {
    // Process the data
    $responseData = [
        'status' => 'success',
        'message' => 'Data received successfully',
        'data' => $parsedData,
    ];

    // Return the response as JSON
    echo json_encode($responseData);
}
?>
