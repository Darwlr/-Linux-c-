all : ChatServer ChatClient
.PHONY : all

ChatServer:ChatServer.cc
	g++ -o $@ $^ -lpthread -std=c++11 -L jsoncpp/lib -ljsoncpp
ChatClient:ChatClient.cc
	g++ -o $@ $^ $(INCLUDES) -std=c++11 -L jsoncpp/lib -ljsoncpp -lncurses -lpthread
.PHONY : clean
clean:
	rm -f ChatServer ChatClient
