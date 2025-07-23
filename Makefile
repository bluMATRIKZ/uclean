.PHONY: all remove

all:
	sudo gcc main.c -o /usr/local/bin/uclean
    
remove:
	sudo rm -f /usr/local/bin/uclean
