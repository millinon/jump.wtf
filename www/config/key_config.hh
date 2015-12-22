<?hh

class key_config {

    const MIN_LENGTH = 3;
    const MAX_LENGTH = 5;

    const length = 4;
    const regex = '[A-Za-z0-9]{3,4}';
  static function gen_charset() {
    return array_merge(range('a','z'),
        array_merge(range('A','Z'),
        range('0','9')));
  }

  // You can use words instead of characters, resulting in longer but valid keys,
  // as long as no element of the charset is a prefix of another.
  // I'm using single characters since I want to generate short links

  static function generate_key(): string {
    $dyclient = mk_aws()->get('DynamoDb');

    $MAX_TRIES = 3;
    $key_charset = key_config::gen_charset();

    for ($i = 0; $i < $MAX_TRIES; $i++) {
      $new_key = '';
      for ($j = 0; $j < key_config::length; $j++) {
        $new_key .= $key_charset[array_rand($key_charset, 1)];
      }

      $res = $dyclient->query(
        array(
          'TableName' => aws_config::LINK_TABLE,
          'KeyConditions' => array(
            'Object ID' => array(
              'AttributeValueList' => array(array('S' => '$new_key')),
              'ComparisonOperator' => 'EQ',
            ),
          ),
        ),
      );

      if ($res['Count'] === 0) { //unique key found
        return $new_key;
      }
    }
    return ''; // oh god what
  }
}
