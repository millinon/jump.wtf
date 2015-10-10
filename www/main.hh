<?hh

include('blackhole/blackhole.hh');

require_once('vendor/facebook/xhp-lib/init.php');

require_once('header.hh');
require_once('forms.hh');
require_once('footer.hh');

require_once('aws.hh');

require_once('config/aws_config.hh');
require_once('config/jump_config.hh');
require_once('config/key_config.hh');

require('index.hh');
require('go.hh');
require('submit.hh');
require('result.hh');
require('error.hh');


function main(): void{

	$matches = array();

	$uri = "";


	if(! preg_match("|/main.hh/([^/]*)|", $_SERVER['PHP_SELF'], $matches) ){
		i_main();
	} else {
		$uri = $matches[1];
	}
	
	error_log("'" . $_SERVER['PHP_SELF'] . "' --> " . $uri);

    $body = null;

	if($uri === 's'){
		error_log("s_main");
		s_main($_POST['action']);
	} else if($uri === 'r') {
		error_log("r_main");
		r_main();
    } else if($uri === '404') {
        error_page(404, $uri);
	} else if($uri !== "") {
		error_log("g_main");
	    g_main($uri);
	} else if($uri === ""){
		error_log("i_main");
		i_main();
	} else {
        error_page(404, $uri);
    }
}

main();
