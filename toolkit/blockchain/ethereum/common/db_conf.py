import os


dbhost = os.environ['MYSQL_URL']
dbuser = os.environ['MYSQL_USERNAME']
dbpass = os.environ['MYSQL_PASSWORD']
dbname = os.environ['YPCD_DB']
DB_URI = 'mysql://%s:%s@%s/%s' % (dbuser, dbpass, dbhost, dbname)
