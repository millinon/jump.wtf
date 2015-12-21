<?hh

require ('api_documentation.hh');

class ValidationException extends Exception {
  protected $msg;

  public function __construct(string $message = "") {
    parent::__construct();
    $this->msg = $message;
  }

  public function __toString() {
    return $this->msg;
  }
}

class jump_api {

  public static $doc;

  public function init(): void {
    self::$doc = api_documentation::api_doc();
  }

  public static function validate(array $input): array {
    // validating the API against the documentation is terrible, but I spent a bunch
    // of time on the documentation in a format that's PHP-readable, so I'll use it

    if (!isset($input['action'])) {
      throw new ValidationException(
        'Missing action parameter -- try {"action": "help"}',
      );
    }

    $func = $input['action'];
    if (!in_array($func, array_keys(self::$doc))) {
      throw new ValidationException("Unknown action $func");
    }

    $action_ref = jump_api::$doc[$func];
    $params = $action_ref['params'];

    foreach (array_keys($input) as $in_param) {
      if (!isset($action_ref[$in_param])) {
        if ($in_param === 'action')
          continue;
        if (!isset($action_ref['params'][$in_param])) {
          throw new ValidationException("Unknown parameter $in_param");
        }
      }

      $param_ref = $params[$in_param];

      if (isset($param_ref['requires-params'])) {
        foreach ($param_ref['requires-params'] as $dependency) {
          if (!isset($input[$dependency])) {
            throw new ValidationException(
              "Parameter $in_param requires that $dependency is set",
            );
          }
        }
      }
    }

    foreach (array_keys($params) as $param) {

      $param_ref = $params[$param];

      if (!isset($input[$param])) {
        if (isset($param_ref['default'])) {
          $input[$param] = $param_ref['default'];
          // default values have been validated in validate_api.hh
        }
      } else {
        $val = $input[$param];

        // check param type
        if (gettype($val) !== $param_ref['type']) {
          throw new ValidationException(
            "bad type for $param parameter, expected {$param_ref['type']}",
          );
        } else if ($param_ref['type'] === 'integer') {
          // check param bounds
          if (isset($param_ref['max-value']) &&
              intval($val) > $param_ref['max-value']) {
            throw new ValidationException("bad value for $param parameter");
          } else if (isset($param_ref['min-value']) &&
                     intval($input[$param]) < $param_ref['min-value']) {
            throw new ValidationException("bad value for $param parameter");
          }
        } else if ($param_ref['type'] === 'string') {

          if (isset($param_ref['regex'])) {
            if (!preg_match($param_ref['regex'], $input[$param])) {
              throw new ValidationException(
                "input $param didn't match parameter regex",
              );
            }
          }

          if (isset($param_ref['min-length']) &&
              strlen($val) < $param_ref['min-length']) {
            throw new ValidationException(
              "length for $param parameter out of range",
            );
          } else if (isset($param_ref['max-length']) &&
                     strlen($val) > $param_ref['max-length']) {
            throw new ValidationException(
              "length for $param parameter out of range",
            );
          }
        }

        if (isset($param_ref['requires-params'])) {
          // check param constraints
          foreach ($param_ref['requires-params'] as $constraint) {
            if (!isset($input[$constraint])) {
              throw new ValidationException(
                "$param requires $constraint to be set",
              );
            }
          }
        }
      }
    }

    // check for missing required parameters -- these are validated as exclusive sets
    foreach ($action_ref['required-params'] as $param_set) {
      $total = 0;
      foreach ($param_set as $param) {
        if (isset($input[$param])) {
          $total++;
        }
      }
      if ($total === 0) {
        throw new ValidationException(
          'One of ['.implode($param_set, ' ').'] must be set',
        );
      } else if ($total > 1) {
        throw new ValidationException(
          'Only one of ['.
          implode($param_set, ' ').
          "] may be set, but $total found",
        );
      }
    }

    // check for unknown parameters
    foreach (array_keys($input) as $in_param) {
    }

    // /paramarama

    return $input;
  }

  public static function error(string $message): array {
    return array('success' => false, 'message' => $message);
  }

  public static function success(array $data): array {
    return array_merge(array('success' => true), $data);
  }

  public static function genUploadURL(array $input): ?array {

    try {
      $input = self::validate($input);
    } catch (ValidationException $ve) {
      return self::error((string) $ve);
    }
    $private = $input['private'];
    $content_type = $input['content-type'];

    $s3client = mk_aws()->get('S3');
    $tmp_id = uniqid('', true); // is this unique enough for a temporary ID shared between (potentially) multiple servers?

    $bucket = ($private ? aws_config::PRIV_BUCKET : aws_config::PUB_BUCKET);

    $expires = (new DateTime())->modify("+5 minutes");

    $policy = array(
      'expiration' => $expires->format(DateTime::ATOM),
      'conditions' => array(
        array('bucket' => "$bucket"),
        array('acl' => 'private'),
        array("starts-with", "\$key", "tmp/"),
        array("content-length-range", 0, jump_config::MAX_FILE_SIZE),
      ) // /conditions
    ); // /policy

    try {

      $command = $s3client->getCommand(
        'PutObject',
        array(
          'ACL' => 'private',
          'Bucket' => "$bucket",
          'Key' => "tmp/".$tmp_id,
          'Content-Type' => $content_type,
          'Policy' => base64_encode(json_encode($policy)),
          'StorageClass' => 'REDUCED_REDUNDANCY',
          'Body' => '',
        ),
      );

      try {
        return array(
          'success' => true,
          'URL' => $command->createPresignedUrl('+5 minutes'),
          'expires' => $expires->format(DateTime::ATOM),
          'tmp-key' => $tmp_id,
          'max-length' => jump_config::MAX_FILE_SIZE,
        );
      } catch (Aws\Common\Exception\InvalidArgumentException $iae) {
        error_log($iae);
        return error('Failed to generate signed URL');
      }

    } catch (Guzzle\Common\Exception\InvalidArgumentException $iae) {
      error_log($iae);
      return error("Failed to generate command");
    }

  }

  public static function genFileURL($input): ?array {
    return NULL;
  }

  public static function genURL($input): ?array {
    try {
      $input = self::validate($input);
    } catch (ValidationException $ve) {
      return self::error((string) $ve);
    }

    if (!filter_var($input['input-url'], FILTER_VALIDATE_URL)) {
      return self::error("Invalid URL detected.");
    }

    $aws = mk_aws();
    $dyclient = $aws->get('DynamoDb');
    $s3client = $aws->get('DynamoDb');
    $glclient = $aws->get('DynamoDb');

    $key = "";

    if (($key = key_config::generate_key()) === "") {
      return self::error("Failed to generate key");
    }

    $salt = uniqid('', true);

    try {
      $result = $dyclient->putItem(
        [
          'TableName' =>
            aws_config::LINK_TABLE,
          'Item' =>
            [
              'Object ID' =>
                ['S' => $key],
              'Checksum' =>
                ['S' => md5($input['input-url'])],
              'url' =>
                ['S' => $input['input-url']],
              'isPrivate' =>
                ['N' => $input['private'] ? 1 : 0],
              'pass' =>
                [
                  'S' =>
                    $input['password'] === ""
                      ? 'nopass'
                      : hash("sha256", $input['password'].$salt),
                ],
              'salt' =>
                ['S' => $salt],
              'active' =>
                ['N' => 1],
              'clicks' =>
                ['N' => $input['clicks']],
              'time' =>
                ['S' => date(DateTime::W3C)],
              'isFile' =>
                ['N' => 0],
            ],
        ],
      );
    } catch (DynamoDbException $ex) {
      error_log('DyDb putItem(url) failed: '.(string) $ex);
      return self::error("Failed to generate URL. Please try again later.");
    }

    return self::success(['url' => 'https://jump.wtf/'.$key]);
  }

  public static function delURL($input): ?array {
    return NULL;
  }

  public static function jumpTo($input): ?array {
    return NULL;
  }
}

class apiHandler {

  private static function error(string $msg): void {
    echo json_encode(jump_api::error($msg));
    die();
  }

  private static function getInput() {

    if ($_SERVER['CONTENT_TYPE'] !== 'application/json') {
      self::error('Missing application/json content type');
      return NULL;
    }

    switch ($_SERVER['REQUEST_METHOD']) {

      case "GET":
        self::error('This API only supports HTTP POST requests');
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
        self::error("Unknown HTTP method");
        return NULL;
        break;
    }

    return NULL;
  }

  private static function help($input): void {
    $topic = isset($input['topic']) ? $input['topic'] : 'help';

    $out = NULL;

    if (!isset(jump_api::$doc['help']['topics'][$topic])) {
      self::error("Help topic '$topic' not found");
    } else if ($topic === 'all') {
      $out = json_encode(jump_api::$doc, JSON_PRETTY_PRINT);
    } else {
      $out = json_encode(jump_api::$doc[$topic], JSON_PRETTY_PRINT);
    }

    if ($_SERVER['REQUEST_METHOD'] === 'GET') {

      foreach (array_keys(jump_api::$doc) as $key) {
        $out = preg_replace(
          "/\"$key\"/",
          "\"<a href=\"https://jump.wtf/a/?topic=$key\">$key</a>\"",
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

  public static function handle(): void {
    //    self::$doc = api_documentation::api_doc();

    $input = NULL;

    if ($_SERVER['REQUEST_METHOD'] === 'GET') {

      header('Content-Type: text/html');

      if (!isset($_GET['action'])) {
        if (!isset($_GET['topic'])) {
          header('Location: /a/?action=help&topic=help');
        } else {
          header('Location: /a/?action=help&topic='.$_GET['topic']);
        }
      } else if ($_GET['action'] != 'help') {
        self::error('Please use HTTP POST for API calls');
      } else {
        $input = $_GET;
      }

      if (!isset($_GET['topic'])) {
        header('Location: /a/?action=help&topic=help');
        die();
      } else if (!in_array($_GET['topic'], array_keys(jump_api::$doc))) {
        self::error('Invalid help topic');
      }

    } else if ($_SERVER['CONTENT_TYPE'] !== 'application/json') {
      self::error('Please use the application/json content type');
    } else {
      header('Content-Type: application/json');
      $input = apiHandler::getInput();
    }

    if ($input === NULL) {
      apiHandler::error('Malformed JSON input');
    } else if (!isset($input['action'])) {
      apiHandler::error('Input missing action field, try {action:"help"}');
    } else {

      $action = $input["action"];

      switch ($action) {

        case 'genUploadURL':
          echo json_encode(jump_api::genUploadURL($input));
          break;

        case 'genFileURL':
          echo json_encode(jump_api::genFileURL($input));
          break;

        case 'genURL':
          echo json_encode(jump_api::genURL($input));
          break;

        case 'delURL':
          echo json_encode(jump_api::delURL($input));
          break;

        case 'jumpTo':
          echo json_encode(jump_api::jumpTo($input));
          break;

        case 'help':
          self::help($input);
          break;

        default:
          apiHandler::error('Unsupported action; try {action:"help"}');
          break;
      }

    }
  }

}

jump_api::init();
