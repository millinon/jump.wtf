<?hh
    
    function gen_footer() : mixed {
        return <footer class="footer">
                <div class="container text-center">
                    <p class="text-muted">&copy; Phillip Goldfarb {date("Y")}</p>
                </div>
            </footer>;
    }


    function gen_footer_scripts() : mixed {
        return <p>
                <script src={jump_config::CDN_HOST . "/vendor/jquery-1.11.1.min.js"}></script>
                <script src={jump_config::CDN_HOST . "/vendor/clipboard.js/dist/clipboard.min.js"}></script>
                <script src={jump_config::CDN_HOST . "/vendor/bootstrap/dist/js/bootstrap.min.js"}></script>
                <script src={jump_config::CDN_HOST ."/vendor/clippy.js/build/clippy.min.js"}></script>
                <script async={true} src={jump_config::CDN_HOST . "/js/" . file_get_contents("htdocs/js/main.js.latest")}></script>
            </p>;
    }
