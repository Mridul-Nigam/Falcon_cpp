
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include <chrono>
#include <thread>
#include <random>

using websocket_server = websocketpp::server<websocketpp::config::asio>;
int i=1;

void send_random_array(websocket_server* s, websocketpp::connection_hdl hdl) {
    // Generate a random array of six elements
    std::vector<int> array;
    //std::random_device rd;
    //std::mt19937 gen(rd());
    //std::uniform_int_distribution<> dis1(0, 10);
    //std::uniform_int_distribution<> dis2(10, 20);
    //std::uniform_int_distribution<> dis3(20, 30);
    //std::uniform_int_distribution<> dis4(30, 40);
    //std::uniform_int_distribution<> dis5(40, 50);
    //std::uniform_int_distribution<> dis6(50, 60);

    array.push_back(i);
    array.push_back(2);
    array.push_back(3);
    array.push_back(4);
    array.push_back(5);
    array.push_back(6);

    std::cout << "The array no. " << i << " sending is: ";
    for (const auto& element : array) {
        std::cout << element << " ";
    }
    std::cout << std::endl;
    ++i;

    // Convert the array to a comma-separated string
    std::stringstream ss;
    for (size_t i = 0; i < array.size(); ++i) {
        ss << array[i];
        if (i < array.size() - 1) {
            ss << ",";
        }
    }
    //std::cout << "Comma-separated string: " << ss.str() << std::endl;

    // Send the string data via WebSocket to the client
    s->send(hdl, ss.str(), websocketpp::frame::opcode::text);
//s->send(hdl, array.data(), array.size() * sizeof(int), websocketpp::frame::opcode::binary);

}

void on_open(websocket_server* s, websocketpp::connection_hdl hdl) {
    //static int i = 1; // Declare static variable to maintain its value across function calls
    std::cout << "Connection opened" << std::endl;
    // Store the connection handle for later use
    s->send(hdl, "Connection opened", websocketpp::frame::opcode::text);

    // Start sending random arrays periodically
     std::thread([s, hdl]() {
        while (true) {
            send_random_array(s, hdl);
//s->send(hdl, "Connection opened", websocketpp::frame::opcode::text);
            //std::this_thread::sleep_for(std::chrono::microseconds(1)); // Send every .1 millisecond
        }
    }).detach();
}

void on_close(websocket_server* s, websocketpp::connection_hdl hdl) {
    std::cout << "Connection closed" << std::endl;
}

int main() {
    websocket_server server;

    // Set logging level
    //server.set_access_channels(websocketpp::log::alevel::none);
    //server.clear_access_channels(websocketpp::log::alevel::all);

    // Initialize Asio
    server.init_asio();

    // Register callbacks
    server.set_open_handler(websocketpp::lib::bind(&on_open, &server, std::placeholders::_1));
    server.set_close_handler(websocketpp::lib::bind(&on_close, &server, std::placeholders::_1));

    // Start the server
    server.listen(9003);
    server.start_accept();

    // Run the server
    server.run();

    return 0;
}
