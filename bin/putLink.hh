#!/usr/bin/env hhvm
<?hh

set_include_path(dirname(__FILE__).'/../www/include');

require ('vendor/autoload.php');

require ('api/api.hh');
require ('helpers.hh');
require ('aws.hh');

$_SERVER['REMOTE_ADDR'] = 'localhost';

if (!isset($argv)) {
  echo 'Run this from the command line';
  exit(1);
} else if ($argc < 2 || $argc > 3) {
  echo 'Usage: $ '.basename(__FILE__).' input-url #custom_key=random'."\n";
  exit(1);
}

$input = $argv[1];

awsHelper::init();

$inuse = false;
$key = '';

echo "Allocating key $key... ";

if ($argc === 3) {
  $key = $argv[2];
  if (jump_api::jump_key_exists($key)) {
    $inuse = true;
    echo "\n";
    if (!preg_match(
          '/^[Yy]/',
          readline("Warning: key $key in use - replace original? (y/n) "),
        )) {
      echo "Operation canceled.\n";
      exit(1);

    }
  }
} else {
  do {
    $key = key_config::generate_key();
  } while (jump_api::jump_key_exists($key));
}

echo "Setting entry in table... ";

$dyclient = awsHelper::dydb_client();

try {
  $result = $dyclient->putItem(
    [
      'TableName' => aws_config::LINK_TABLE,
      'Item' => [
        'Object ID' => ['S' => $key],
        'pass' => ['S' => 'nopass'],
        'private_b' => ['BOOL' => false],
        'active_b' => ['BOOL' => true],
        'file_b' => ['BOOL' => false],
        'time' => ['S' => date(DateTime::W3C)],
        'clicks' => ['N' => '0'],
        'url' => ['S' => $input],
      ],
    ],
  );
  echo "done\n";
} catch (DynamoDbException $dydbe) {
  echo "\nDyDb PutItem failed: ".(string) $dydbe."\n";
  exit(1);
}

if ($inuse) {
  $cache = new Memcached();
  if ($cache->addServer('localhost', 11211)) {
    $cache->delete($key);
  }
}

echo base_url().$key."\n";

exit(0);

/* vim: set ft=php ts=4 sw=4 tw=0 et :*/
