#!/usr/bin/env hhvm
<?hh

set_include_path(dirname(__FILE__).'/../www/include');

require ('api/api.hh');

if (!isset($argv)) {
  echo 'Run this from the command line';
  exit(1);
} else if ($argc !== 3) {
  echo 'Usage: $ '.basename(__FILE__).' custom_url "Text goes here"'."\n";
  exit(1);
}

// this base64-encodes the text and redirects to the data URI
// it only works in firefox at this time

$data = "data:text/plain;base64,".base64_encode($argv[2]);

awsHelper::init();

$inuse = false;

$key = $argv[1];

echo "Allocating key $key... ";

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
        'url' => ['S' => $data],
      ],
    ],
  );
  echo "done\n";
} catch (DynamoDbException $dydbe) {
  echo "\nDyDb PutItem failed: ".(string) $dydbe."\n";
  exit(1);
}

if ($inuse) {

  echo "Invalidating Cloudfront object... ";

  $cfclient = awsHelper::cf_client();

  try {
    $cfclient->createInvalidation(
      [
        'DistributionId' => aws_config::CF_DIST_ID,
        'InvalidationBatch' => [
          'CallerReference' => 'jump.wtf-putfile-'.$key.'.'.rand(0, 8),
          'Paths' => ['Quantity' => 1, 'Items' => ['/'.$key.$ext]],
        ],
      ],
    );

    echo " done\n";
  } catch (CloudfrontException $ce) {
    echo "\nCloudFront invalidation failed: ".(string) $ce."\n";
  }
}

echo
  json_encode(
    ['success' => true, 'url' => jump_config::base_url().$key],
    JSON_PRETTY_PRINT,
  ).
  "\n"
;

/* vim: set ft=php ts=4 sw=4 tw=0 et :*/
