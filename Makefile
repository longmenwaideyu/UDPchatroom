all:chatServer chatClient
chatServer:chatServer.c
	gcc $^ -o $@
chatClient:chatClient.c
	gcc $^ -o $@ -lpthread
clean:
	rm -f chatClient
	rm -f chatServer