<?hh

function check() {

$filename = 'b/blackhole.dat';
$ipaddress = $_SERVER['REMOTE_ADDR'];

if(!file_exists($filename)){
	touch($filename);
}

$fp = fopen($filename, 'r');
while ($line = fgets($fp)) {
		$u = explode(' ', $line);
		if ($u[0] == $ipaddress) die("404 file not found");
}
fclose($fp);
}

//check();
