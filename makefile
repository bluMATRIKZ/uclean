.PHONY: all remove

all:
	sudo gcc main.c -o /bin/uclean
    
remove:
	sudo rm -f /bin/uclean
