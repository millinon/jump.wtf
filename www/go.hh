<?hh

include_once('blackhole/blackhole.hh');

require_once ('api.hh');
require_once ('error.hh');

function g_main(string $uri): void {

  awsHelper::init();

  error_log('/g ' . $uri);

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
