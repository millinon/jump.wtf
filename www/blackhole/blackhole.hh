<?hh


class blackhole {

    static $blacklist = ['_asterisk', 'cgi-bin', '(wp-)?admin', 'blackhole', 'phpMyAdmin', 'a2billing'];

    static $filename = 'blackhole/blackhole.dat';

    static function main(): void {

        if(!defined('BLACKHOLE') && 
            preg_match('~^/+('.implode(blackhole::$blacklist,'|').')(/.*)?~',$_SERVER['REQUEST_URI'])){
                    self::ban();
        }


        $ipaddress = $_SERVER['REMOTE_ADDR'];

      if (!file_exists(self::$filename)) {
        touch(self::$filename);
      }

      $fp = fopen(self::$filename, 'r');
      while ($line = fgets($fp)) {
        $u = explode(' ', $line);
        if ($u[0] == $ipaddress){
            self::deny();
            die();// if deny is modified
        }
      }
      fclose($fp);
      define('BLACKHOLE', true);
    }

    static function ban(){
        require_once('blackhole/index.hh');
    }

    static function deny(){
        die('404 file not found');
    }

}

if(!defined('BLACKHOLE')){
    blackhole::main();
}
