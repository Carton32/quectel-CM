release: clean qmi-proxy
	$(CC) -Wall -s QmiWwanCM.c GobiNetCM.c main.c MPQMUX.c QMIThread.c util.c udhcpc.c -o quectel-CM -lpthread -ldl

debug: clean
	$(CC) -Wall -g QmiWwanCM.c GobiNetCM.c main.c MPQMUX.c QMIThread.c util.c udhcpc.c -o quectel-CM -lpthread -ldl

qmi-proxy:
	$(CC) -Wall -s quectel-qmi-proxy.c  -o quectel-qmi-proxy -lpthread -ldl

clean:
	rm -rf quectel-CM *~
	rm -rf quectel-qmi-proxy
