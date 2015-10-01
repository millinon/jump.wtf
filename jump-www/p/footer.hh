<?hh
	
	function gen_footer(){

    return <footer class="footer">
		<div class="container text-center">
				<p class="text-muted">&copy; Phillip Goldfarb {date("Y")}</p>
		</div>
    </footer>;
	}


	function gen_footer_scripts(){

	return array(
	<script src={jump_config::CDN_HOST . "/h/vendor/jquery-1.11.1.min.js"}></script>,
	//<script src="//code.jquery.com/jquery-1.11.3.min.js"></script>,
	//<script src="//maxcdn.bootstrapcdn.com/bootstrap/3.3.5/js/bootstrap.min.js"></script>,
	<script src={jump_config::CDN_HOST . "/h/vendor/clipboard.js/dist/clipboard.min.js"}></script>,
    <script src={jump_config::CDN_HOST . "/h/vendor/bootstrap/dist/js/bootstrap.min.js"}></script>,
	<script async={true} src={jump_config::CDN_HOST ."/h/vendor/clippy.js/build/clippy.min.js"}></script>,
	<script async={true} src={jump_config::CDN_HOST . "/h/js/" . file_get_contents("h/js/main.js.latest")}></script>
	);
	
	}
