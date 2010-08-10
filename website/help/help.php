<?php
header("Content-Type: text/plain");
$language = $_GET['language'];
$channel = $_GET['channel'];
$version = $_GET['version'];
$topic = str_replace('/', '', $_GET['topic']);
if(!file_exists("{$topic}.html"))
{
    header("Location: http://viewer-help.secondlife.com/{$language}/Second%20Life%20Release/{$version}/{$topic}");
}
else
{
    header("Location: /help/{$topic}");
}
?>
