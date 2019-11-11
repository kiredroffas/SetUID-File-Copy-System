all: scopy addConfig

scopy: scopy.c
	gcc -o scopy scopy.c

addConfig: addConfig.c
	gcc -o addConfig addConfig.c

clean:
	rm scopy addConfig