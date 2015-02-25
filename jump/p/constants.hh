<?hh

$MINLEN = 3;
$MAXLEN = 5;

$BASEURL = "https://jump.wtf/"; /* URL to private file uploads */
$FBASEURL = "https://f.jump.wtf/"; /* URL to public file uploads: I use a CDN for this */

$UBASEDIR = "/var/www/jump/u"; /* directory where uploads are moved temporarily */

$VAULTNAME = "jump.wtf"; /* vault to upload backups to */
$SAVE_BACKUP = TRUE; /* set to false to skip backups */

$MAXCLICKS = 1000; /* maximum expiration clicks */
$MAXURLLEN = 128; /* maximum URL length */
$MAXPASSLEN = 20; /* maximum password length */

$PUBBUCKET = "jump.wtf"; /* bucket to store public files in */
$PRIVBUCKET = "p.jump.wtf"; /* bucket to store private files in */

$CDN_HOST = "https://cdn.jump.wtf"; /* Where clients should download HTML resources from */

require('p/key_charset.hh');
