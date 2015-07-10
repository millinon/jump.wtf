<?hh

/*
include('b/blackhole.hh');

include('p/log.hh');

require('p/aws.hh');

irequire('p/aws_constants.hh');
require('p/key_constants.hh');
*/

function generate_key(): string{
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

				if($res['Count'] == 0){ //unique key found
					return $new_key;
				}
			}
		}

	}
	return ''; // oh god what
}

function err($s): void{
	$_SESSION['action'] = 'error';
	$_SESSION['problem'] = $s;
	header('location:r');
	exit();
}

function s_main($action): void{

	session_start();

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
				unlink('jump_config::UBASEDIR/' . $file['name']);
				err('File too large');
			}

			$filename = $file['name'];
			$filesize = $file['size'];

			$matches = null;

			if(preg_match('/((\.[a-zA-Z0-9]{2,4})|(\.[a-zA-Z])){1,4}$/',$file['name'],$matches) > 0){
				if($matches != NULL)	$ext = $matches[0];
				else $ext = '';
			} else $ext = '';

			if(! move_uploaded_file($file['tmp_name'], jump_config::UBASEDIR . '/' . $new_key . $ext)){
				err('Problem with uploaded file: ' . error_get_last()["message"]);
			}

			$res = $s3client->putObject(array(
						'Bucket' => ($clicks > 0 ? aws_config::PRIV_BUCKET : aws_config::PUB_BUCKET),
						'Key' => $new_key . $ext,
						'SourceFile' => jump_config::UBASEDIR . '/' . $new_key . $ext,
						'StorageClass' => 'REDUCED_REDUNDANCY',
						));

			$url = $res['ObjectURL'];

			if(jump_config::SAVE_BACKUP){
				$file = fopen(jump_config::UBASEDIR . '/' . $new_key . '.meta.txt', 'w');
				fwrite($file, 'ID=' . $new_key . "\n");
				fwrite($file, 'file=' . $filename . "\n");
				fwrite($file, 'size=' . $filesize . "\n");
				fwrite($file, 'IP=' . $IP. "\n");
				fwrite($file, 'time=' . time() . "\n");
				fclose($file);


				$zip = new ZipArchive();
				if($zip->open(jump_config::UBASEDIR . '/' . $new_key . '_arc.zip', ZipArchive::CREATE | ZipArchive::OVERWRITE) == TRUE){
					$zip->addFile(jump_config::UBASEDIR . '/' . $new_key . $ext, $new_key . $ext);
					$zip->addFile(jump_config::UBASEDIR . '/' . $new_key . '.meta.txt', $new_key . '.meta.txt');
					$zip->close();

					$glclient->uploadArchive(array(
								'vaultName' => aws_config::BACKUP_VAULT_NAME,
								'body' => fopen(jump_config::UBASEDIR . '/' . $new_key . '_arc.zip', 'r')
								));
				}

				unlink(jump_config::UBASEDIR . '/' . $new_key . $ext);
				unlink(jump_config::UBASEDIR . '/' . $new_key . '.meta.txt');
				unlink(jump_config::UBASEDIR . '/' . $new_key . '_arc.zip');

			}

		} else {
			$url = $_POST['new_url'];
			$ext = '';
		}

		if(strlen($url) > jump_config::MAX_URL_LEN){
			err('Invalid URL.');
			exit();
		} else if(strlen($password) > jump_config::MAX_PASS_LEN){
			err('Invalid password.');
			exit();
		} else if($clicks > jump_config::MAX_CLICKS || $clicks < -1){
			err('Invalid expiration clicks');
			exit();
		}

		$result = $dyclient->putItem(array(
					'TableName' => aws_config::LINK_TABLE,
					'Item' => array_merge(array(
							'Object ID' => array('S' => $new_key),
							'Checksum' => array('S' => md5($url)),
							'url' => array('S' => $url),
							'pass' => array('S' => $password != '' ? hash('sha256',$password . $new_key) : 'nopass'),
							'hits' => array('N' => 0),
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
		$_SESSION['new_link'] = (($type == 'url' || $clicks!=-1) ? jump_config::BASEURL : jump_config::FBASEURL) . $new_key . $ext;
		header('location:r');
	} else if($action == 'del'){
		$pass = $_POST['del_pass'];
		$key = $_POST['del_url'];

		if(strlen($key) > 20){
			err('Invalid URL.');
			exit();
		} else if(strlen($pass) > jump_config::MAX_PASS_LEN){
			err('Invalid password.');
			exit();
		}

		$it = $dyclient->getIterator('Query',array(
					'TableName' => aws_config::LINK_TABLE,
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
								'TableName' => aws_config::LINK_TABLE,
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
						if(jump_config::SAVE_BACKUP){
							$s3client->deleteObject(array(
										'Bucket' => ($item['isPrivate']['N'] == 1 ? aws_config::PRIV_BUCKET : aws_config::PUB_BUCKET),
										'Key' => $item['filename']['S']));
						}
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

/*
if(!isset($_POST['action'])){
	header('Location:./');
	exit();
} else s_main($_POST['action']);
*/
