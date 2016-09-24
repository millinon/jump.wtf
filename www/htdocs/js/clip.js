
(function(){


    function register(name){

    clip = new Clipboard('#' + name);

    clip.on('success', function(e){

        msg = $("#copy-success");
        msg.stop();
        msg.fadeIn('fast', function(){
            $(this).delay(3000).fadeOut('slow');
        });

    });

    clip.on('error', function(e){
        msg = $("#copy-failure");
        msg.stop();
        msg.fadeIn('fast', function(){
            $(this).delay(3000).fadeOut('slow');
        });
    
        /*
        btn = document.getElementById(name);
        par = btn.parentNode;
        el = document.createElement("div");
        el.className = "alert alert-warning";
        el.setAttribute("style","text-align:center;margin:0 auto;width:20%;");
        el.style.height = btn.clientHeight;
        el.innerHTML = "Couldn't copy to clipboard.";
        par.replaceChild(el,btn);*/
        });

    return clip;
}

    register('copy-url');
    register('copy-cdn');
    register('copy-tor');

}());
