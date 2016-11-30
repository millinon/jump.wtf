$(document).ready(function(){

    $("#shorten-btn").click(function() {

        var input = $("#input-url");


        var data = JSON.stringify({
            action: "genURL",
            "input-url": $(input).val()
        });

        var xhr = new XMLHttpRequest();
        xhr.open("POST", "https://jump.wtf/a", true);
        xhr.setRequestHeader("Content-Type", "application/json");
        xhr.onreadystatechange = function() {
            if(xhr.readyState == 4){
                var resp = JSON.parse(xhr.responseText);
                $("#output-url").val(resp.success?resp.url:"#Error");
                $("#copy-url-btn").attr("data-clipboard-target", resp.success?resp.url:"#Error");
            }
        };
        xhr.send(data);
    });

    (function(){


        function register(name){

            clip = new Clipboard('#' + name);

            clip.on('success', function(e){
                /*
                msg = $("#copy-success");
                msg.stop();
                msg.fadeIn('fast', function(){
                    $(this).delay(3000).fadeOut('slow');
                });*/

            });

            clip.on('error', function(e){
               /* msg = $("#copy-failure");
                msg.stop();
                msg.fadeIn('fast', function(){
                    $(this).delay(3000).fadeOut('slow');
                });*/
            });

            return clip;
        }

        register('copy-url-btn');
        register('copy-file-btn');

    }());

});
