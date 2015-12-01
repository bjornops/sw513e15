DIR=/usr/lib/cgi-bin
WASP : server.c
	sudo gcc -std=c99 server.c -o $(DIR)/WASP
	sudo $(DIR)/fixOwnership.sh
	sudo service apache2 restart
