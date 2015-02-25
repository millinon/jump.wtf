<?hh include('b/blackhole.hh');
session_start();
require('p/header.hh');
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
					echo '<h2>Deletion successful</h2><br>';
					echo 'Link deleted.';
					break;

			default:
					break;
			}
			echo '<br><br><br><a href=\'./\'>Back</a>';
			} else {
					header('Location:./');
			}

echo "			</div></div>";
require('p/footer.hh');
