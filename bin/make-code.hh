#!/usr/bin/env hhvm
<?hh

set_include_path(dirname(__FILE__).'/../www/include');

require ('api/api.hh');

if (!isset($argv)) {
  echo ("Run this from the command line");
  die();
} else if ($argc < 2 || $argc > 4) {
  echo
    ("Usage: \$ hhvm".__FILE__." PROMO_CODE #custom_urls=1 #large_files=1")
  ;
  die();
}

$code = $argv[1];

$urls = $argv[2] ?? 1;
$files = $argv[3] ?? 1;

awsHelper::init();

$dyclient = awsHelper::dydb_client();

try {
  $dyclient->putItem(
    [
      'TableName' => aws_config::PROMO_TABLE,
      'Item' => [
        'code' => ['S' => $code],
        'custom-urls' => ['N' => $urls],
        'large-files' => ['N' => $files],
      ],
    ],
  );
} catch (DynamoDbException $dbe) {
  echo (string) $dbe;
  die();
}

echo "Code created";

/* vim: set ft=php ts=4 sw=4 tw=0 et :*/
