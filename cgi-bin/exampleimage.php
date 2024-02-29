<?php
// Specify the path to the existing WebP image file
$imagePath = "/Users/aait-mal/Desktop/nokia.webp";

// Check if the image file exists
if (file_exists($imagePath)) {
    // Set the Content-Type header to indicate that the response is a WebP image
    header("Content-Type: image/webp");

    // Output the WebP image to the browser
    readfile($imagePath);
} else {
    // If the image file doesn't exist, return an error image or handle accordingly
    header("Content-Type: image/png"); // Use a default error image
    readfile("/path/to/error/image.png");
}
?>