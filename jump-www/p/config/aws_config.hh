<?hh

class aws_config {
    const BACKUP_VAULT_NAME = "jump.wtf"; /* vault to upload backups to */

	const LINK_TABLE = "key-list"; /* name of the DynamoDB link table */

	const LOG_TABLE = "jump.wtf-log"; /* name of the DynamoDB logging table */

    const PUB_BUCKET = "jump.wtf"; /* bucket to store public files in */
    const PRIV_BUCKET = "p.jump.wtf"; /* bucket to store private files in */
}
