<?hh
include('b/blackhole.hh');

require('p/header.hh');
require('p/footer.hh');

function r_main(){

session_start();

echo gen_html_tag();

echo gen_head();

$body = <body></body>;
$body->appendChild(gen_nav());

$con = <div class="container centered"></div>;

if($_SESSION['action'] == 'del_success'){
	$con->appendChild(<h2>Deletion successful</h2>);
	$con->appendChild(<p>Link deleted.</p>);
} else if($_SESSION['action'] != 'del_success') {
	if($_SESSION['action'] == 'gen_success'){
		$con->appendChild(<h1>Link Generated</h1>);
	} else {
		$con->appendChild(<h1>File Uploaded</h1>);
	}
	$con->appendChild(<p>Your link has been generated: <a href={$_SESSION['new_link']} target="_blank">{$_SESSION['new_link']}</a></p>);
	$con->appendChild(<br />);
	$con->appendChild(<button id="copybutton" class="btn btn-default" data-clipboard-text={$_SESSION["new_link"]}><span class="glyphicon glyphicon-share" aria-hidden="true"></span>Copy to clipboard</button>);
	$con->appendChild(<script src="https://cdn.jump.wtf/h/js/vendor/ZeroClipboard.min.js"></script>);
	$con->appendChild(<script src="https://cdn.jump.wtf/h/js/clip-0.min.js"></script>);
} else {
	$con->appendChild(<h1>Error!</h1>);
	$con->appendChild(<p>$_SESSION['problem']</p>);
}

for( $i = 0; $i < 4; $i++){
	$con->appendChild(<br />);
}

$con->appendChild(<a href=".">Go back</a>);

$body->appendChild(<div class="jumbotron">{$con}</div>);
/*		<h1>{
				switch($_SESSION['action']){
					case 'error':
						return "Error!";
						break;

					case 'gen_success':
						return "Link Generated";
				}
			}</h1>
		<p>{
				switch($_SESSION['action']){
					case 'error':
						$_SESSION['problem'];
						break;
					
					case gen_success:
						echo '<script src=https://cdn.jump.wtf/h/js/vendor/ZeroClipboard.min.js></script>';
						echo '<script src=https://cdn.jump.wtf/h/js/clip-0.min.js></script>';

echo  		"<div class='container-fluid'>";
echo			"<div class='row equalrow'>";
	if(isset($_SESSION['action'])){
			$action = $_SESSION['action'];
			switch($action){
			case 'error':
					echo '<h2>Error!</h2><br>';
					echo $_SESSION['problem'];
					break;
			
			case 'gen_success':
					echo '<h2>Link Generated</h2><br>';
					echo 'Your link has been generated: <a href=\'' . $_SESSION['new_link'] . '\' target=_blank>' . $_SESSION['new_link'] . '</a>';
					echo "<br><button id='copybutton' class='btn btn-default' data-clipboard-text='" . $_SESSION['new_link'] . "'><span class='glyphicon glyphicon-share' aria-hidden='true'>Copy to clipboard</button><br>";
					echo '<script src=https://cdn.jump.wtf/h/js/vendor/ZeroClipboard.min.js></script>';
					echo '<script src=https://cdn.jump.wtf/h/js/clip-0.min.js></script>';
					break;

			case 'file_success':
					echo '<h2>File Uploaded</h2><br>';
					echo 'Your file has been uploaeded to: <a href=\'' . $_SESSION['new_link'] . '\' target=_blank>' . $_SeSSION['new_link'] . '</a>';
					echo "<br><button id='copybutton' class='btn btn-default' data-clipboard-text='" . $_SESSION['new_link'] . "'><span class='glyphicon glyphicon-share' aria-hidden='true'>Copy to clipboard</button><br>";
					echo '<script src=http://cdn.jump.wtf/h/js/vendor/ZeroClipboard.min.js></script>';
					echo '<script src=http://cdn.jump.wtf/h/js/clip-0.min.js></script>';
					break;

			case 'del_success':
					break;

			default:
					break;
			}
			echo '<br><br><br><a href=\'./\'>Back</a>';
			} else {
					header('Location:./');
			}

echo "			</div></div>"; */

$body->appendChild(gen_footer());
foreach( gen_footer_scripts() as $script )
	$body->appendChild($script);

echo $body;

echo "</html>";
}

r_main();
