<?hh

include('b/blackhole.hh');

require_once('p/xhp.hh');

require_once('p/header.hh');
require_once('p/forms.hh');
require_once('p/footer.hh');

require_once('p/aws.hh');
require_once('p/aws_constants.hh');
require_once('p/key_constants.hh');

require('p/i.hh');
require('p/g.hh');
require('p/s.hh');
require('p/r.hh');


function main(){

	$matches = array();

	$uri = "";


	//if(! preg_match("|^/([a-z])(\.hh)?(/(.*))?|", $_SERVER['PHP_SELF'], $matches) ){
	if(! preg_match("|^/main.hh/(.*)|", $_SERVER['PHP_SELF'], $matches) ){
		i_main();
	} else {
		$uri = $matches[1];
	}
	
	error_log("'" . $_SERVER['PHP_SELF'] . "' --> " . $uri);

	if($uri == 's'){
		error_log("s_main");
		s_main($_POST['action']);
	} else if($uri == 'r') { //isset($_SESSION['action'])){
		error_log("r_main");
		r_main();
	} else if($uri != ""){
		error_log("g_main");
		g_main($uri);
	} else {
		error_log("i_main");
		i_main();
	}
}

main();
