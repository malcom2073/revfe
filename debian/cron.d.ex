#
# Regular cron jobs for the revfe package
#
0 4	* * *	root	[ -x /usr/bin/revfe_maintenance ] && /usr/bin/revfe_maintenance
