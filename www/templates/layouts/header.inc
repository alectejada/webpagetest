<?php

declare(strict_types=1);
// Copyright 2020 Catchpoint Systems Inc.
// Use of this source code is governed by the Polyform Shield 1.0.0 license that can be
// found in the LICENSE.md file.
require_once __DIR__ . '/../../common.inc';

use WebPageTest\Util;

require_once __DIR__ . '/../../page_data.inc';
require_once __DIR__ . '/../../include/TestInfo.php';
require_once __DIR__ . '/../../include/TestResults.php';
require_once __DIR__ . '/../../include/TestRunResults.php';



if(isset($testPath) ) {
  $testInfo = TestInfo::fromFiles($testPath);
  $testResults = TestResults::fromFiles($testInfo);
}
if (isset($testResults)) {
  $adultKeywords = array();
  if (is_file('../../settings/adult.txt'))
    $adultKeywords = file('../../settings/adult.txt', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
  $isAdult = $testResults->isAdultSite($adultKeywords);
  if ($isAdult) {
    define('ADULT_SITE', true);
    $adult_site = true;
  }
}

// For users that aren't logged in, include details about the test so it can be stored in indexdb for local history support
if ($id && isset($test) && is_array($test) &&
        isset($test['testinfo']['created']) &&
        isset($test['testinfo']['url']) &&
        !isset($user) && !isset($_COOKIE['google_email'])) {
    $history = array(
        'id' => $id,
        'url' => $test['testinfo']['url'],
        'created' => $test['testinfo']['created'],
        'location' => isset($test['testinfo']['locationText']) ? $test['testinfo']['locationText'] : '',
        'runs' => isset($test['testinfo']['runs']) ? $test['testinfo']['runs'] : 1,
        'label' => isset($test['testinfo']['label']) ? $test['testinfo']['label'] : '',
        'video' => $test['testinfo']['video'] ? true : false
    );
    $history_json = json_encode($history);
    echo "\n<script>\nconst wptTestInfo=$history_json;\n</script>\n";
}

// If $tab is null, make it an empty string for strcasecmp
$tab ??= "";

if (!defined('EMBED')) {
?>
<?php
$alert = $notification_alert ?? Util::getSetting('alert');
if ($alert) {
    echo '<div class="alert-banner">' . $alert . '</div>';
}
?>
<wpt-header>
        <header>
            <a class="wptheader_logo" href="/">
                <img src="https://webpagetest.org/images/wpt-logo.svg" alt="WebPageTest, by Catchpoint" />
            </a>
            <details class="wptheader_menu">
                <summary class="wptheader_menubtn">Menu:</summary>
                <nav>
                    <ul class="wptheader_nav">
                        <?php
        if ($id) {
            $resultUrl = "/results.php?test=$id";
            if (array_key_exists('end', $_REQUEST))
                $resultUrl .= "&end={$_REQUEST['end']}";
            elseif (FRIENDLY_URLS)
                $resultUrl = "/result/$id/";
        }
        if (!isset($navTabs) || !count($navTabs)) {
            $navTabs['Start Test'] = '/';
            if (!Util::getSetting('disableTestlog')) {
              $navTabs['Test History'] = FRIENDLY_URLS ? '/testlog/1/' : '/testlog.php?days=1';
            }

            if (is_dir('wptmonitor')) {
                $navTabs['Monitor'] = '/wptmonitor/';
            }
            $apiUrl = Util::getSetting('api_url');
            if ($apiUrl) {
                $navTabs['API'] = $apiUrl;
            }

            if (Util::getSetting('forums_url')) {
                $navTabs['Forums'] = Util::getSetting('forums_url');
            }
            $navTabs['Docs'] = 'https://docs.webpagetest.org/';
            $blogUrl = Util::getSetting('blog_url');
            if ($blogUrl) {
                $navTabs['Blog'] = $blogUrl;
            }

            $navTabs['About'] = FRIENDLY_URLS ? '/about' : '/about.php';

        }
        foreach ($navTabs as $tabName => $tabUrl) {
            // make sure we have a test result to navigate to
            if (strlen($tabUrl)) {
                // highlight the current tab
                $target = '';
                $current = '';
                $opens = '';
                if( !strcasecmp($tabName, $tab) )
                    $current = ' class="wptheader-current" tabindex="-1"';
                if (substr($tabUrl, 0, 4) == 'http' && $tabName != 'API') {
                    $target = ' target="_blank" rel="noopener"';
                    $opens = ' (opens in a new tab)';
                }
                if ($opens != '') {
                    echo "<li><a $current title=\"$tabName$opens\" href=\"$tabUrl\"$target><span>$tabName</span></a></li>";
                } else {
                    echo "<li><a $current href=\"$tabUrl\"$target><span>$tabName</span></a></li>";
                }
            }
        }
?>
                    </ul>
                    <ul class="wptheader_acct">

                    <?php

if ($supportsAuth && !defined('EMBED')) {
    if ($supportsCPAuth) {
        $is_logged_in = isset($request_context) && !is_null($request_context->getUser()) && !is_null($request_context->getUser()->getAccessToken());
        if ($is_logged_in) {
          echo "<li><a href='/account'>My Account</a></li>";
          $logoutBtn =
<<<HTML
<li>
<form method='POST' action='/logout' class='logout-form'>
<button type='submit'>Logout</button>
<input type="hidden" name="csrf_token" value="{$_SESSION['csrf_token']}" />
</form>
</li>
HTML;
          echo $logoutBtn;
        } else {
          echo '<li><a href="/login">Login</a></li>';
          echo "<li><a href='/signup'>Sign-up</a></li>";
        }
    } else if ($supportsSaml) {
    if (isset($USER_EMAIL) && Util::getSetting('saml_account')) {
        $saml_account = Util::getSetting('saml_account');
        $saml_logout = Util::getSetting('saml_logout');
        if ($saml_logout) {
            $saml_logout = htmlspecialchars($saml_logout);
            $logoutUrl = "javascript:wptLogout('$saml_logout');";
        } else {
            $logoutUrl = "javascript:wptLogout();";
        }
        echo "<li><a href=\"$saml_account\">My Account</a></li><li><a href=\"$logoutUrl\">Logout</a></li>";
    } else {
    echo '<li><a href="/saml/login.php">Login</a></li>';
    $register = Util::getSetting('saml_register');
    if ($register) {
        echo "<li><a href='$register' onclick=\"try{if(_gaq!=undefined){_gaq.push(['_trackEvent','Outbound','Click','Signup']);}}catch(err){}\">Sign-up</a></li>";
    }
}
} elseif( isset($user) ) {
$logoutUrl = 'https://www.webpagetest.org/forums/member.php?action=logout';
echo "<li>Welcome, " . htmlspecialchars($user) . "</li><li><a href=\"$logoutUrl\">Logout</a></li>";
} else if( isset($_COOKIE['google_email']) && isset($_COOKIE['google_id']) ) {
$logoutUrl = 'javascript:wptLogout();';
$google_email = htmlspecialchars($_COOKIE['google_email']);
echo "<li>Welcome, $google_email </li><li><a href=\"$logoutUrl\">Logout</a></li>";
} else if (Util::getSetting('google_oauth_client_id') && Util::getSetting('google_oauth_client_secret')) {
echo '<li><a href="/oauth/login.php">Login with Google</a></li>';
}
}
?>
                        
                    </ul>
                </nav>
            </details>
        </header>
    </wpt-header>

<?php
} // EMBED
?>