<?php
header("Content-Type: text/plain");
$language = $_GET['language'];
$channel = $_GET['channel'];
$version = $_GET['version'];
$topic = str_replace('/', '', $_GET['topic']);
header("HTTP/1.1 301 Moved Permanently");
if(!file_exists("{$topic}.html"))
{
    header("Location: http://viewer-help.secondlife.com/{$language}/Second%20Life%20Release/2.1.0.0/{$topic}");
}
else
{
    header("Location: /help/{$topic}");
}
?>
