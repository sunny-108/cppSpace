#include <memory>
#include <iostream>
#include <string>

class Connection{

    std::string serverID_;
    int connectionID_;
    static int nextID_;

    public:
    Connection(std::string serverName): serverID_(serverName), connectionID_(nextID_++){
        std::cout<<"\n server name = "<<serverID_;
        std::cout<<"\n connectionID = "<<connectionID_<<std::endl;
    }

    ~Connection(){
        std::cout<<" Disconnected from "<<serverID_<<std::endl;
    }

    void sendData(std::string data){
        std::cout<<"sending data = "<<data<<std::endl;
    }

};

int Connection::nextID_ = 1;

std::shared_ptr<Connection> connect_to_server(const std::string& server){
    return std::make_shared<Connection>(server);
}

int main(){

    
    auto conn = connect_to_server("db.server.com");
    std::cout<<" conn# "<<conn.use_count();
    {
        auto conn1 = conn;
        conn1->sendData("data conn 01");
        std::cout<<" conn1# "<<conn.use_count();
    }
    {
        auto conn2 = conn;
        conn2->sendData("data conn 02");
        std::cout<<" conn2# "<<conn.use_count();
    }

    conn->sendData("data conn 00");
    std::cout<<" conn# "<<conn.use_count();
}