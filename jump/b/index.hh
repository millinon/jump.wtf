<?hh /*

Title: Blackhole for Bad Bots
Description: Automatically trap and block bots that don't obey robots.txt rules
Project URL: http://perishablepress.com/blackhole-bad-bots/
Author: Jeff Starr (aka Perishable)
Version: 2.0
License: GPLv2 or later

This program is free software; you can redistribute it and/or modify it under the 
terms of the GNU General Public License as published by the Free Software Foundation; 
either version 2 of the License, or (at your option) any later version.

		This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
		See the GNU General Public License for more details.

		Credits: The Blackhole includes customized/modified versions of these fine scripts:
		- Network Query Tool @ http://www.drunkwerks.com/docs/NetworkQueryTool/
		- Kloth.net Bot Trap @ http://www.kloth.net/internet/bottrap.php

 */

$version = '2.0';

// variables
$filename = 'blackhole.dat';
$badbot   = 0;

$request   = sanitize($_SERVER['REQUEST_URI']);
$ipaddress = sanitize($_SERVER['REMOTE_ADDR']);
$useragent = sanitize($_SERVER['HTTP_USER_AGENT']);
$protocol  = sanitize($_SERVER['SERVER_PROTOCOL']);
//$method    = sanitize($_SERVER['REQUEST_METHOD']);

// date_default_timezone_set('UTC');
//date_default_timezone_set('America/Los_Angeles');
$date = date('l, F jS Y @ H:i:s');
$time = time();

// sanitize
function sanitize($string) {
		$string = trim($string); 
		$string = strip_tags($string);
		$string = htmlspecialchars($string, ENT_QUOTES, 'UTF-8');
		$string = str_replace("\n", "", $string);
		$string = trim($string); 
		return $string;
}

// check target | bugfix
if (!$ipaddress || !preg_match("/^[\w\d\.\-]+\.[\w\d]{1,4}$/i", $ipaddress)) { 
		exit('Error: You did not specify a valid target host or IP.');
}

require('blackhole.hh');

// record hit
$fp = fopen($filename, 'a+');
fwrite($fp, "$ipaddress - $protocol - $date - $useragent - $request\n");
fclose($fp);

die('404 file not found');

exit;
