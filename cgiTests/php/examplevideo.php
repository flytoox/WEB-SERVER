<?php
// video.php

// Set the path to the video file
$videoPath = '/Users/aait-mal/goinfre/vid.mp4';

// Check if the file exists
if (file_exists($videoPath)) {
    // Set appropriate headers for video streaming
    header('Content-Type: video/mp4');
    header('Content-Length: ' . filesize($videoPath));

    // Output the video content
    readfile($videoPath);
} else {
    // Return a 404 Not Found response if the video file doesn't exist
    header("HTTP/1.0 404 Not Found");
    echo "File not found.";
}
?>
