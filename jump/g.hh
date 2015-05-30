<?hh

include('b/blackhole.hh');

require('p/aws_constants.hh');
require('p/aws.hh');


function g_main($uri){

	$aws = mk_aws();
	$dyclient = $aws->get('DynamoDb');
	$s3client = $aws->get('S3');

	$it = $dyclient->getIterator('Query',array(
				'TableName' => 'key-list',
				'ConsistentRead' => true,
				'KeyConditions' => array(
					'Object ID' => array(
						'AttributeValueList' => array(
							array('S' => $uri)
							),
						'ComparisonOperator' => 'EQ'
						))
				));

	if(iterator_count($it) != 1){
		header('Location:./');
	} else {
		foreach($it as $item){
			if($item['active']['N'] == 0 || $item['clicks']['N'] == 0){
				header('Location:.');
				exit();
			}

			if($item['isFile']['N'] == 1){
				if($item['clicks']['N'] > 0){							
					$s = $s3client->getObjectUrl(aws_config::PRIVBUCKET,$item['filename']['S'], '+15 minutes');
				} else {
					$s = aws_config::FBASEURL . $item['filename']['S'];
				}

				header('Location:' . $s);
			} else {
				header('Location:' . $item['url']['S']);
			}

			if($item['clicks']['N'] > 0){
				$dyclient->updateItem(array(
							'TableName' => 'key-list',
							'Key' => array(
								'Object ID' => array(
									'S' => $item['Object ID']['S']
									),
								'Checksum' => array(
									'S' => $item['Checksum']['S']
									)
								),
							'AttributeUpdates' => array(
								'clicks' => array(
									'Action' => 'ADD',
									'Value' => array(
										'N' => '-1'
										)
									)
								)
							));
			}
		}
	}
}

$path = explode('/', trim($_SERVER['SCRIPT_NAME'], '/'));

$uri  = explode('/', trim($_SERVER['REQUEST_URI'], '/'));

foreach ($path as $key => $val) {
	if ($val == $uri[$key]) {
		unset($uri[$key]);
	} else {
		break;
	}
}

$uri = implode('/', $uri);

$toks = explode(".", $uri);

g_main($toks[0]);
