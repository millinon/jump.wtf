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

  public static function error(string $message): array {
    return array('success' => false, 'message' => $message);
  }

  public static function success(array $data): array {
    return array_merge(array('success' => true), $data);
  }

  public static function genUploadURL($input): ?array {

    $private = isset($input["private"]) ? $input["private"] : false;
    $content_type =
      isset($input["content_type"]) ? $input["content_type"] : false;

    $s3client = mk_aws()->get('S3');
    $tmp_id = uniqid(); // is this unique enough for a temporary ID shared between (potentially) multiple servers?

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
          'Bucket' => "$bucket",
          'Key' => "tmp/".$tmp_id,
          'Content-Type' =>
            ($content_type == null
               ? "application/octet-stream"
               : $content_type),
          'Policy' => base64_encode(json_encode($policy)),
          'Body' => '',
        ),
      );

      try {
        return success(
          array(
            'URL' => $command->createPresignedUrl('+5 minutes'),
            'expires' => $expires->format(DateTime::ATOM),
          ),
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
    return NULL;
  }

  public static function delURL($input): ?array {
    return NULL;
  }

  public static function jumpTo($input): ?array {
    return NULL;
  }
}

class apiHandler {

  protected static $doc;

  private static function validate(string $func, array $input): array {
    // validating the API against the documentation is terrible, but I spent a bunch
    // of time on the documentation in a format that's PHP-readable, so I'll use it

    $ref = apiHandler::$doc[$func];
    $params = $ref['params'];

    foreach (array_keys($params) as $param) {

      // check if param is required, or supply the default value
      if (!isset($input[$param])) {
        if ($params[$param]['optional']) {
          $input[$param] = $params[$param]['default'];
        } else {
          throw new ValidationException("missing $param parameter");
        }
      } else {
        $val = $input[$param];

        // check param dependencies
        foreach ($params['requires-params'] as $dep) {
          $sign = substr($dep, 0, 1);
          $param2 = substr($dep, 1);

          if ($sign === '+' && !isset($input[$param2])) {
            throw new ValidationException(
              "$param requires $param2 to be set",
            );
          } else if ($sign === '-' && isset($input[$param2])) {
            throw new ValidationException(
              "only one of $param and $param2 may be specified",
            );
          }
        }

        // check param type
        if (gettype($input[$param]) !== $params[$param]['type']) {
          throw new ValidationException("bad value for $param parameter");
        }

        // check param size
        if ($params[$param]['type'] === 'int') {
          if (isset($ref[$param]['max-value']) &&
              intval($val) > $params[$param]['max-value']) {
            throw new ValidationException("bad value for $param parameter");
          } else if (isset($params[$param]['min-value']) &&
                     intval($input[$param]) < $params[$param]['min-value']) {
            throw new ValidationException("bad value for $param parameter");
          }
        } else if ($ref[$param]['type'] === 'string') {
          if (isset($params[$param]['min-length']) &&
              strlen($val) > $params[$param]['min-length']) {
            throw new ValidationException(
              "value for $param parameter out of range",
            );
          } else if (isset($params[$param]['max-length']) &&
                     strlen($val) > $params[$param]['max-length']) {
            throw new ValidationException(
              "value for $param parameter out of range",
            );
          }
        }

      }

    }

    return $input;
  }

  private static function error(string $msg): void {
    echo jump_api::error($msg);
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
        return json_decode(file_get_contents("php://input"), true);
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

    if (!isset(self::$doc[$topic])) {
      self::error("Help topic '$topic' not found");
    } else if ($topic === 'all') {
      $out = json_encode(self::$doc, JSON_PRETTY_PRINT);
    } else {
      $out = json_encode(self::$doc[$topic], JSON_PRETTY_PRINT);
    }

    if ($_SERVER['REQUEST_METHOD'] === 'GET') {

      foreach (array_keys(self::$doc) as $key) {
        error_log($key);
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
    self::$doc = api_documentation::api_doc();

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
      } else if (!in_array($_GET['topic'], array_keys(apiHandler::$doc))) {
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

      try {

        switch ($action) {

          case 'genUploadURL':
            echo
              json_encode(
                jump_api::genUploadURL(
                  apiHandler::validate('genUploadURL', $input),
                ),
              )
            ;
            break;

          case 'genFileURL':
            echo
              json_encode(
                jump_api::genFileURL(
                  apiHandler::validate('genFileURL', $input),
                ),
              )
            ;
            break;

          case 'genURL':
            echo
              json_encode(
                jump_api::genURL(apiHandler::validate('genURL', $input)),
              )
            ;
            break;

          case 'delURL':
            echo
              json_encode(
                jump_api::delURL(apiHandler::validate('genURL', $input)),
              )
            ;
            break;

          case 'jumpTo':
            echo
              json_encode(
                jump_api::jumpTo(apiHandler::validate('jumpTo', $input)),
              )
            ;
            break;

          case 'help':
            self::help($input);
            break;

          default:
            apiHandler::error('Unsupported action; try {action:"help"}');
            break;
        }

      } catch (ValidationException $ve) {
        apiHandler::error((string) $ve);
      }

    }
  }

}

