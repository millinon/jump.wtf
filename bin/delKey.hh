#!/usr/bin/env hhvm
<?hh

set_include_path(dirname(__FILE__).'/../www/include');

require ('api/api.hh');

if (!isset($argv)) {
  echo 'Run this from the command line';
  exit(1);
} else if ($argc !== 2) {
  echo 'Usage: $ '.basename(__FILE__).' key-to-delete'."\n";
  exit(1);
}

$key = $argv[1];

if (!jump_api::jump_key_exists($key)) {
  echo 'Error: key not found';
  exit(1);
}

awsHelper::init();

$dyclient = awsHelper::dy_client();
$s3client = awsHelper::s3_client();
$cfclient = awsHelper::cf_client();

echo 'Marking key as inactive...';

try {
  $dyclient->updateItem(
    [
      'TableName' => aws_config::LINK_TABLE,
      'Key' => ['Object ID' => ['S' => $key]],
      'UpdateExpression' => 'SET active_b = false',
    ],
  );
} catch (DynamoDbException $dde) {
  echo 'Operation failed';
  exit(1);
}

echo 'Key marked inactive';

$cache = new Memcached();
if ($cache->addServer('localhost', 11211)) {
  $cache->delete($key);
}

/* vim: set ft=php ts=4 sw=4 tw=0 et :*/
