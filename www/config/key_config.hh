<?hh

class key_config {
    const length = 4;
    static function gen_charset(){
        return array('0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z');
    }

    // No, I'm not dumb. You can use words instead of characters, resulting in longer but valid keys,
    // as long as no element of the charset is a prefix of another

    static function generate_key(): string {
        $dyclient = mk_aws()->get('DynamoDb');

        $MAX_TRIES = 3;
        $key_charset = key_config::gen_charset();

        for($i = 0; $i < $MAX_TRIES; $i++){
            $new_key = '';
            for($j = 0; $j < key_config::length; $j++){
                $new_key .= $key_charset[array_rand($key_charset, 1)];
            }

            $res = $dyclient->query(array(
                        'TableName' => aws_config::LINK_TABLE,
                        'KeyConditions' => array(
                            'Object ID' => array(
                                'AttributeValueList' => array(
                                    array('S' => '$new_key')
                                    ),
                                'ComparisonOperator' => 'EQ'
                                )
                            )
                        ));

            if($res['Count'] === 0){ //unique key found
                return $new_key;
            }
        }
        return ''; // oh god what
    }
}
