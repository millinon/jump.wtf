<?hh

class aws_config {
    const BASEURL = "https://jump.wtf/"; /* URL to private file uploads */
    const FBASEURL = "https://f.jump.wtf/"; /* URL to public file uploads: I use a CDN for this */

    const UBASEDIR = "/var/jump/u"; /* directory where uploads are moved temporarily */

    const VAULTNAME = "jump.wtf"; /* vault to upload backups to */
    const SAVE_BACKUP = TRUE; /* set to false to skip backups */

    const MAXCLICKS = 1000; /* maximum expiration clicks */
    const MAXURLLEN = 128; /* maximum URL length */
    const MAXPASSLEN = 20; /* maximum password length */

    const PUBBUCKET = "jump.wtf"; /* bucket to store public files in */
    const PRIVBUCKET = "p.jump.wtf"; /* bucket to store private files in */

    const CDN_HOST = "https://cdn.jump.wtf"; /* Where clients should download HTML resources from */
}
