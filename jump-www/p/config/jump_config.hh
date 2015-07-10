<?hh

class jump_config {
    const BASEURL = "https://jump.wtf/"; /* URL to private file uploads */
    const FBASEURL = "https://f.jump.wtf/"; /* URL to public file uploads: I use a CDN for this */

    const UBASEDIR = "/var/jump/u"; /* directory where uploads are moved temporarily */

	const SAVE_BACKUP = TRUE; /* set to false to skip backups of user-uploaded files */

	const DO_LOG = FALSE; /* set to false to disable IP logging */

    const MAX_CLICKS = 1000; /* maximum expiration clicks */
    const MAX_URL_LEN = 128; /* maximum URL length */
    const MAX_PASS_LEN = 20; /* maximum password length */

    const CDN_HOST = "//cdn.jump.wtf"; /* Where clients should download HTML resources from */

	const MEMCACHE_HOSTS = "jump-bans.hpfzol.cfg.use1.cache.amazonaws.com";
}
