<?hh

function err($s): void {
  $_SESSION['action'] = 'error';
  $_SESSION['message'] = $s;
  header('location:r');
  die();
}

function s_main($action): void {

  session_start();

  $aws = mk_aws();
  $dyclient = $aws->get('DynamoDb');
  $s3client = $aws->get('S3');
  $cfclient = $aws->get('CloudFront');
  $glclient = $aws->get('Glacier');

  if ($action === 'new') {
    error_log('action = new');
    $url = "";
    $ext = "";
    $filename = "";
    $filesize = 0;

    $expires = isset($_POST['expires']) ? $_POST['expires'] : false;
    $clicks = $expires ? max($_POST['clicks'], 1) : -1;
    $password = $_POST['pass'];
    $IP = $_SERVER['REMOTE_ADDR'];

    if (($new_key = key_config::generate_key()) === "") {
      err('Failed to generate key');
    }

    $type = $_POST['sub_type'];

    if ($type === 'file') {

      $file = $_FILES['file'];

      if ($file['size'] > jump_config::MAX_FILE_SIZE) {
        unlink('jump_config::UBASEDIR/'.$file['name']);
        err('File too large');
      }

      $filename = $file['name'];
      $filesize = $file['size'];

      $matches = null;

      // extract the original filename and the extension
      if (preg_match(
            '/((\.[a-zA-Z0-9]{2,4})|(\.[a-zA-Z])){1,4}$/',
            $file['name'],
            $matches,
          ) >
          0) {
        if ($matches !== NULL)
          $ext = $matches[0]; else
          $ext = '';
      } else
        $ext = '';

      if (!move_uploaded_file(
            $file['tmp_name'],
            jump_config::UBASEDIR.'/'.$new_key.$ext,
          )) {
        error_log("move_uploaded_file failed");
        err('Problem with uploaded file');
      }

      try {
        $result = $s3client->putObject(
          array(
            'Bucket' =>
              ($clicks > 0
                 ? aws_config::PRIV_BUCKET
                 : aws_config::PUB_BUCKET),
            'Key' => $new_key.$ext,
            'SourceFile' => jump_config::UBASEDIR.'/'.$new_key.$ext,
            'StorageClass' => 'REDUCED_REDUNDANCY',
          ),
        );
        $url = $result['ObjectURL'];
      } catch (S3EXception $ex) {
        error_log(
          "Failed to upload to S3: ".
          $ex->getMessage().
          " [".
          $ex->getAwsErrorCode().
          "]",
        );
        err("Upload failed, please try again.");
      }

      if (jump_config::SAVE_BACKUP) {

        $file = fopen(jump_config::UBASEDIR.'/'.$new_key.'.meta.txt', 'w');
        fwrite($file, 'ID='.$new_key."\n");
        fwrite($file, 'file='.$filename."\n");
        fwrite($file, 'size='.$filesize."\n");
        fwrite($file, 'IP='.$IP."\n");
        fwrite($file, 'time='.time()."\n");
        fclose($file);

        $zip = new ZipArchive();
        if ($zip->open(
              jump_config::UBASEDIR.'/'.$new_key.'_arc.zip',
              ZipArchive::CREATE | ZipArchive::OVERWRITE,
            ) ===
            TRUE) {
          $zip->addFile(
            jump_config::UBASEDIR.'/'.$new_key.$ext,
            $new_key.$ext,
          );
          $zip->addFile(
            jump_config::UBASEDIR.'/'.$new_key.'.meta.txt',
            $new_key.'.meta.txt',
          );
          $zip->close();

          try {
            $glclient->uploadArchive(
              array(
                'vaultName' => aws_config::BACKUP_VAULT_NAME,
                'body' => fopen(
                  jump_config::UBASEDIR.'/'.$new_key.'_arc.zip',
                  'r',
                ),
              ),
            );
          } catch (CloudFrontException $ex) {
            error_log(
              "Failed to upload to Glacier: ".
              $ex->getMessage().
              " [".
              $ex->getAwsErrorCode().
              "]",
            );
            err("Upload failed, please try again.");
          }

          unlink(jump_config::UBASEDIR.'/'.$new_key.'.meta.txt');
          unlink(jump_config::UBASEDIR.'/'.$new_key.'_arc.zip');

        } else {
          error_log("Failed to create zip archive");
        }
      }

      unlink(jump_config::UBASEDIR.'/'.$new_key.$ext);

    } else if ($type === 'url') {
      $url = $_POST['new_url'];
      $ext = '';
    } else {
      err('Invalid request.');
    }

    if (strlen($url) > jump_config::MAX_URL_LEN) {
      err('Invalid URL.');
    } else if (strlen($password) > jump_config::MAX_PASS_LEN) {
      err('Invalid password.');
    } else if ($clicks > jump_config::MAX_CLICKS || $clicks < -1) {
      err('Invalid expiration clicks');
    }

    try {
      $result =
        $dyclient->putItem(
          array(
            'TableName' =>
              aws_config::LINK_TABLE,
            'Item' =>
              array_merge(
                array(
                  'Object ID' => array('S' => $new_key),
                  'Checksum' => array('S' => md5($url)),
                  'url' => array('S' => $url),
                  'pass' =>
                    array(
                      'S' =>
                        $password !== ''
                          ? hash('sha256', $password.$new_key)
                          : 'nopass',
                    ),
                  'hits' => array('N' => 0),
                  'active' => array('N' => 1),
                  'clicks' => array('N' => $clicks),
                  'isPrivate' => array('N' => ($clicks > 0 ? 1 : 0)),
                  'IP' => array('S' => $IP),
                  'time' => array('S' => date(DateTime::W3C)),
                  'isFile' => array('N' => ($type === 'url' ? 0 : 1)),
                ),
                $type === 'url'
                  ? array()
                  : array(
                    'filename' => array('S' => $new_key.$ext),
                    'origname' => array('S' => $filename),
                    'ext' => array('S' => ($ext === '' ? '_' : $ext)),
                    'filesize' => array('N' => $filesize),
                  ),
              ),
          ),
        );

    } catch (DynamoDbException $ex) {
      error_log(
        "Failed to add item to table: ".
        $ex->getMessage().
        " [".
        $ex->getAwsErrorCode().
        "]",
      );
      err("Failed to generate URL. Please try again later.");
    }

    if ($type === 'url') {
      $_SESSION['action'] = 'url_success';
    } else if ($type === 'file') {
      $_SESSION['action'] = 'file_success';
    }
    $_SESSION['message'] =
      /*(($type === 'url' || $clicks !== -1)
         ? jump_config::BASEURL
         : jump_config::FBASEURL).*/
      jump_config::BASEURL . 
      $new_key .
      $ext;
    header('location:r');
  } else if ($action === 'del') {
    error_log('action = delete');

    $bucket = "";
    $key = "";
    $isPrivate = FALSE;
    $isFile = FALSE;
    $filename = "";
    $check = "";
    $table_pass = "WRONG PASSWORD";

    $pass = $_POST['del_pass'];
    $key = $_POST['del_url'];

    if (strlen($key) > key_config::length) {
      err('Invalid URL.');
    } else if (strlen($pass) > jump_config::MAX_PASS_LEN ||
               strlen($pass) === 0) {
      err('Invalid password.');
    }

    try {

      $it = $dyclient->getIterator(
        'Query',
        array(
          'TableName' => aws_config::LINK_TABLE,
          'ConsistentRead' => true,
          'KeyConditions' => array(
            'Object ID' => array(
              'AttributeValueList' => array(array('S' => $key)),
              'ComparisonOperator' => 'EQ',
            ),
          ),
        ),
      );

      if (iterator_count($it) != 1) {
        err('Incorrect key or password.');
      }

      //$item = $it->current();
      foreach ($it as $item) { // ???
        $table_pass = $item['pass']['S'];
        $isFile = $item['isFile']['N'] === 1;
        if ($isFile) {
          $filename = $item['filename']['S'];
        }
        $isPrivate = $item['isPrivate']['N'] === 1;
        $check = $item['Checksum']['S'];
      }
      error_log(hash('sha256', $pass, $key)." =?= ".$table_pass);
    } catch (DynamoDbException $ex) {
      error_log(
        "DynamoDB failed: ".
        $ex->getMessage().
        " [".
        $ex->getAwsErrorCode().
        "]",
      );
      err("Incorrect key or password.");
    }

    if (hash('sha256', $pass.$key) === $table_pass) {

      try {
        $dyclient->updateItem(
          array(
            'TableName' => aws_config::LINK_TABLE,
            'Key' => array(
              'Object ID' => array('S' => $key),
              'Checksum' => array('S' => $check),
            ),
            'AttributeUpdates' => array(
              'active' => array(
                'Action' => 'PUT',
                'Value' => array('N' => '0'),
              ),
            ),
          ),
        );
      } catch (DynamoDbException $ex) {
        error_log(
          "DynamoDB failed: ".
          $ex->getMessage().
          " [".
          $ex->getAwsErrorCode().
          "]",
        );
        err("Failed to delete link.");
      }

    } else {
      err("Incorrect key or password.");
    }

    if ($isFile) {
      $bucket = $isPrivate ? aws_config::PRIV_BUCKET : aws_config::PUB_BUCKET;
      try {
        $s3client->deleteObject(
          array('Bucket' => $bucket, 'Key' => $filename),
        );
      } catch (S3Exception $ex) {
        error_log(
          "Failed to delete file from S3: ".
          $ex->getMessage().
          " [".
          $ex->getAwsErrorCode().
          "]",
        );
        // this isn't a fatal error since updateItem must have succeeded

      }
      if ($isPrivate && aws_config::CF_DIST_ID !== "") {
        try {
          $cfclient->createInvalidation(
            array(
              'DistributionId' => aws_config::CF_DIST_ID,
              'CallerReference' =>
                'jumpwtf.delete-'.$filename.'.'.rand(0, 8),
              'Paths' => array(
                'Quantity' => 1,
                'Items' => array('/'.$filename),
              ),
            ),
          );
        } catch (CloudfrontException $ex) {
          error_log(
            "Failed to purge file from CF distribution: ".
            $ex->getMessage().
            " [".
            $ex->getAwsErrorCode().
            "]",
          );
        }
      }
    }

    // deletion successful
    if ($isFile) {
      $_SESSION['action'] = 'del_file';
    } else {
      $_SESSION['action'] = 'del_url';
    }
    header('location:r');
  } else {
    err('Invalid Request');
  }
}
