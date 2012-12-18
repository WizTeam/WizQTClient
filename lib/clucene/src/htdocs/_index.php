<?php

$url = substr($_SERVER['REQUEST_URI'], strlen($_SERVER['SCRIPT_NAME']));
$scr = substr($_SERVER['SCRIPT_URL'], strlen($_SERVER['SCRIPT_NAME']));
if ( substr($url,0,1) == '/' )
  $url = "?title=" . substr($url,1);
header("Location: https://sourceforge.net/apps/mediawiki/clucene/index.php". $url);

?>
