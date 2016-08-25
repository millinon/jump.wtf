    sed '1d;$d' id_rsa | base64 -d
    sed '1d;s/ .*$//' id_rsa.pub | base64 -d
