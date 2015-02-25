<?php require("p/constants.hh"); ?>
		<hr>
    <footer>
				<p>&copy; Phillip Goldfarb 2015</p>
                                <?php if(defined('HHVM_VERSION')) echo "<p>+HHVM</p>"; ?>                               
    </footer>
    </div> <!-- /container -->
		<script src=<?php echo $CDN_HOST; ?>/h/js/vendor/jquery-1.11.1.min.js></script>
		<script src=https://code.jquery.com/jquery-1.11.1.min.js></script>
		<script async src=<?php echo $CDN_HOST; ?>/h/js/clippy.min.js></script>
		<script async src=<?php echo $CDN_HOST . "/h/js/" . file_get_contents("h/js/main.js.latest"); ?>
></script>
		</body>
</html>
