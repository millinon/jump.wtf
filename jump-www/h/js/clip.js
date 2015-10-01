var clip = new Clipboard('#copybutton');

clip.on('success', function(e){
        btn = document.getElementById("copybutton");
        par = btn.parentNode;
        el = document.createElement("div");
        el.className = "alert alert-success";
        el.setAttribute("style","text-align:center;margin:0 auto;width:20%;");
        el.style.height = btn.clientHeight;
        el.innerHTML = "Copied to clipboard.";
        par.replaceChild(el,btn);
        });
clip.on('error', function(e){
        btn = document.getElementById("copybutton");
        par = btn.parentNode;
        el = document.createElement("div");
        el.className = "alert alert-warning";
        el.setAttribute("style","text-align:center;margin:0 auto;width:20%;");
        el.style.height = btn.clientHeight;
        el.innerHTML = "Couldn't copy to clipboard.";
        par.replaceChild(el,btn);
        });
