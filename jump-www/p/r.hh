<?hh

function heading(string $action): string {
    switch($action){
        case "del_file":
        case "del_url":
            return "Deletion successful";

        case "url_success":
            return "Link generated";

        case "file_success":
            return "File uploaded";

        case "error":
            return "Error!";
		
        default:
            header('location:./');
            die();
            return "";
        }
}

function message(string $action, ?string $url, ?string $err): mixed {
    switch($action){
        case "del_file":
            return <p>Link deleted; file deletion pending.</p>;
        case "del_url":
            return <p>Link deleted.</p>;

        case "url_success":
        case "file_success":
            return <p>Your link has been generated: <a id="newlink" href={$url} target="_blank">{$url}</a></p>;

        case "error":
            return <p>{$err}</p>;
        }
}

function r_main(): mixed{

	session_start();

	error_log('action = ' . $_SESSION['action']);
    
	$body = <body></body>;
	$body->appendChild(gen_nav());

	$con = <div class="container centered">
            <h2>{heading($_SESSION['action'])}</h2>
        </div>;

    $con->appendChild(message($_SESSION['action'], $_SESSION['new_link'], $_SESSION['problem']));
        
    if($_SESSION['action'] === 'url_success' || $_SESSION['action'] === 'file_success'){
		$con->appendChild(<button id="copybutton" class="btn btn-default" data-clipboard-text={$_SESSION['new_link']}><span class="glyphicon glyphicon-share" aria-hidden="true"></span>Copy to clipboard</button>);
    }

	for( $i = 0; $i < 4; $i++){
		$con->appendChild(<br />);
	}

	$con->appendChild(<a href=".">Go back</a>);

	$body->appendChild(<div class="jumbotron">{$con}</div>);


$body->appendChild(gen_footer());
foreach( gen_footer_scripts() as $script )
    $body->appendChild($script);
$body->appendChild(<script src={jump_config::CDN_HOST . "/h/js/" . file_get_contents("h/js/clip.js.latest")}></script>);

    echo <x:doctype><html lang="en">{gen_head()}{$body}</html></x:doctype>;
//	session_destroy();
}
