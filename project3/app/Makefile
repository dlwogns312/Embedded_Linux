APP := app

all: app

app:
	arm-none-linux-gnueabi-gcc -static -o $(APP) $(APP).c

clean:
	rm -rf $(APP) *.o