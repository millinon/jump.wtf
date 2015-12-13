<?hh

class jump_api {

    public static function genUploadURL($input): ?array {

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
                return array( "success" => true, "URL" => $command->createPresignedUrl("+5 minutes"), "exipires" => $expires->format(DateTime::ATOM));
            } catch(Aws\Common\Exception\InvalidArgumentException $iae){
                error_log($iae);
                return array( "success" => false, "message" => "Failed to generate signed URL");
            }

        } catch(Guzzle\Common\Exception\InvalidArgumentException $iae){
            error_log($iae);
            return array( "success" => false, "message" => "Failed to generate command");
        }

    }
}

class apiHandler {

    private static function error(string $msg){
        echo json_encode(array("success" => false, "message" => "$msg"));
        die();
    }

    private static function getInput()/*: ?KeyedContainer */{
        switch($_SERVER["REQUEST_METHOD"]){

        case "GET":
            if(!isset($_GET["q"])){
                apiHandler::error("Missing q parameter in GET request");
                return null;
            } else return json_decode($_GET["q"], true);
            break;

        case "POST":
            return json_decode(file_get_contents("php://input"), true);
            break;

        default:
            apiHandler::error("Unknown HTTP method");
            return null;
            break;
        }
    }

    public static function handle(): void {
        header('Content-Type: application/json');

        $input = apiHandler::getInput();

        if($input === null){
            apiHandler::error("Malformed JSON input");
        } else if(!isset($input["action"])){
            apiHandler::error("Input missing action field");
        } else {

             $action = $input["action"];

             switch($action){

             case "genUploadURL":
                 //echo json_encode(jump_api::genUploadURL($input));
                 apiHandler::error("go away");
                 break;

             default:
                 apiHandler::error("Unsupported action; supported actions: genUploadURL");
                 break;
             }

         }
    }

}

