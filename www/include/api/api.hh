<?hh

require_once ('aws.hh');
require_once ('api/reference.hh');
require_once ('mimes.hh');

require_once ('api/validation.hh');
require_once ('api/router.hh');

class KeygenException extends Exception {
  protected $t;

  public function __construct(int $tries) {
    parent::__construct();
    $this->t = $tries;
  }

  public function __toString() {
    return 'Couldn\'t generate a key in '.$this->t.'tries';
  }
}

class jump_api {

  public static function jump_key_exists(string $key): bool {
    $dyclient = awsHelper::dydb_client();

    $res = $dyclient->query(
      [
        'TableName' => aws_config::LINK_TABLE,
        'KeyConditions' => [
          'Object ID' => [
            'AttributeValueList' => [['S' => $key]],
            'ComparisonOperator' => 'EQ',
          ],
        ],
      ],
    );

    return $res['Count'] > 0;
  }

  private static function gen_uniq_key(int $maxtries): string {
    $tries = 0;
    $new_key = '';

    do {
      $tries++;
      if ($tries > $maxtries) {
        throw new KeygenException($maxtries);
      }
      $new_key = key_config::generate_key();
    } while (self::jump_key_exists($new_key));

    return $new_key;
  }

  public static function get_mime(string $extension): string {
    return
      mimes::$mime_types[strtolower(substr($extension, 1))] ?: "application/octet-stream";
  }

  public static $doc;
  public static $cache;

  public function init(): void {
    self::$doc = api_config::api_methods();
    self::$cache = new Memcached();
    if (!self::$cache->addServer('localhost', 11211)) {
      self::$cache = NULL;
    }

  }

  public static function error(string $message, int $code = 500): array {
    return ['success' => false, 'message' => $message, 'code' => $code];
  }

  public static function success(array $data, int $code = 400): array {
    return array_merge(['success' => true, 'code' => $code], $data);
  }

  public static function genUploadURL(array $input): array {

    try {
      $input = api_validator::validate($input);
    } catch (ValidationException $ve) {
      return self::error((string) $ve, 400);
    }

    $limit = jump_config::MAX_FILE_SIZE;
    if (isset($input['promo-code'])) {
      $balance = self::getBalance(
        ['action' => 'getBalance', 'promo-code' => $input['promo-code']],
      );
      if (!$balance['success']) {
        return self::error('Invalid promo code', 400);
      } else {
        if ($balance['large-files'] > 0) {
          $limit = jump_config::PROMO_MAX_FILE_SIZE;
        }
      }
    }

    $private = $input['private'];
    $s3client = awsHelper::s3_URL_client();

    $tmp_id = uniqid('gu-', true); // is this unique enough for a temporary ID shared between (potentially) multiple servers?

    $bucket = ($private ? aws_config::PRIV_BUCKET : aws_config::PUB_BUCKET);

    $expires = (new DateTime())->modify("+5 minutes");

    try {

      $command = $s3client->getCommand(
        'PutObject',
        [
          'ACL' => 'private',
          'Body' => '',
          'Bucket' => $bucket,
          'ContentType' => 'application/octet-stream',
          'Key' => "tmp/".$tmp_id,
        ],
      );

      try {
        return [
          'success' => true,
          'URL' =>
            (string) $s3client->createPresignedRequest(
              $command,
              '+5 minutes',
            )->getURI(),
          'expires' => $expires->format(DateTime::ATOM),
          'tmp-key' => $tmp_id,
          'max-length' => $limit,
          'content-type' => 'application/octet-stream',
          'http-method' => 'PUT',
        ];
      } catch (Aws\Common\Exception\InvalidArgumentException $iae) {
        error_log($iae);
        return error('Failed to generate signed URL');
      }

    } catch (Guzzle\Common\Exception\InvalidArgumentException $iae) {
      error_log($iae);
      return error("Failed to generate command");
    }

  }

  public static function genFileURL(array $input): array {
    try {
      $input = api_validator::validate($input);
    } catch (ValidationException $ve) {
      return self::error((string) $ve, 400);
    }

    $s3client = awsHelper::s3_client();
    $dyclient = awsHelper::dydb_client();
    $glclient = awsHelper::gl_client();

    $dest_bucket =
      $input['private'] ? aws_config::PRIV_BUCKET : aws_config::PUB_BUCKET;
    $result = NULL;
    $tmp_file = NULL;
    $extension = $input['extension'] ?: '.txt';

    $size_limit = jump_config::MAX_FILE_SIZE;
    $local_limit = jump_config::MAX_LOCAL_FILE_SIZE;
    $used_promo_size = false;
    $used_promo_urls = false;

    $balance = ['success' => false, 'custom-urls' => 0, 'large-files' => 0];

    if (isset($input['promo-code'])) {
      $balance = self::getBalance(
        ['action' => 'getBalance', 'promo-code' => $input['promo-code']],
      );
      if (!$balance['success']) {
        return self::error("Invalid promo code", 400);
      } else if ($balance['large-files'] > 0) {
        $size_limit = jump_config::PROMO_MAX_FILE_SIZE;
        $local_limit = jump_config::PROMO_MAX_LOCAL_FILE_SIZE;
      }
    }

    $content_type =
      empty($input['content-type'])
        ? self::get_mime($extension)
        : $input['content-type'];

    if (isset($input['tmp-key'])) {
      // user already uploaded the file to S3
      $tmp_file = $input['tmp-key'];
    } else {

      if (isset($input['local-file'])) {

        //$tmp_file = uniqid('lf-', true);

        /* the API regex for this param should check this, but I don't want to take any chances */
        if (strpos($input['local-file'], '/') !== false) {
          return self::error(
            'Invalid local-file parameter, shenanigans detected',
            400,
          );
        }

        $tmp_file = $input['local-file'];

        if (!file_exists(jump_config::UPLOAD_DIR.'/'.$input['local-file'])) {
          return
            self::error('Invalid local-file parameter -- file not found');
        } else if (filesize(
                     jump_config::UPLOAD_DIR.'/'.$input['local-file'],
                   ) >
                   $local_limit) {
          return self::error('File too large', 400);
        }

      } else if (isset($input['file-data'])) {
        $body = base64_decode($input['file-data'], true);

        if (!$body) {
          return self::error(
            'file-data parameter invalid, should be base64 encoded',
            400,
          );
        }

        $tmp_file = uniqid('fd-', true);

        file_put_contents(jump_config::UPLOAD_DIR.'/'.$tmp_file, $body);

      } else {
        return self::error("Constraint verification failed", 400);
      }

      $file = jump_config::UPLOAD_DIR.'/'.$tmp_file;
      $size = filesize($file);

      if (!file_exists($file)) {
        return self::error('Problem with local file', 500);
      } else if ($size > jump_config::MAX_LOCAL_FILE_SIZE) {
        if ($size <= $local_limit) {
          $used_promo_size = TRUE;
        } else {
          unlink($file);
          return self::error('File too large', 400);
        }
      }

      try {
        $result = $s3client->putObject(
          [
            'Bucket' => $dest_bucket,
            'ACL' => 'private',
            'Key' => 'tmp/'.$tmp_file,
            'SourceFile' => $file,
          ],
        );
      } catch (S3Exception $s3e) {
        error_log('S3 file upload failed: '.(string) $s3e);
        return self::error('failed to upload file to S3', 500);
      }

      unlink($file);
    }

    try {
      $result = $s3client->headObject(
        ['Bucket' => $dest_bucket, 'Key' => 'tmp/'.$tmp_file],
      );
    } catch (S3Exception $s3e) {
      return
        self::error('File tmp/'.$tmp_file.' not found in S3 bucket', 412);
    } catch (AccessDeniedException $ade) {
      return self::error('Internal exception', 503);
    }

    if ($result['ContentLength'] > jump_config::MAX_FILE_SIZE) {
      if ($result['ContentLength'] <= $size_limit) {
        $used_promo_size = TRUE;
      } else {
        $s3client->deleteObject(
          ['Bucket' => $dest_bucket, 'Key' => 'tmp/'.$tmp_file],
        );
        return self::error('File too large', 400);
      }
    }

    if (isset($input['custom-url'])) {
      if (!isset($input['promo-code'])) {
        $s3client->deleteObject(
          ['Bucket' => $dest_bucket, 'Key' => 'tmp/'.$tmp_file],
        );
        return self::error('A custom URL requires a promo code', 403);
      } else if (!$balance['success']) {
        $s3client->deleteObject(
          ['Bucket' => $dest_bucket, 'Key' => 'tmp/'.$tmp_file],
        );
        return self::error('Invalid promo code', 400);
      } else if ($balance['custom-urls'] <= 0) {
        $s3client->deleteObject(
          ['Bucket' => $dest_bucket, 'Key' => 'tmp/'.$tmp_file],
        );
        return
          self::error('Promo code has no remaining custom-url credits', 402);
      } else {
        if (self::jump_key_exists($input['custom-url'])) {
          $s3client->deleteObject(
            ['Bucket' => $dest_bucket, 'Key' => 'tmp/'.$tmp_file],
          );
          return self::error('Key requested is not available', 409);
        } else {
          $new_key = $input['custom-url'];
          $used_promo_url = TRUE;
        }
      }
    } else {

      $new_key = '';
      try {
        $new_key = self::gen_uniq_key(3);
      } catch (KeygenException $ke) {
        error_log((string) $ke);
        return self::error((string) $ke, 503);
      }

    }

    // copy the temporary file to its new home
    try {
      $result = $s3client->copyObject(
        [
          'ACL' => $input['private'] ? 'private' : 'public-read',
          'Bucket' => $dest_bucket,
          'CopySource' => urlencode($dest_bucket.'/tmp/'.$tmp_file),
          'ContentType' => $content_type,
          'Key' => $new_key.$extension,
          'StorageClass' => 'REDUCED_REDUNDANCY',
          //'Metadata' => ['IP' => $_SERVER['REMOTE_ADDR']],
          'MetadataDirective' => 'REPLACE',
        ],
      );

    } catch (S3Exception $s3e) {
      error_log('S3 copyObject failed: '.(string) $s3e);
      return self::error('Failed to copy S3 object', 500);
    } catch (AccessDeniedException $ade) {
      return self::error('Internal exception', 503);
    }

    $pass = 'nopass';
    $salt = '';

    if (!empty($input['password'])) {
      $salt = uniqid('', true);
      $pass = hash('sha256', $input['password'].$salt);
    }

    try {
      $res = $dyclient->putItem(
        [
          'TableName' => aws_config::LINK_TABLE,
          'Item' => array_merge(
            [
              'Object ID' => ['S' => $new_key],
              'pass' => ['S' => $pass],
              'private_b' => ['BOOL' => $input['private']],
              'active_b' => ['BOOL' => true],
              'file_b' => ['BOOL' => true],
              'time' => ['S' => date(DateTime::W3C)],
              'filename' => ['S' => $new_key.$extension],
              'ext' => ['S' => $extension],
              'clicks' => ['N' => strval($input['clicks'] ?: 0)],
            ],
            (!empty($salt) ? ['salt' => ['S' => $salt]] : []),
          ),
        ],
      );
    } catch (DynamoDbException $dydbe) {
      error_log('DynamoDb died: '.(string) $dydbe);
      return self::error('Failed to create new URL', 503);
    }
    $s3client->deleteObject(['Bucket' => $dest_bucket, 'Key' => $tmp_file]);

    $cdn_host = jump_config::cdn_host();
    $base = jump_config::base_url();

    if ($used_promo_size || $used_promo_urls) {

      try {
        $dyclient->updateItem(
          [
            'TableName' => aws_config::PROMO_TABLE,
            'Key' => ['code' => ['S' => $input['promo-code']]],
            'ExpressionAttributeNames' => [
              '#cu' => 'custom-urls',
              '#lf' => 'large-files',
            ],
            'ExpressionAttributeValues' => [
              ':val1' => [
                'N' => strval(
                  $balance['custom-urls'] - ((int) $used_promo_urls),
                ),
              ],
              ':val2' => [
                'N' => strval(
                  $balance['large-files'] - ((int) $used_promo_size),
                ),
              ],
            ],
            'UpdateExpression' => 'SET #cu = :val1, #lf = :val2',
          ],
        );
      } catch (DynamoDbException $dbe) {
        error_log("Failed to decrement promo code ".$input['promo-code']);
      }
    }

    if (!$input['private'] && self::$cache !== NULL) {
      self::$cache->add($new_key, jump_config::FILE_HOST.$new_key.$extension);
    }

    return self::success(
      array_merge(
        ['url' => $base.$new_key],
        $input['private']
          ? []
          : ['cdn-url' => jump_config::FILE_HOST.$new_key.$extension],
      ),
    );
  }

  public static function genURL($input): array {
    try {
      $input = api_validator::validate($input);
    } catch (ValidationException $ve) {
      return self::error((string) $ve, 400);
    }

    if (!filter_var($input['input-url'], FILTER_VALIDATE_URL)) {
      return self::error("Invalid URL detected.", 400);
    }

    //        $s3client = awsHelper::s3_client();
    $dyclient = awsHelper::dydb_client();
    //        $glclient = awsHelper::gl_client();

    $balance = ['success' => false, 'custom-urls' => 0];

    if (!empty($input['promo-code'])) {
      $balance = self::getBalance(
        ['action' => 'getBalance', 'promo-code' => $input['promo-code']],
      );
    }
    $used_promo_url = FALSE;

    if (!empty($input['custom-url'])) {
      if (empty($input['promo-code'])) {
        return self::error('A custom URL requires a promo code', 403);
      } else if (!$balance['success']) {
        return self::error('Invalid promo code', 400);
      } else if ($balance['custom-urls'] <= 0) {
        return
          self::error('Promo code has no remaining custom-url credits', 402);
      } else {
        if (self::jump_key_exists($input['custom-url'])) {
          return self::error('Key requested is not available', 412);
        } else {
          $key = $input['custom-url'];
          $used_promo_url = TRUE;
        }
      }
    } else {

      $key = "";
      try {
        $key = self::gen_uniq_key(3);
      } catch (KeygenException $ke) {
        error_log((string) $ke);
        return self::error((string) $ke, 503);
      }

    }

    $pass = 'nopass';
    $salt = '';

    if (!empty($input['password'])) {
      $salt = uniqid('', true);
      $pass = hash('sha256', $input['password'].$salt);
    }

    try {
      $result = $dyclient->putItem(
        [
          'TableName' => aws_config::LINK_TABLE,
          'Item' => array_merge(
            [
              'Object ID' => ['S' => $key],
              'url' => ['S' => $input['input-url']],
              'private_b' => ['BOOL' => $input['private']],
              'pass' => ['S' => $pass],
              'active_b' => ['BOOL' => true],
              'clicks' => ['N' => strval($input['clicks'] ?: 0)],
              'time' => ['S' => date(DateTime::W3C)],
              'file_b' => ['BOOL' => false],
            ],
            (!empty($salt) ? ['salt' => ['S' => $salt]] : []),
          ),
        ],
      );
    } catch (DynamoDbException $ex) {
      error_log('DyDb putItem(url) failed: '.(string) $ex);
      return
        self::error("Failed to generate URL. Please try again later.", 503);
    }

    if ($used_promo_url) {
      try {
        $dyclient->updateItem(
          [
            'TableName' => aws_config::PROMO_TABLE,
            'Key' => ['code' => ['S' => $input['promo-code']]],
            'ExpressionAttributeNames' => ['#cu' => 'custom-urls'],
            'ExpressionAttributeValues' => [
              ':val' => ['N' => strval($balance['custom-urls'] - 1)],
            ],
            'UpdateExpression' => 'SET #cu = :val',
          ],
        );
      } catch (DynamoDbException $dbe) {
        error_log(
          "Failed to decrement promo code ".
          $input['promo-code'].
          ": ADD custom-urls -1",
        );
      }
    }

    $base = jump_config::base_url();
    return self::success(['url' => $base.$key]);
  }

  public static function delURL($input): array {

    try {
      $input = api_validator::validate($input);
    } catch (ValidationException $ve) {
      return self::error((string) $ve, 400);
    }

    $s3client = awsHelper::s3_client();
    $dyclient = awsHelper::dydb_client();
    $cfclient = awsHelper::cf_client();

    $key = "";
    $filename = "";
    $isPrivate = false;
    $check = "";
    $table_pass = "";
    $salt = "";
    $isFile = false;

    if (isset($input['jump-url'])) {
      if (!filter_var($input['jump-url'], FILTER_VALIDATE_URL)) {
        return self::error("URL validation failed", 400);
      } else {
        $toks = parse_url($input['jump-url']);
        if (!$toks) {
          return self::error("Problem with URL specified", 400);
        }
        $matches = [];
        if (!preg_match(
              "~^/(".
              key_config::extended_regex.
              ')'.
              '('.
              jump_config::extension_regex.
              ')',
              $toks['path'],
              $matches,
            )) {
          error_log("path: ".$toks['path']);
          return self::error("Invalid URL specified", 400);
        } else {
          $key = $matches[1];
        }
      }
    } else {
      $key = $input['jump-key'];
    }

    $res = $dyclient->getItem(
      [
        'TableName' => aws_config::LINK_TABLE,
        'ConsistentRead' => true,
        'Key' => ['Object ID' => ['S' => $key]],
      ],
    );

    if (!isset($res['Item'])) {
      return self::error("Key lookup failed", 404);
    }

    $item = $res['Item'];

    $table_pass = $item['pass']['S'];
    $isFile =
      isset($item['file_b']['BOOL'])
        ? boolval($item['file_b'])
        : (intval($item['isFile']['N']) === 1);
    if ($isFile) {
      $filename = $item['filename']['S'];
    }
    $isPrivate = intval($item['isPrivate']['N']) === 1;
    $salt = isset($item['salt']) ? $item['salt']['S'] : $key;

    if (hash('sha256', $input['password'].$salt) === $table_pass) {
      try {
        $dyclient->updateItem(
          [
            'TableName' => aws_config::LINK_TABLE,
            'Key' => ['Object ID' => ['S' => $key]],
            'UpdateExpression' => 'SET active_b = false',
          ],
        );
      } catch (DynamoDbException $dde) {
        error_log("Failed to delete item $key: ".(string) $dde);
        return self::error("Failed to delete item", 500);
      }

      if ($isFile) {
        $bucket =
          $isPrivate ? aws_config::PRIV_BUCKET : aws_config::PUB_BUCKET;
        try {
          $s3client->deleteObject(['Bucket' => $bucket, 'Key' => $filename]);
        } catch (S3Exception $s3e) {
          error_log("Failed to delete file $key");
        }

        if (!$isPrivate && aws_config::CF_DIST_ID !== "") {
          try {
            $cfclient->createInvalidation(
              [
                'DistributionId' => aws_config::CF_DIST_ID,
                'InvalidationBatch' =>
                  [
                    'CallerReference' =>
                      'jump.wtf-delete-'.$filename.'.'.rand(0, 8),
                    'Paths' => [
                      'Quantity' => 1,
                      'Items' => ['/'.$filename],
                    ],
                  ],
              ],
            );
          } catch (CloudfrontException $ce) {
            error_log("Failed to purge $filename");
          }
        }

        if (self::$cache !== NULL) {

          self::$cache->delete($key);

        }

        return self::success(['was-file' => true]);
      } else {
        return self::success(['was-file' => false]);
      }

      //return self::success(['note' => "Item deleted"]);
    } else {
      return self::error("Incorrect key or password", 500);
    }

    return self::error("Deletion failed", 503);
  }

  public static function jumpTo($input): array {

    try {
      $input = api_validator::validate($input);
    } catch (ValidationException $ve) {
      return self::error((string) $ve, 400);
    }

    $s3client = awsHelper::s3_client();
    $dyclient = awsHelper::dydb_client();

    $url = "";
    $expires = NULL;
    $isFile = false;

    if (isset($input['jump-key'])) {
      $key = $input['jump-key'];
    } else if (!filter_var($input['jump-url'], FILTER_VALIDATE_URL)) {
      return self::error("Invalid URL specified", 400);
    } else {
      $toks = parse_url($input['jump-url']);
      if (!$toks) {
        return self::error("Problem with URL specified", 400);
      }
      $matches = [];
      if (!preg_match(
            "~^/(".
            key_config::regex.
            ")(\\.[\\w.]{1,".
            jump_config::MAX_EXT_LENGTH.
            "})$~",
            $toks['path'],
            $matches,
          )) {
        return self::error("Invalid URL specified", 400);
      }
      $key = $matches[1];
    }

    $cached = (self::$cache == NULL) ? FALSE : self::$cache->get($key);

    if ($cached !== FALSE) {
      // cache hit

      $url = $cached;

    } else {

      $res = $dyclient->getItem(
        [
          'TableName' => aws_config::LINK_TABLE,
          'Key' => ['Object ID' => ['S' => $key]],
          'ConsistentRead' => true,
        ],
      );

      if (!isset($res['Item'])) {
        return self::error('Key not found', 404);
      }

      $item = $res['Item'];

      // I know this is bad but I didn't know dynamodb knew about bools when I started
      $isActive =
        isset($item['active_b']['BOOL'])
          ? $item['active_b']['BOOL']
          : (isset($item['active']['N'])
               ? (intval($item['active']['N']) === 1)
               : false);

      $isPrivate =
        isset($item['private_b']['BOOL'])
          ? $item['private_b']['BOOL']
          : (isset($item['isPrivate']['N'])
               ? (intval($item['isPrivate']['N']) === 1)
               : false);

      $isFile =
        isset($item['file_b']['BOOL'])
          ? $item['file_b']['BOOL']
          : (isset($item['isFile']['N'])
               ? (intval($item['isFile']['N']) === 1)
               : false); // ???

      $clicks =
        isset($item['clicks']['N']) ? intval($item['clicks']['N']) : 0;

      if (!$isActive) {
        return self::error("Link removed", 410);
      }

      if ($isFile) {
        // private file: generate a signed URL
        if ($isPrivate) {
          try {
            $command = $s3client->getCommand(
              'GetObject',
              [
                'Bucket' => aws_config::PRIV_BUCKET,
                'Key' => $item['filename']['S'],
              ],
            );
            $url = (string) $s3client->createPresignedRequest(
              $command,
              '+15 minutes',
            )->getURI();
            $expires = (new DateTime())->modify("+15 minutes");
          } catch (S3Exception $s3e) {
            return self::error("Problem fetching link", 503);
          }

          // public file: generate a CDN-backed URL
        } else {
          $url = jump_config::FILE_HOST.$item['filename']['S'];
        }

        // URL: just return the stored URL
      } else {
        $url = $item['url']['S'];
      }

      if (!$isPrivate && self::$cache !== NULL) {
        self::$cache->add($key, $url);
      }

      $promise = null;

      $kill_str = '';

      // remove stupid deprecated attributes from DynamoDB
      {
        $kill_list = [];

        if (isset($item['active'])) {
          array_push($kill_list, 'active');
        }

        if (isset($item['isPrivate'])) {
          array_push($kill_list, 'isPrivate');
        }

        if (isset($item['isFile'])) {
          array_push($kill_list, 'isFile');
        }

        if (isset($item['Checksum'])) {
          array_push($kill_list, 'Checksum');
        }

        if (isset($item['IP'])) {
          array_push($kill_list, 'IP');
        }

        if (isset($item['origname'])) {
          array_push($kill_list, 'origname');
        }

        if (isset($item['hits'])) {
          array_push($kill_list, 'hits');
        }

        if (count($kill_list) > 0) {
          $kill_str = 'REMOVE '.implode(', ', $kill_list);
        }
      }

      // TODO: only do updateItem if we need to change clicks or active
      // for now, I want to replace the dumb is* keys with booleans
      try {
        $dyclient->updateItem(
          [
            'TableName' => aws_config::LINK_TABLE,
            'Key' => ['Object ID' => ['S' => $key]],
            'ExpressionAttributeValues' => [
              ':a' => [
                'BOOL' => ($isActive && (!$isPrivate || $clicks >= 1)),
              ],
              ':c' => ['N' => strval(($isPrivate ? ($clicks - 1) : 0))],
              ':f' => ['BOOL' => (bool) $isFile],
              ':p' => ['BOOL' => (bool) $isPrivate],
            ],
            //                'ExpressionAttributeNames' => [
            //                    '#u' => 'url'
            //                    ],
            //            'ConditionExpression' => '((attribute_exists(active) AND active = 1) OR (attribute_exists(active_b) AND active_b = true)) AND ' .
            //            '((((attribute_exists(isPrivate) AND isPrivate == 1) OR (attribute_exists(private_b) AND private_b == true)) AND ' .
            //                            'clicks >= 1) OR ((attribute_exists(isPrivate) AND isPrivate == 0) or (attribute_exists(private_b) AND private_b == false)))',
            'UpdateExpression' =>
              'SET active_b = :a, clicks = :c, file_b = :f, private_b = :p '.
              $kill_str,
          ],
        );

      } catch (DynamoDbException $dde) {
        error_log('updateItem('.$key.') failed');
      }

    }

    if (isset($expires)) {
      return self::success(
        [
          'url' => $url,
          'is-file' => $isFile,
          'expires' => $expires->format(DateTime::ATOM),
        ],
      );
    } else {
      return self::success(['url' => $url, 'is-file' => $isFile]);
    }
  }

  public static function getBalance($input): array {

    try {
      $input = api_validator::validate($input);
    } catch (ValidationException $ve) {
      return self::error((string) $ve);
    }

    $dyclient = awsHelper::dydb_client();

    try {
      $res = $dyclient->getItem(
        [
          'TableName' => aws_config::PROMO_TABLE,
          'ConsistentRead' => true,
          'Key' => ['code' => ['S' => $input['promo-code']]],
        ],
      );
    } catch (DynamoDbException $dydbe) {
      return self::error('Promo code not found', 404);
    }

    if (!isset($res['Item'])) {
      return self::error("Promo code lookup failed", 404);
    }

    $large_files = intval($res['Item']['large-files']['N']);
    $custom_urls = intval($res['Item']['custom-urls']['N']);

    return self::success(
      [
        'large-files' => $large_files,
        'large-file-size' => jump_config::PROMO_MAX_FILE_SIZE,
        'custom-urls' => $custom_urls,
      ],
    );
  }

}

class apiHandler {

  private static function error(string $msg, int $code): void {
    echo json_encode(jump_api::error($msg, $code));
    die();
  }

  private static function getInput() {

    if ($_SERVER['CONTENT_TYPE'] !== 'application/json') {
      self::error('Missing or wrong application/json content type', 406);
      return NULL;
    }

    switch ($_SERVER['REQUEST_METHOD']) {

      case "GET":
        self::error('This API only supports HTTP POST requests', 405);
        return NULL;
        /*
         if(!isset($_GET["q"])){
         apiHandler::error("Missing q parameter in GET request");
         return null;
         } else {
         $json = json_decode($_GET["q"], true);
         }*/
        break;

      case "POST":
        $in = file_get_contents('php://input', 'r');
        //        echo 'input: '.$in.' --> ';
        return json_decode($in, true);
        break;

      default:
        self::error("Unknown HTTP method", 405);
        return NULL;
        break;
    }

    return NULL;
  }

  private static function help($input): void {
    $api_help = api_config::api_help();

    $topic = $input['topic'] ?: 'help'; //isset($input['topic']) ? $input['topic'] : 'help';

    if (!isset($api_help['help']['topics'][$topic])) {
      self::error("Help topic '$topic' not found", 404);
    }

    $out = "";

    $options =
      ($_SERVER['REQUEST_METHOD'] === 'GET')
        ? (JSON_PRETTY_PRINT +
           JSON_UNESCAPED_SLASHES +
           JSON_UNESCAPED_UNICODE)
        : 0;

    switch ($topic) {

      case 'all':
        $out = json_encode($api_help, $options);
        break;

      case 'constraints':
        $out =
          json_encode(
            [
              'description' => 'Specifies required parameters for a method',
              'note' =>
                'Exactly one element of each array in the constraint array must be set',
              'examples' => [
                [
                  'constraint' => [['required']],
                  'satisfied-by' => ['required'],
                ],
                [
                  'constraint' => [['option1', 'option2']],
                  'satisfied-by' => ['option1'],
                ],
                [
                  'constraint' => [
                    ['option1', 'option2'],
                    ['option2', 'option3'],
                  ],
                  'satisfied-by' => ['option2'],
                ],
                [
                  'constraint' => [
                    ['option1', 'option2'],
                    ['option2', 'option3'],
                  ],
                  'satisfied-by' => ['option1', 'option3'],
                ],
              ],
            ],
            $options,
          );
        break;

      default:
        $out = json_encode($api_help[$topic], $options);
        break;
    }

    if ($_SERVER['REQUEST_METHOD'] === 'GET') {

      foreach (array_keys($api_help['help']['topics']) as $key) {
        $out = preg_replace(
          "/\"$key\":/",
          "\"<a href=\"https://jump.wtf/a/?topic=$key\">$key</a>\":",
          $out,
        );
      }

      $out = preg_replace("/\\\\/", "", $out);

      $out = "\"$topic\": ".$out;

      echo
        "<html><head><title>jump.wtf Documentation</title></head><body><pre>$out</pre></body></html>"
      ;

    } else {
      echo $out;
    }
  }

  public static function handle(?array $input_arr = null): void {
    awsHelper::init();
    $input = NULL;
    $api_help = api_config::api_help();

    if ($input_arr !== null) {
      $input = $input_arr;
    } else if ($_SERVER['REQUEST_METHOD'] === 'GET') {

      header('Content-Type: text/html');

      if (!isset($_GET['action'])) {
        if (!isset($_GET['topic'])) {
          header('Location: /a/?action=help&topic=help');
        } else {
          header('Location: /a/?action=help&topic='.$_GET['topic']);
        }
      } else if ($_GET['action'] != 'help') {
        self::error('Please use HTTP POST for API calls', 405);
      } else {
        $input = $_GET;
      }

      if (!isset($_GET['topic'])) {
        header('Location: /a/?action=help&topic=help');
        die();
      } else if (!in_array(
                   $_GET['topic'],
                   array_keys($api_help['help']['topics']),
                 )) {
        self::error('Invalid help topic', 404);
      }

    } else if ($_SERVER['CONTENT_TYPE'] !== 'application/json') {
      self::error('Please use the application/json content type', 405);
    } else {
      header('Content-Type: application/json');
      $input = apiHandler::getInput();
    }

    if ($input === NULL) {
      apiHandler::error('Malformed JSON input', 400);
    } else if (!isset($input['action'])) {
      apiHandler::error(
        'Input missing action field, try {"action":"help"}',
        40,
      );
    } else if ($input['action'] === 'help') {
      self::help($input);
    } else {

      echo json_encode(api_router::route($input));
    }
  }
}

jump_api::init();
