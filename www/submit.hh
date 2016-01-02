<?hh

function err($s): void {
    $_SESSION['action'] = 'error';
    $_SESSION['message'] = $s;
    header('location:r');
    die();
}

function s_main($action): void {

    session_start();

    $result = ['success' => false, 'message' => 'Invalid request'];

    if ($action === 'new') {
        error_log('action = new');
        $url = "";
        $ext = "";

        $expires = isset($_POST['expires']) ? $_POST['expires'] : false;
        $clicks = $expires ? max($_POST['clicks'], 1) : -1;
        $password = $_POST['pass'];
        $IP = $_SERVER['REMOTE_ADDR'];
        $type = $_POST['sub_type'];

        if ($type === 'file') {

            $tmp_name = uniqid('', true);

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
                        $ext = '.txt';
            } else
                $ext = '.txt';

            if (!move_uploaded_file(
                $file['tmp_name'],
                jump_config::UBASEDIR.'/'.$tmp_name,
            )) {
                error_log("move_uploaded_file failed");
                err('Problem with uploaded file');
            }

            if($expires){
                $result = jump_api::genFileURL(['action' => 'genFileURL', 'local-file' => $tmp_name, 'extension'=>$ext, 'private' => true, 'clicks' => $clicks, 'password' => $password]);
            } else {
                $result = jump_api::genFileURL(['action' => 'genFileURL', 'local-file' => $tmp_name, 'extension'=>$ext, 'password' => $password]);
            }

            $_SESSION['action'] = 'file_success';

            unlink(jump_config::UBASEDIR.'/'.$tmp_name);

            

        }  else if($type === 'url'){
            $url = $_POST['new_url'];
            
            if($expires){
                $result = jump_api::genURL(['action' => 'genURL', 'input-url' => $url, 'private' => true, 'clicks' => $clicks, 'password' => $password]);
            } else {
                $result = jump_api::genURL(['action' => 'genURL', 'input-url' => $url, 'password' => $password]);
            }

            $_SESSION['action'] = 'url_success';

        } else {
            err('Invalid request');
        }
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

        $result = jump_api::delURL(['action'=>'delURL', 'jump-url'=>$key, 'password'=>$pass]);

        if(!$result['success']){
            error_log($result['message']);
            err('Failed to delete URL');
        } else {
            // deletion successful
            if ($isFile) {
                $_SESSION['action'] = 'del_file';
            } else {
                $_SESSION['action'] = 'del_url';
            }

        }
    } else {
        err('Invalid request');
    }
    
    if(! $result['success']){
        err($result['message']);
    } else {
        $_SESSION['message'] = $result['url'];
        header('location:r');
    }
}
