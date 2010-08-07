<?php
// None for now.
$version = explode('.', $_GET['version']);
array_pop($version);
$version_short = implode('.', $version);
header("Location: http://wiki.secondlife.com/wiki/Release_Notes/Second_Life_Release/{$version_short}");
