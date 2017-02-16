#!/usr/bin/env hhvm
<?hh

set_include_path(dirname(__FILE__).'/../www/include');

require ('api/api.hh');
require ('helpers.hh');

if (!isset($argv)) {
  echo 'Run this from the command line';
  exit(1);
} else if ($argc < 2 || $argc > 3) {
  echo 'Usage: $ '.basename(__FILE__).' filename #custom_url=random'."\n";
  exit(1);
}

$file = $argv[1];

echo "Uploading file $file\n";

$ext = '.'.(pathinfo($file, PATHINFO_EXTENSION) ?: '.txt');

echo "Got extension ".$ext."\n";

if (!file_exists($file)) {
  echo "Error: file not found\n";
  exit(1);
}

  awsHelper::init();

if ($argc > 2) {

  $inuse = false;

  $key = pathinfo($argv[2], PATHINFO_FILENAME);

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

  echo "Uploading file to S3... ";

  $s3client = awsHelper::s3_client();

  try {
    $result = $s3client->putObject(
      [
        'Bucket' => aws_config::PUB_BUCKET,
        'ACL' => 'public-read',
        'Key' => $key.$ext,
        'StorageClass' => 'REDUCED_REDUNDANCY',
        'ContentType' => jump_api::get_mime($ext),
        'MetadataDirective' => 'REPLACE',
        'SourceFile' => $file,
      ],
    );

  } catch (S3Exception $s3e) {
    echo "\nS3 PutObject failed: ".(string) $s3e;
    exit(1);
  }

  echo "done\nSetting entry in table... ";

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
          'file_b' => ['BOOL' => true],
          'time' => ['S' => date(DateTime::W3C)],
          'filename' => ['S' => $key.$ext],
          'ext' => ['S' => $ext],
          'clicks' => ['N' => '0'],
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
      [
        'success' => true,
        'url' => base_url().$key,
        'cdn-url' => jump_config::FILE_HOST.$key.$ext,
      ],
      JSON_PRETTY_PRINT,
    ).
    "\n"
  ;
  exit(1);
} else if ($argc < 3) {
  $tmp = uniqid('lf-', true);
  $dest = jump_config::UPLOAD_DIR.'/'.$tmp;

  if (!copy($file, $dest)) {
    echo "Couldn't copy file to $dest";
    exit(1);
  }

  echo
    json_encode(
      api_router::route(
        [
          'action' => 'genFileURL',
          'extension' => $ext,
          'local-file' => $tmp,
        ],
      ),
      JSON_PRETTY_PRINT,
    ).
    "\n"
  ;
}

/* vim: set ft=php ts=4 sw=4 tw=0 et :*/
