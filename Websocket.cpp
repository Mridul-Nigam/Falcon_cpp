#include <iostream>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;
int i=1;
void on_message(server* s, websocketpp::connection_hdl hdl, server::message_ptr msg) {
    try {
        std::string data = msg->get_payload();
        std::cout << "Received Message: " << data << std::endl;

        // Parse the received data as a comma-separated list of integers
        std::vector<int> array;
        std::istringstream iss(data);
        std::string num;
        while (std::getline(iss, num, ',')) {
            array.push_back(std::stoi(num));
        }

        // Process the received array as needed
std::cout<<"i : "<<i<<std::endl;
i++;
        std::cout << "Received Array:";
        for (const auto& element : array) {
            std::cout << " " << element;
        }
        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception in on_message(): " << e.what() << std::endl;
    }
}


int main() {
    try {
        server ws_server;

        // Register message handler
        ws_server.set_message_handler(std::bind(&on_message, &ws_server, std::placeholders::_1, std::placeholders::_2));

        // Initialize the server
        ws_server.init_asio();
        ws_server.listen(9002);
        ws_server.start_accept();

        // Run the server
        ws_server.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception in main(): " << e.what() << std::endl;
    }

    return 0;
}
