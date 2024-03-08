<?php
ob_start(); // Start output buffering

session_start();

// Function to parse the cookie string and populate $_COOKIE
function parseCookies() {
    if (!empty($_SERVER['HTTP_COOKIE'])) {
        $cookies = explode(';', $_SERVER['HTTP_COOKIE']);
        foreach ($cookies as $cookie) {
            $parts = explode('=', $cookie, 2); // Limit to 2 parts to handle '=' in cookie values
            $name = trim($parts[0]);
            $value = isset($parts[1]) ? trim($parts[1]) : '';
            $_COOKIE[$name] = $value;
        }
    }
}

// Call the function to parse cookies
parseCookies();


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
    return isset($_SESSION['loggedIn']) && $_SESSION['loggedIn'] === 'true';
}

// Check if the user is already logged in
if (isset($_COOKIE['loggedIn']) && $_COOKIE['loggedIn'] === 'true') {
    // If the user is logged in, create a session
    $_SESSION['loggedIn'] = $_COOKIE['loggedIn'];
    $_SESSION['username'] = $_COOKIE['username'];
    echo "Setting Logged In: " . $_SESSION['loggedIn']; // Debug output
    echo "<br/>";
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
    <?php if (isLoggedIn()): ?>
        <h1>Welcome, <?php echo $_SESSION['username']; ?>!</h1>
    <?php else: ?>
        <h1>Login</h1>
        <form action="/Desktop/WebServ/cgiTests/login.php" method="post">
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

    if (authenticateUser($username, $password) && !isLoggedIn()) {
		// If authentication succeeds, create a session and set cookies
		$_SESSION['username'] = $username;
		setcookie('loggedIn', 'true', time() + (86400 * 30), '/'); // Set cookie to expire in 30 days
		setcookie('username', $username, time() + (86400 * 30), '/'); // Set cookie to expire in 30 days

        header('Location: /Desktop/WebServ/cgiTests/login.php'); // Redirect to the same page
		exit;
	}

}
?>

<?php
ob_end_flush(); // Flush output buffer and send content to the browser
?>
