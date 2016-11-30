<?hh

function in_tor(): bool {
  return
    isset($_SERVER['SERVER_NAME']) &&
    preg_match('/\.onion$/', $_SERVER['SERVER_NAME']) == 1;
}

function cdn_host(): string {
  if (in_tor()) {
    return jump_config::H_CDN_HOST;
  } else {
    return jump_config::CDN_HOST;
  }
}

function base_url(): string {
  if (in_tor()) {
    return jump_config::H_BASEURL;
  } else {
    return jump_config::BASEURL;
  }
}

// return true if it's a bad url
function filter_url(string $url): bool {

  $parsed = parse_url($url);

  if (empty($parsed['host']) ||
      (!empty(jump_config::$banned_hosts) &&
       preg_match(
         // this is just a regex of all the hostnames
         '/'.implode('|', jump_config::$banned_hosts).'/i',
         // this is the URL's hostname
         $parsed['host'],
       ) == 1)) {
    return true;
  } else if (!empty(jump_config::$banned_terms) &&
             preg_match(
               '/'.implode('|', jump_config::$banned_terms).'/i',
               $url,
             ) == 1) {
    return true;
  } else {
    return false;
  }
}
