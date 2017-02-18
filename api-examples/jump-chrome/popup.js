function shortenURL(url, callback, errCallback) {
    var xhr = new XMLHttpRequest();

    var data = JSON.stringify({
        'action': 'genURL',
        'input-url': url
    });

    xhr.open('POST', 'https://jump.wtf/a', true);
    xhr.setRequestHeader('Content-Type', 'application/json');
    xhr.responseType = 'json';
    xhr.onreadystatechange = function() {
        if(xhr.readyState == 4){
            var resp = xhr.response;
            if(!resp){
                errCallback("XHR failed.");
            } else if(!resp.success){
                errCallback(resp.message);
            } else {
                callback(resp.url);
            }
        }
    };
    xhr.timeout = 5000;
    xhr.ontimeout = function(e){
        errCallback("XHR timed out.");
    };
    xhr.send(data);
}

function renderStatus(statusText) {
    document.getElementById('status').textContent = statusText;
}

document.addEventListener('DOMContentLoaded', function() {
    chrome.tabs.query({active:true, currentWindow:true}, function(tabs) {
        shortenURL(tabs[0].url, function(shortURL){
            document.getElementById('status').textContent = 'URL Shortened';
            document.getElementById('message').textContent = 'Your jump.wtf link: ';
            var link = document.getElementById('link');
            link.href = shortURL;
            link.textContent = shortURL;

            var img = document.getElementById('qr-code');
            img.src = 'https://chart.googleapis.com/chart?chs=300x300&cht=qr&chl=' + encodeURIComponent(shortURL) + '&choe=UTF-8';
            img.hidden = false;

            document.getElementById('button').style.display = 'block';
        }, function(error){
            document.getElementById('status').textContent = 'Error';
            document.getElementById('message').textContent = error;
        });
    });

    document.getElementById('button').addEventListener('click', function (){
        var text = document.getElementById('link').href;
        var input = document.createElement('input');
        input.style.position = 'fixed';
        input.style.opacity = 0;
        input.value = text;
        document.body.appendChild(input);
        input.select();
        document.execCommand('Copy');
        document.body.removeChild(input);
        document.getElementById('btn-div').innerHTML = 'Link copied to clipboard.'
    });
});
