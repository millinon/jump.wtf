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
            }
        };
        xhr.send(data);
    });



});
