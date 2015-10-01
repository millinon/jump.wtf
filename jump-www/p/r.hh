<?hh
/*
include('b/blackhole.hh');

require('p/header.hh');
require('p/footer.hh');
*/

function r_main(): void{

	session_start();

	error_log('action = ' . $_SESSION['action']);

	echo gen_html_tag();

	echo gen_head();

	$body = <body></body>;
	$body->appendChild(gen_nav());

	$con = <div class="container centered"></div>;

	if($_SESSION['action'] === 'del_success'){
		$con->appendChild(<h2>Deletion successful</h2>);
		$con->appendChild(<p>Link deleted.</p>);
	} else if($_SESSION['action'] === 'gen_success') {
		if($_SESSION['action'] === 'gen_success'){
			$con->appendChild(<h1>Link Generated</h1>);
		} else {
			$con->appendChild(<h1>File Uploaded</h1>);
		}
		$con->appendChild(<p>Your link has been generated: <a href={$_SESSION['new_link']} target="_blank">{$_SESSION['new_link']}</a></p>);
		$con->appendChild(<br />);
		$con->appendChild(<button id="copybutton" class="btn btn-default" data-clipboard-text={$_SESSION["new_link"]}><span class="glyphicon glyphicon-share" aria-hidden="true"></span>Copy to clipboard</button>);
		$con->appendChild(<script src={jump_config::CDN_HOST . "/h/vendor/zeroclipboard/dist/ZeroClipboard.min.js"}></script>);
		$con->appendChild(<script src={jump_config::CDN_HOST . "/h/js/" . file_get_contents('h/js/clip.js.latest')}></script>);
	} else if($_SESSION['action'] === 'error'){
		$con->appendChild(<h1>Error!</h1>);
		$con->appendChild(<p>{$_SESSION['problem']}</p>);
	} else {
		header('location:./');
	}

	for( $i = 0; $i < 4; $i++){
		$con->appendChild(<br />);
	}

	$con->appendChild(<a href=".">Go back</a>);

	$body->appendChild(<div class="jumbotron">{$con}</div>);

	$body->appendChild(gen_footer());
	foreach( gen_footer_scripts() as $script )
		$body->appendChild($script);

	echo $body;

	echo "</html>";

	session_destroy();
}

//r_main();
