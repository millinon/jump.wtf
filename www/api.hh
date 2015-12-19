<?hh

require('api_documentation.hh');

class jump_api {

    private static function error(string $message): array {
        return array('success' => false, 'message' => $message);
    }

    public static function success(array $data): array {
        return array_merge(array('success' => true), $data);
    }

    public static function getUploadURL($input): ?array {

        $private = isset($input["private"]) ? $input["private"] : false;
        $content_type = isset($input["content_type"]) ? $input["content_type"] : false;

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
                array("content-length-range", 0, jump_config::MAX_FILE_SIZE)
            ) // /conditions
        ); // /policy


        try {

            $command = $s3client->getCommand('PutObject', array(
                'Bucket' => "$bucket",
                'Key' => "tmp/" . $tmp_id,
                'Content-Type' => ($content_type == null ? "application/octet-stream" : $content_type),
                'Policy' => base64_encode(json_encode($policy)),
                'Body' => ''
            ));

            try {
                return success(array('URL' => $command->createPresignedUrl('+5 minutes'), 'expires' => $expires->format(DateTime::ATOM)));
            } catch(Aws\Common\Exception\InvalidArgumentException $iae){
                error_log($iae);
                return error('Failed to generate signed URL');
            }

        } catch(Guzzle\Common\Exception\InvalidArgumentException $iae){
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
    
    private static function error(string $msg): void{
        echo json_encode(array("success" => false, "message" => "$msg"));
        die();
    }

    private static function getInput() {

        if($_SERVER['CONTENT_TYPE'] !== 'application/json'){
            self::error('Missing application/json content type');
            return NULL;
        }
        
        switch($_SERVER['REQUEST_METHOD']){

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
                $json = json_decode(file_get_contents("php://input"), true);
            break;

        default:
            apiHandler::error("Unknown HTTP method");
            break;
        }

        return NULL;
    }

    private static function help($input): void {
        $topic = isset($input['topic']) ? $input['topic'] : 'help';

        $doc = api_documentation::api_doc();

        $out = NULL;

        if(!isset($doc[$topic])){
            self::error("Help topic '$topic' not found");
        } else if($topic === 'all'){
            $out = json_encode($doc, JSON_PRETTY_PRINT);
        } else {
            $out = json_encode($doc[$topic], JSON_PRETTY_PRINT);
        }

        if($_SERVER['REQUEST_METHOD'] === 'GET'){

            foreach(array_keys($doc) as $key){
                error_log($key);
                $out = preg_replace("/$key/", "<a href=\"https://jump.wtf/a/?topic=$key\">$key</a>",$out);
            }

            $out = preg_replace("/\\\\/", "", $out);
            //$out = preg_replace("/\n/", "<br />", $out);

            echo "<html><head><title>jump.wtf Documentation</title></head><body><pre>$out</pre></body></html>";

        } else {
            echo $out;
        }
    }

    public static function handle(): void {
        $api_doc = api_documentation::api_doc();

        $input = NULL;

        if ($_SERVER['REQUEST_METHOD'] === 'GET'){

            header('Content-Type: text/html');

            if(!isset($_GET['action'])){
                if(!isset($_GET['topic'])){
                    header('Location: /a/?action=help&topic=help');
                } else {
                    header('Location: /a/?action=help&topic='.$_GET['topic']);
                }
            } else if($_GET['action'] != 'help'){
                self::error('Please use HTTP POST for API calls');
            } else {
                $input = $_GET;
            }

            if(!isset($_GET['topic'])){
                header('Location: /a/?action=help&topic=help');
                die();
            } else if(!in_array($_GET['topic'], array_keys($api_doc))){
                self::error('Invalid help topic');
            }

        } else if ($_SERVER['CONTENT_TYPE'] !== 'application/json'){
            self::error('Please use the application/json content type');
        } else {
            header('Content-Type: application/json');
            $input = apiHandler::getInput();
        }


        if($input === NULL){
            apiHandler::error('Malformed JSON input');
        } else if(!isset($input['action'])){
            apiHandler::error('Input missing action field, try {action:"help"}');
        } else {

            $action = $input["action"];

            switch($action){

            case 'getUploadURL':
                echo json_encode(jump_api::getUploadURL($input));
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

