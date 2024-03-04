<?php
ob_start(); // Start output buffering

session_start();

// Function to authenticate user
function authenticateUser($username, $password) {
    // Here you would typically validate the username and password against a database
    // For simplicity, we'll just check if the username and password match a hardcoded value
    $validUsername = 'admin';
    $validPassword = 'password';

    if ($username === $validUsername && $password === $validPassword) {
        return true;
    } else {
        return false;
    }
}

// Function to check if the user is logged in
function isLoggedIn() {
    return isset($_SESSION['username']);
}

// Check if the user is logging out
if (isset($_GET['logout'])) {
    session_destroy();
    setcookie('loggedIn', '', time() - 3600); // Remove the cookie
    header('Location: login.php'); // Redirect to login page
    exit;
}

// Check if the user is already logged in
if (isset($_COOKIE['loggedIn']) && $_COOKIE['loggedIn'] === 'true') {
    // If the user is logged in, create a session
    $_SESSION['username'] = $_COOKIE['username'];
    echo "Setting session username: " . $_SESSION['username']; // Debug output
}

?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login</title>
</head>
<body>
	<!-- print logged in and authentication status -->
	<p>Logged in: <?php echo isLoggedIn() ? 'true' : 'false'; ?></p>
	<p>Authentication status: <?php echo isset($_SESSION['username']) ? 'true' : 'false'; ?></p>
    <?php if (isLoggedIn()): ?>
        <h1>Welcome, <?php echo $_SESSION['username']; ?>!</h1>
        <p><a href="?logout">Logout</a></p>
    <?php else: ?>
        <h1>Login</h1>
        <form action="/Desktop/WebServ/cgi-bin/login.php" method="post">
            <label for="username">Username:</label>
            <input type="text" id="username" name="username" required><br><br>
            <label for="password">Password:</label>
            <input type="password" id="password" name="password" required><br><br>
            <button type="submit">Login</button>
        </form>
    <?php endif; ?>

</body>
</html>

<?php
// Handle login form submission
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $username = $_POST['username'];
    $password = $_POST['password'];

    if (authenticateUser($username, $password)) {
		// If authentication succeeds, create a session and set cookies
		$_SESSION['username'] = $username;
		setcookie('loggedIn', 'true', time() + (86400 * 30), '/'); // Set cookie to expire in 30 days
		setcookie('username', $username, time() + (86400 * 30), '/'); // Set cookie to expire in 30 days

		// Debug output
		echo "Setting session username: " . $_SESSION['username'] . "<br>";
		echo "Setting cookie loggedIn: true<br>";
		echo "Setting cookie username: " . $username . "<br>";

		header("Location: /Desktop/WebServ/cgi-bin/login.php"); // Redirect to the login page to refresh the view
		echo "Redirecting to login page..."; // Debug output
		// exit;
	}

}
?>

<?php
ob_end_flush(); // Flush output buffer and send content to the browser
?>
