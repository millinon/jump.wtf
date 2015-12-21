<?hh

class blackhole {

  static $blacklist = ['_asterisk', 'cgi-bin', '(wp-)?admin', 'blackhole'];

  static function main(): void {
    if (preg_match(
          "~^/+(".implode(self::$blacklist, '|').")(/.*)?~",
          $_SERVER['REQUEST_URI'],
        ) ===
        1) {
      require_once ('blackhole/index.hh');
    } else {
      $filename = 'blackhole/blackhole.dat';
      $ipaddress = $_SERVER['REMOTE_ADDR'];

      if (!file_exists($filename)) {
        touch($filename);
      }

      $fp = fopen($filename, 'r');
      while ($line = fgets($fp)) {
        $u = explode(' ', $line);
        if ($u[0] == $ipaddress)
          die("404 file not found");
      }
      fclose($fp);
    }
  }

}

blackhole::main();
