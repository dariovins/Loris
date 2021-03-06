<?php
/**
 * This class provides a single point to deal with user authentication
 * and validation of login, displaying the login page if necessary.
 *
 * PHP Version 5
 *
 * @category Main
 * @package  Main
 * @author   Loris Team <loris.mni@bic.mni.mcgill.ca>
 * @license  http://www.gnu.org/licenses/gpl-3.0.txt GPLv3
 * @link     https://www.github.com/aces/Loris/
 */

/**
 * SinglePointLogin class
 *
 * This class provides a single-point user login to the NeuroDB GUIs
 * Its public functions can be used to display a login window at the
 * current location while preserving the current state of the GUI.
 *
 * @category Main
 * @package  Main
 * @author   Loris Team <loris.mni@bic.mni.mcgill.ca>
 * @license  http://www.gnu.org/licenses/gpl-3.0.txt GPLv3
 * @link     https://www.github.com/aces/Loris/
 */
class SinglePointLogin
{
    /**
     * Defines whether a user is currently logged in
     */
    var $_isLoggedIn = false;

    /**
     * Internally held copy of username
     */
    var $_username = null;

    /**
     * To carry the error messages
     */
    var $_lastError = null;

    /**
     * Defines whether a user's password has expired
     */
    var $_passwordExpired = false;

    /**
     * Dumps the password expiry form
     *
     * @return void
     * @access public
     */
    function showPasswordExpiryScreen()
    {
        // FIXME: This is a temporary hack until the password reset logic is
        // refactored to be sane and keep a token that can be authenticated.
        //
        // We store the username requesting the the password reset on the
        // PHP session superglobal so that the password expiry page can
        // trust it.
        $_SESSION['PasswordExpiredForUser'] = $_POST['username'];
        header("HTTP/1.1 303 See Other");
        header("Location: /login/password-expiry/");
        exit(0);
    }


    /**
     * Checks whether anyone is currently logged in
     *
     * @return bool
     * @access public
     */
    function isLoggedIn()
    {
        return $this->_isLoggedIn;
    }

    /**
     * Checks whether users password is expired
     *
     * @return bool
     * @access public
     */
    function passwordExpired()
    {
        return $this->_passwordExpired;
    }

    /**
     * Saves the new password if last was expired
     *
     * @return void
     * @access public
     */
    function save()
    {
        // get saved data to pre-populate form
        $user =& User::factory($_POST['username']);

        // get user's data
        $data = $user->getData();

        // check password strength
        if (!User::isPasswordStrong(
            $_POST['password'],
            array(
             $_POST['confirm'] ?? '',
             $data['UserID'],
             $data['Email'],
            ),
            array(
             '==',
             '!=',
             '!=',
            )
        )
        ) {
            $this->_lastError = 'The password is weak, or'
               . ' the passwords do not match';
        }

        if (password_verify($_POST['password'], $data['Password_hash'])) {
            $this->_lastError = 'You cannot keep the same password';
        }

        // if errors
        if (!empty($this->_lastError)) {
            return false;
        }

        // Reset passwordExpired flag
        $this->_passwordExpired = false;
        $user->updatePassword($_POST['password']);
        return true;
    }

    /**
     * Attempt to authenticate a user's credentials by any supported means.
     *
     * @return void (but may modified session state)
     */
    function authenticate()
    {
        // Reset passwordExpired flag
        $this->_passwordExpired = false;

        if (!function_exists('getallheaders')) {
            /**
             * In case unable to detect getallheaders function
             *
             * @return array headers
             */
            function getallheaders()
            {
                $headers = '';
                foreach ($_SERVER as $name => $value) {
                    if (substr($name, 0, 5) == 'HTTP_') {
                        $headers[str_replace(
                            ' ',
                            '-',
                            ucwords(
                                strtolower(
                                    str_replace(
                                        '_',
                                        ' ',
                                        substr(
                                            $name,
                                            5
                                        )
                                    )
                                )
                            )
                        )] = $value;
                    }
                }
                return $headers;
            }
        }
        // First try JWT authentication, which is cheaper and
        // doesn't involve database calls
        $headers    = getallheaders();
        $authHeader = isset($headers['Authorization'])
                         ? $headers['Authorization']
                         : '';
        if (!empty($authHeader)) {
            $token = explode(" ", $authHeader);
            // Index 0 is "Bearer", 1 is the token
            if ($this->JWTAuthenticate($token[1]) === true) {
                return true;
            }

        }

        // Now try password validation

        // This wasn't a login form, so don't waste any time
        if (empty($_POST['login'])) {
            return false;
        }
        return $this->passwordAuthenticate($_POST['username'], $_POST['password']);
    }

    /**
     * Attempts to authenticate a JWT token
     *
     * @param string $token The token extracted from the
     *                      Authorization: Bearer header
     *
     * @return true if the session is valid, false otherwise
     */
    function JWTAuthenticate($token)
    {
        $factory = NDB_Factory::singleton();
        $config  = $factory->config();

        $sharedKey = $config->getSetting("JWTKey");

        //$jwt = \Firebase\JWT\JWT::encode($token, $sharedKey);
        try {
            $decoded = \Firebase\JWT\JWT::decode($token, $sharedKey, array("HS256"));
        } catch(Exception $e) {
            return false;
        }

        $decodedArray    = (array) $decoded;
        $this->_username
            = isset($decodedArray['user'])
                ? $decodedArray['user'] : 'Unknown';
        return isset($decodedArray['user']);
    }

    /**
     * Checks whether a given username and password are valid
     *
     * @param string $username The username to validate
     * @param string $password The username to validate
     * @param bool   $redirect If this flag is true, this
     *                         function may instead print
     *                         out a login or password expiry
     *                         page. when trying to authenticate.
     *                         If false, it won't provide any output
     *                         in those situation.
     *
     * @return bool true if the username and password are valid
     * @access public
     */
    function passwordAuthenticate($username, $password, $redirect = true)
    {
        $this->_lastError = '';

         ///////////////////////////
         ///initialization/////
         //////////////////////////
        $this->_username = isset($username) ? $username : 'Unknown';

        $setArray = array(
                     'userID'          => $this->_username,
                     'Success'         => 'Y',
                     'Failcode'        => null,
                     'Login_timestamp' => date('Y-m-d H:i:s'),
                     'IP_address'      => isset($_SERVER['REMOTE_ADDR'])
                                              ? $_SERVER['REMOTE_ADDR']
                                              : 'Unknown',
                     'Page_requested'  => isset($_SERVER['REQUEST_URI'])
                                              ? $_SERVER['REQUEST_URI']
                                              : 'Unknown',
                    );
        // don't waste execution time if no form was submitted

        if (empty($username)) {
            $this->_lastError = 'Please enter a username';
            $this->insertFailedDetail(
                $this->_lastError,
                $setArray
            );
            return false;
        }

        if (empty($password)) {
            $this->_lastError = 'Please enter a password';
            $this->insertFailedDetail(
                $this->_lastError,
                $setArray
            );
            return false;
        }

        // create DB object
        $factory = NDB_Factory::singleton();
        $DB      = $factory->database();

        // check users table to see if we have a valid user
        $query = "SELECT COUNT(*) AS User_count,
                        UserID,
                        Password_expiry < NOW() as Expired,
                        Active,
                        Pending_approval,
                        Password_hash,
			active_from,
			active_to

                    FROM users
                  WHERE UserID = :username
                  GROUP BY UserID";
        $row   = $DB->pselectRow($query, array('username' => $username));

        if (($row['User_count'] ?? 0) == 1) {
            // validate passsword
            $oldhash = $row['Password_hash'];
            if (password_verify($password, $oldhash)) {
                if (password_needs_rehash($oldhash, PASSWORD_DEFAULT)) {
                    $user =& User::factory($username);
                    $user->updatePassword($password);
                }

                if ($row['Active'] == 'N'
                    || $this->disabledDueToInactivity($username, 365)
                ) {
                    $this->_lastError = "Your account has been deactivated."
                        . " Please contact your project administrator to"
                        . " reactivate this account.";
                    $this->insertFailedDetail(
                        "user account not active",
                        $setArray
                    );

                    return false;
                }

                // check if the account is longer active
                $date       = new DateTime();
                $currentDay = $date->getTimestamp();

                if (($row['active_to'] != null)
                    && ($currentDay > strtotime($row['active_to']))
                ) {
                    $this->_lastError = "Your account has expired."
                        . " Please contact your project administrator to re-activate"
                        . " this account.";
                    return false;

                }

                // check if the account is active
                if (($row['active_from'] != null)
                    && ($currentDay < strtotime($row['active_from']))
                ) {
                    $this->_lastError = "Your account is not active yet."
                        ." According to our records it will be active from 
			{$row['active_from']}"
                        . " Please contact your project administrator";
                    return false;

                }

                if ($row['Pending_approval'] == 'Y') {
                    $this->_lastError = "Your account has not yet been activated."
                        . " Please contact your project administrator to activate"
                        . " this account.";
                    $this->insertFailedDetail("user account pending", $setArray);
                    return false;
                }

                // save the new password if the last password expired
                if ($row['Expired']) {
                    $success = $this->save();

                    if ($success == false) {
                        if ($redirect) {
                            $this->showPasswordExpiryScreen();
                        }
                        return false;
                    }
                }

                // user is valid
                $this->_username = $row['UserID'];

                $DB->insert('user_login_history', $setArray);
                return true;
            }
        }

        // bad usename or password
        $this->_lastError = "Incorrect email or password";
        return false;
    }

    /**
     * Sets the session data (State object)
     *
     * @return void
     * @access public
     */
    function setState()
    {
        // someone is logged in
        $this->_isLoggedIn = true;

        // create the session managed State object
        if (!isset($_SESSION['State'])) {
            $_SESSION['State'] =& State::singleton();
        }

        $_SESSION['State']->setUsername($this->_username);
    }

    /**
     * Clears the authentication related session data (State object)
     *
     * @return void
     * @access public
     */
    function clearState()
    {
        // someone is logged in
        $this->_isLoggedIn = false;

        // create the session managed State object
        if (!isset($_SESSION['State'])) {
            $_SESSION['State'] =& State::singleton();
        }

        $_SESSION['State']->setUsername(null);
    }

     /**
      * Inserts the login (or failed-login) detail into the user_login_history
      *
      * @param String $description description for the failed-login
      * @param Array  $setArray    contains data to be inserted
      *
      * @return null
      */
    function insertFailedDetail($description, $setArray)
    {
        // create DB object
        $DB =& Database::singleton();

        $setArray['Success']     = 'N';
        $setArray['Fail_detail'] = $description;

        // in future, add mapping of error message to Failcode field enum
        $DB->insert('user_login_history', $setArray);
    }

    /**
     * Checks if a user has not logged in for a given amount of time
     * As a side-effect, if true, will also set user field 'active' to 'N'
     * Need to have already confirmed valid user
     *
     * @param String $username            username
     * @param Int    $maximumDaysInactive Most allowable days for inactivity
     *
     * @return bool - true if the user has now been disabled
     */
    function disabledDueToInactivity($username, $maximumDaysInactive)
    {
        $DB    =& Database::singleton();
        $query = "SELECT MAX(Login_timestamp) as Login_timestamp
                  FROM user_login_history 
                  WHERE UserID = :username";

        $row        = $DB->pselectRow($query, array('username' => $username));
        $last_login = new DateTime($row['Login_timestamp']);
        $time_now   = new DateTime("now");
        $interval   = $last_login->diff($time_now);

        if ($interval->days > $maximumDaysInactive) {
            $DB->update(
                'users',
                array('Active' => 'N'),
                array('UserID' => $username)
            );
            return true;
        }
        return false;
    }
}
?>
