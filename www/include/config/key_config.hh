<?hh

class key_config {

  const MIN_LENGTH = 3;
  const MAX_LENGTH = 20;

  const target_length = 4;

  // th`is is what generated URLs will look like
  const regex = '[\\w]{3,5}';
  // this is what requested URLs will look like
  const extended_regex = '[\\w{}()-]{3,30}';

  static function gen_charset() {
    return array_merge(
      range('a', 'z'),
      array_merge(range('A', 'Z'), range('0', '9')),
    );
  }

  // You can use words instead of characters, resulting in longer but valid keys,
  // as long as no element of the charset is a prefix of another.
  // I'm using single characters since I want to generate short links

  static function generate_key(): string {
    $key_charset = key_config::gen_charset();

    $new_key = '';
    for ($j = 0; $j < self::target_length; $j++) {
      $new_key .= $key_charset[array_rand($key_charset, 1)];
    }
    return $new_key;
  }
}
