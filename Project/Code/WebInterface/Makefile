DIR=/usr/lib/cgi-bin
WASP : server.cpp
	sudo g++ -Wno-write-strings server.cpp -o $(DIR)/WASP
	sudo $(DIR)/fixOwnership.sh
	sudo service apache2 restart
