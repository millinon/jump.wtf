<?hh

require_once ('aws.hh');

function g_main(string $uri): void {

  if (strpos($uri, '/') !== false) {
    error_page(404, $uri);
  }

  $uri = explode(".", $uri)[0];

  $result = jump_api::jumpTo(['action' => 'jumpTo', 'jump-key' => $uri]);

  if ($result['success'] !== true) {
    error_log($result['message']);
    error_page($result['code'], $uri);
  } else {
    header('Location:'.$result['url']);
  }
}
