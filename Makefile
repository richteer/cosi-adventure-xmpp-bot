all: cosi-bot.o bot

cosi-bot.o: cosi-bot.c cosi-bot.h
	gcc -c $<

bot: bot.c cosi-bot.o
	gcc -o $@ $^ -lstrophe -lexpat -lssl -lresolv
