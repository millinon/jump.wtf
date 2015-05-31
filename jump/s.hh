<?hh

include('b/blackhole.hh');

require('p/aws.hh');

require('p/aws_constants.hh');
require('p/key_constants.hh');

session_start();

function generate_key(){
	$dyclient = mk_aws()->get('DynamoDb');

	$MAX_TRIES = 3;	
	$key_charset = key_config::gen_charset();

	for($i = 0; $i < $MAX_TRIES; $i++){
		$new_key = '';
		for($j = 1; $j <= key_config::max_len; $j++){
			$nextChar = $key_charset[mt_rand(0,count($key_charset)-1)];
			$new_key .= $nextChar;
			if($j >= key_config::min_len){
				$res = $dyclient->query(array(
							'TableName' => 'key-list',
							'KeyConditions' => array(
								'Object ID' => array(
									'AttributeValueList' => array(
										array('S' => '$new_key')
										),
									'ComparisonOperator' => 'EQ'
									)
								)
							));

				if($res['Count'] == 0){ //unique key found
					return $new_key;
				}
			}
		}

	}
	return ''; // oh god what
}

function err($s){
	$_SESSION['action'] = 'error';
	$_SESSION['problem'] = $s;
	header('location:r');
	exit();
}

function main($action){

	$aws = mk_aws();
	$dyclient = $aws->get('DynamoDb');
	$s3client = $aws->get('S3');
	$cfclient = $aws->get('CloudFront');
	$glclient = $aws->get('Glacier');

	$filename = "";
	$filesize = 0;

	if($action == 'new'){
		$expires = isset($_POST['expires']) ? $_POST['expires'] : false;
		$clicks = $expires ? max($_POST['clicks'],1) : -1;
		$password = $_POST['pass'];
		$IP = $_SERVER['REMOTE_ADDR'];

		if(($new_key = generate_key()) == ""){
			err('Failed to generate key');			
		}

		$type = $_POST['sub_type'] == 'file' ? 'file' : 'url';

		if($type == 'file'){

			$file = $_FILES['file'];

			if($file['size'] > 8*1024*1024*10){
				unlink('aws_config::UBASEDIR/' . $file['name']);
				err('File too large');
			}

			$filename = $file['name'];
			$filesize = $file['size'];

			$matches = null;

			if(preg_match('/((\.[a-zA-Z0-9]{2,4})|(\.[a-zA-Z])){1,4}$/',$file['name'],$matches) > 0){
				if($matches != NULL)	$ext = $matches[0];
				else $ext = '';
			} else $ext = '';

			if(! move_uploaded_file($file['tmp_name'], aws_config::UBASEDIR . '/' . $new_key . $ext)){
				err('Problem with uploaded file: ' . error_get_last()["message"]);
			}

			$res = $s3client->putObject(array(
						'Bucket' => ($clicks > 0 ? aws_config::PRIVBUCKET : aws_config::PUBBUCKET),
						'Key' => $new_key . $ext,
						'SourceFile' => aws_config::UBASEDIR . '/' . $new_key . $ext,
						'StorageClass' => 'REDUCED_REDUNDANCY',
						));

			$url = $res['ObjectURL'];

			if(aws_config::SAVE_BACKUP){
				$file = fopen(aws_config::UBASEDIR . '/' . $new_key . '.meta.txt', 'w');
				fwrite($file, 'ID=' . $new_key . "\n");
				fwrite($file, 'file=' . $filename . "\n");
				fwrite($file, 'size=' . $filesize . "\n");
				fwrite($file, 'IP=' . $IP. "\n");
				fwrite($file, 'time=' . time() . "\n");
				fclose($file);


				$zip = new ZipArchive();
				if($zip->open(aws_config::UBASEDIR . '/' . $new_key . '_arc.zip', ZipArchive::CREATE | ZipArchive::OVERWRITE) == TRUE){
					$zip->addFile(aws_config::UBASEDIR . '/' . $new_key . $ext, $new_key . $ext);
					$zip->addFile(aws_config::UBASEDIR . '/' . $new_key . '.meta.txt', $new_key . '.meta.txt');
					$zip->close();

					$glclient->uploadArchive(array(
								'vaultName' => aws_config::VAULTNAME,
								'body' => fopen(aws_config::UBASEDIR . '/' . $new_key . '_arc.zip', 'r')
								));
				}

				unlink(aws_config::UBASEDIR . '/' . $new_key . $ext);
				unlink(aws_config::UBASEDIR . '/' . $new_key . '.meta.txt');
				unlink(aws_config::UBASEDIR . '/' . $new_key . '_arc.zip');

			}

		} else {
			$url = $_POST['new_url'];
			$ext = '';
		}

		if(strlen($url) > aws_config::MAXURLLEN){
			err('Invalid URL.');
			exit();
		} else if(strlen($password) > aws_config::MAXPASSLEN){
			err('Invalid password.');
			exit();
		} else if($clicks > aws_config::MAXCLICKS || $clicks < -1){
			err('Invalid expiration clicks');
			exit();
		}

		$result = $dyclient->putItem(array(
					'TableName' => 'key-list',
					'Item' => array_merge(array(
							'Object ID' => array('S' => $new_key),
							'Checksum' => array('S' => md5($url)),
							'url' => array('S' => $url),
							'pass' => array('S' => $password != '' ? hash('sha256',$password + $new_key) : 'nopass'),
							'active' => array('N' => 1),
							'clicks' => array('N' => $clicks),
							'isPrivate' => array('N' => ($clicks > 0 ? 1 : 0)),
							'IP' => array('S' => $IP),
							'time' => array('S' => date(DateTime::W3C)),
							'isFile' => array('N' => ($type == 'url' ? 0 : 1)),
							), $type == 'url' ? array() :
						array(
							'filename' => array('S' => $new_key . $ext),
							'origname' => array('S' => $filename),
							'ext' => array('S'=>($ext == '' ? '_' : $ext)),
							'filesize' => array('N' => $filesize)
						     ))
					));
		$_SESSION['action'] = 'gen_success';
		$_SESSION['new_link'] = (($type == 'url' || $clicks!=-1) ? aws_config::BASEURL : aws_config::FBASEURL) . $new_key . $ext;
		header('location:r');
	} else if($action == 'del'){
		$pass = $_POST['del_pass'];
		$key = $_POST['del_url'];

		if(strlen($key) > 20){
			err('Invalid URL.');
			exit();
		} else if(strlen($pass) > aws_config::MAXPASSLEN){
			err('Invalid password.');
			exit();
		}

		$it = $dyclient->getIterator('Query',array(
					'TableName' => 'key-list',
					'ConsistentRead' => true,
					'KeyConditions' => array(
						'Object ID' => array(
							'AttributeValueList' => array(
								array('S' => $key)
								),
							'ComparisonOperator' => 'EQ'
							))
					));

		if(iterator_count($it) != 1){
			err('Incorrect key or password.');
			exit();
		} else {
			foreach($it as $item){
				if(hash('sha256',$pass + $item['Object ID']['S']) != $item['pass']['S']){
					err('Incorrect key or password.');
					exit();
				} else {
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
									'active' => array(
										'Action' => 'PUT',
										'Value' => array(
											'N' => '0'
											)
										)
									)
								));
					if($item['isFile']['N'] == 1){
						$s3client->deleteObject(array(
									'Bucket' => ($item['isPrivate']['N'] == 1 ? aws_config::PRIVBUCKET : aws_config::PUBBUCKET),
									'Key' => $item['filename']['S']
									));
						if($item['isPrivate']['N'] == 0){
							$cfclient->createInvalidation(array(
										'DistributionId' => 'E1J94UZFGW5FEI',
										'CallerReference' => 'jumpwtf.delete-'.time() . '.' . rand(0,8),
										'Paths' => array(
											'Quantity' => 1,
											'Items' => array('/' . $item['filename']['S'])
											)));
						}
					}
					$_SESSION['action'] = 'del_success';
					header('location:r');
				}
			}
		}

	} else {
		header('Location:./');
		exit();
	}

}

if(!isset($_POST['action'])){
	header('Location:./');
	exit();
} else main($_POST['action']);
