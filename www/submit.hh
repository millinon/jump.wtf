<?hh

require_once ('api/api.hh');

function err($s): void {
  $_SESSION['action'] = 'error';
  $_SESSION['message'] = $s;
  header('location:r');
  die();
}

function s_main(): void {
  session_start();

  session_regenerate_id(true);

  session_unset();

  awsHelper::init();

  if (!isset($_POST['action'])) {
    err('Invalid request: missing action');
  }

  $result = ['success' => false, 'message' => 'Invalid request?'];

  $input = [];

  switch ($_POST['action']) {
    case 'new-url':
      $input['action'] = 'genURL';
      if (!isset($_POST['input-url']) || empty($_POST['input-url'])) {
        err('Invalid request: missing input URL');
      } else {
        $input['input-url'] = $_POST['input-url'];
      }
      break;

    case 'new-file':
      $input['action'] = 'genFileURL';
      break;

    case 'del-url':
      $input['action'] = 'delURL';
      if (!isset($_POST['del-url']) || empty($_POST['del-url'])) {
        err('Invalid request: missing input URL');
      } else {
        $input['jump-url'] = $_POST['del-url'];
      }
      if (!isset($_POST['password']) || empty($_POST['password'])) {
        err('Invalid request: missing input URL');
      }
      break;

    default:
      error_log('got invalid $_POST[\'action\'] = '.$_POST['action']);
      err('Invalid request: unknown action');
  }

  if (!empty($_POST['clicks'])) {
    $input['clicks'] = intval($_POST['clicks']);
    $input['private'] = true;
  }

  if (!empty($_POST['password'])) {
    $input['password'] = $_POST['password'];
  }

  if (!empty($_POST['promo-code'])) {
    $input['promo-code'] = $_POST['promo-code'];
  }

  if (!empty($_POST['custom-url'])) {
    if (!isset($input['promo-code'])) {
      err('Invalid request: custom URL requires promo code');
    }

    $input['custom-url'] = $_POST['custom-url'];
  }

  if ($input['action'] === 'genFileURL') {
    $tmp_name = uniqid('lf-', true);

    if (!isset($_FILES['input-file'])) {
      error_log(print_r($_FILES, TRUE));
      err('File not uploaded');
    }

    $file = $_FILES['input-file'];

    if (!move_uploaded_file(
          $file['tmp_name'],
          jump_config::UPLOAD_DIR.'/'.$tmp_name,
        )) {
      error_log("move_uploaded_file failed");
      err('Problem transferring file');
    }

    $ext = pathinfo($file['name'], PATHINFO_EXTENSION);

    if (empty($ext)) {
      //pathinfo($file['name'], PATHINFO_EXTENSION) :? '.txt'
      $ext = '.txt';
    }

    $input['extension'] = '.'.$ext;

    $input['local-file'] = $tmp_name;

  }

  $_SESSION['action'] = $input['action'];

  $result = api_router::route($input);

  $_SESSION['success'] = $result['success'];

  if (!$result['success']) {
    $_SESSION['message'] = $result['message'];
  } else {

    switch ($input['action']) {
      case 'genFileURL': //FALLTHROUGH
        if (isset($result['cdn-url'])) {
          $_SESSION['cdn-url'] = $result['cdn-url'];
        }

      case 'genURL':
          $_SESSION['url'] = $result['url'];

          if(jump_config::in_tor() && isset($result['hidden-url'])){
              $_SESSION['hidden-url'] = $result['hidden-url'];
          }
          

        break;

      case 'delURL':
        $_SESSION['was-file'] = $result['was-file'];
        break;

      default:
        err('Unknown action');
        break;
    }

  }

  header('location:r');
}
