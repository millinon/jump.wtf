<?hh

require_once ('api.hh');

function err($s): void {
  $_SESSION['action'] = 'error';
  $_SESSION['message'] = $s;
  header('location:r');
  die();
}

function s_main(): void {
  awsHelper::init();

  $result = ['success' => false, 'message' => 'Invalid request?'];

  $private = isset($_POST['expires']) ? $_POST['expires'] : false;
  $clicks = isset($_POST['clicks']) ? intval($_POST['clicks']) : 1;
  $pass = isset($_POST['password']) ? $_POST['password'] : '';
  $inurl = isset($_POST['new-url']) ? $_POST['new-url'] : '';
  $delurl = isset($_POST['del-url']) ? $_POST['del-url'] : '';

  $action = 'failure';

  if (!isset($_POST['action'])) {
    err("Invalid request!");
  } else if ($_POST['action'] === 'new-url') {
    $result = jump_api::genURL(
      array_merge(
        ['action' => 'genURL', 'input-url' => $inurl],
        $pass !== '' ? ['password' => $pass] : [],
        $private ? ['private' => true, 'clicks' => $clicks] : [],
      ),
    );

    $action = 'url_success';
  } else if ($_POST['action'] === 'new-file') {
    $tmp_name = uniqid('lf-', true);

    if (!isset($_FILES['input-file'])) {
      error_log(print_r($_FILES, TRUE));
      err('Invalid request!!!');
    }

    $file = $_FILES['input-file'];

    if (!move_uploaded_file(
          $file['tmp_name'],
          jump_config::UPLOAD_DIR.'/'.$tmp_name,
        )) {
      error_log("move_uploaded_file failed");
      err('Problem with uploaded file');
    }

    if ($file['size'] > jump_config::MAX_LOCAL_FILE_SIZE) {
      unlink(jump_config::UPLOAD_DIR.'/'.$tmp_name);
      err('File too large.');
    }

    $matches = null;
    $ext = '';

    // extract the original filename and the extension
    if (preg_match('/(\.\w+)+$/', $file['name'], $matches) > 0) {
      if ($matches !== NULL) {
        $ext = $matches[0];
      } else {
        $ext = '.txt';
      }
    } else {
      $ext = '.txt';
    }

    $result = jump_api::genFileURL(
      array_merge(
        [
          'action' => 'genFileURL',
          'local-file' => $tmp_name,
          'extension' => $ext,
        ],
        $pass !== '' ? ['password' => $pass] : [],
        $private ? ['private' => true, 'clicks' => $clicks] : [],
      ),
    );

    $action = 'file_success';

    unlink(jump_config::UPLOAD_DIR.'/'.$tmp_name);

  } else if ($_POST['action'] === 'del-url') {
    if (filter_var($delurl, FILTER_VALIDATE_URL)) {
      $result =
        jump_api::delURL(['action' => 'delURL', 'jump-url' => $delurl]);
    } else {
      $result =
        jump_api::delURL(['action' => 'delURL', 'jump-key' => $delurl]);
    }

    $action = 'del_success';

  } else {
    err("Invalid action");
  }

  if ($result['success'] !== true) {
    err($result['message']);
  } else {

    session_start();

    session_regenerate_id(true);

    session_unset();

    $_SESSION['action'] = $action;

    if (isset($result['url'])) {
      $_SESSION['url'] = $result['url'];
    }

    if (isset($result['cdn-url'])) {
      $_SESSION['cdn-url'] = $result['cdn-url'];
    }

    header('location:r');
  }
}
