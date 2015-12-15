<?hh

function g_main(string $uri): void {

  if (strpos($uri, '/') !== false) {
    error_page(404, $uri);
  }

  $uri = explode(".", $uri)[0];

  $aws = mk_aws();
  $dyclient = $aws->get('DynamoDb');
  $s3client = $aws->get('S3');

  $it = $dyclient->getIterator(
    'Query',
    array(
      'TableName' => aws_config::LINK_TABLE,
      'ConsistentRead' => true,
      'KeyConditions' => array(
        'Object ID' => array(
          'AttributeValueList' => array(array('S' => $uri)),
          'ComparisonOperator' => 'EQ',
        ),
      ),
    ),
  );

  if (iterator_count($it) !== 1) {

    error_page(404, $uri);
    exit();

  } else {

    foreach ($it as $item) {
      if ($item['active']['N'] == 0 || $item['clicks']['N'] == 0) {
        error_page(404, $uri);
        exit();
      }

      if ($item['isFile']['N'] == 1) {
        if ($item['clicks']['N'] > 0) {
          $s = $s3client->getObjectUrl(
            aws_config::PRIV_BUCKET,
            $item['filename']['S'],
            '+15 minutes',
          );
        } else {
          $s = jump_config::FILE_HOST . $item['filename']['S'];
        }
        header('Location:'.$s);
      } else {
        header('Location:'.$item['url']['S']);
      }

      if ($item['clicks']['N'] > 0) {
        $dyclient->updateItem(
          array(
            'TableName' => aws_config::LINK_TABLE,
            'Key' => array(
              'Object ID' => array('S' => $item['Object ID']['S']),
              'Checksum' => array('S' => $item['Checksum']['S']),
            ),
            'AttributeUpdates' => array(
              'clicks' => array(
                'Action' => 'ADD',
                'Value' => array('N' => '-1'),
              ),
            ),
          ),
        );
      }

    }
  }
}
