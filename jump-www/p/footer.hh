<?hh
	require_once("p/xhp.hh");
	require_once("p/aws_constants.hh");
	
	function gen_footer(){

    return <footer class="footer">
		<div class="container text-center">
				<p class="text-muted">&copy; Phillip Goldfarb {date("Y")}</p>
		</div>
    </footer>;
	}


//    echo "</div>";

	function gen_footer_scripts(){

	return array(
	<script src={aws_config::CDN_HOST . "/h/js/vendor/jquery-1.11.1.min.js"}></script>,
	<script async={true} src={aws_config::CDN_HOST ."/h/vendor/clippy.js/build/clippy.min.js"}></script>,
	<script async={true} src={aws_config::CDN_HOST . "/h/js/" . file_get_contents("h/js/main.js.latest")}></script>
	);
	
	}

//	echo_footer();
