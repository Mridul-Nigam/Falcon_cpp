#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <winsock2.h>
using namespace std;
#pragma comment(lib, "ws2_32.lib") // Link with the Winsock library
#include <chrono>

// C++ library headers
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "drdc.h"
#include "dhdc.h"

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <json/json.h>
#include <set>

#define DEFAULT_K_SLAVE   500.0
#define DEFAULT_K_BOX     500.0
#define SLAVE_BOX_SIZE      0.06
#define MIN_SCALE           0.2
#define MAX_SCALE           5.0

#define MIN(a,b) ((a)<(b))?(a):(b)
#define MAX(a,b) ((a)>(b))?(a):(b)

typedef websocketpp::server<websocketpp::config::asio> server;
typedef server::connection_ptr connection_ptr;

bool streaming = false; // Flag to control streaming
bool   engaged = false;
int    master, slave;
double mx0, my0, mz0;
double mx, my, mz;
double sx0, sy0, sz0;
double sx, sy, sz;
double tx, ty, tz;
double fx, fy, fz;
double time0, scale=1.0;
double refTime = dhdGetTime();
double Kslave = DEFAULT_K_SLAVE;
double Kbox = DEFAULT_K_BOX;

struct forceData {
    /*int fX;
    int fY;
    int fZ;*/
    double fX;
    double fY;
    double fZ;
    int64_t sendTime;
    double dt;
    //double fX0;
    //double fY0;
    //double fZ0;
    //bool eng;
};
struct Position {
    double x;
    double y;
    double z;
};


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
if (!engaged && dhdGetButtonMask(master) != 0x00) {
                printf("I m here");
                // store start position
                dhdGetPosition(&mx0, &my0, &mz0, master);
                dhdGetPosition(&sx0, &sy0, &sz0, slave);
                engaged = true;
            }
            // detect button release, disable slave control
            else if (engaged && dhdGetButtonMask(master) == 0x00) {
                engaged = false;
            }

            // if slave control is enabled, move the slave to match the master movement
            if (engaged) {
                printf("%f  %f  %f\n", mx0, my0, mz0);
                // get master position
                dhdGetPosition(&mx, &my, &mz, master);
                tx = sx0 + scale * (mx - mx0);
                ty = sy0 + scale * (my - my0);
                tz = sz0 + scale * (mz - mz0);
            }
            if (5 > 3) {
                dhdGetPosition(&mx, &my, &mz, master);
                int encoders[DHD_MAX_DOF] = {};
                dhdGetForce(&fx, &fy, &fz);
                if (dhdGetEnc(encoders) < 0)
                {
                    std::cout << "error: failed to read encoders (" << dhdErrorGetLastStr() << ")" << std::endl;
                    break;
                }
                auto sendTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                dhdGetPosition(&mx, &my, &mz);
    array.push_back(i);
    array.push_back(mx);
    array.push_back(my);
    array.push_back(mz);

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

                duration++;
                printf("\ndata# %d curr: %f sent position: %d % d % d ", duration, drdGetTime() - time1, encoders[0], encoders[1], encoders[2]);
                dhdSleep(0.02);

}

void on_open(websocket_server* s, websocketpp::connection_hdl hdl) {
    //static int i = 1; // Declare static variable to maintain its value across function calls
    std::cout << "Connection opened" << std::endl;
    // Store the connection handle for later use
    s->send(hdl, "Connection opened", websocketpp::frame::opcode::text);

    // Start sending random arrays periodically
     std::thread([s, hdl]() {
//----------------------------------------------------

    double sequenceNumber = 0.005;
    double time1 = drdGetTime();
    int duration = 0;
    int    done = 0;
    dhdEnableExpertMode();
    while (!done) 
{            
send_random_array(s, hdl);
//s->send(hdl, "Connection opened", websocketpp::frame::opcode::text);
            //std::this_thread::sleep_for(std::chrono::microseconds(1)); // Send every .1 millisecond
        }
    }).detach();
            
            fx = fy = fz = 0.0;
            // apply force to master
            dhdSetForce(fx, fy, fz, master);

            // print stats and check for exit condition
            time0 = dhdGetTime();
            if (time0 - refTime > 0.04) {
                printf("[%c] scale = %0.03f | K = %04d | master %0.02f kHz | slave %0.02f kHz            \r",
                    (engaged ? '*' : ' '), scale, (int)Kslave, dhdGetComFreq(master), drdGetCtrlFreq(slave));
                refTime = time0;
                //if (!drdIsRunning(slave)) done = -1;
                if (dhdKbHit()) {
                    switch (dhdKbGet()) {
                    case 'q': done = 1;   break;
                    case 'k': Kslave -= 100.0; break;
                    case 'K': Kslave += 100.0; break;
                    case 's': if (!engaged) scale = MAX(MIN_SCALE, scale - 0.1); break;
                    case 'S': if (!engaged) scale = MIN(MAX_SCALE, scale + 0.1); break;
                    }
                }
            }
    // report exit cause
    printf("                                                                           \r");
    if (done == -1) printf("\nregulation finished abnormally on slave device\n");
    else            printf("\nexiting on user request\n");

}

//-----------------------------------------------------------

void on_close(websocket_server* s, websocketpp::connection_hdl hdl) {
    std::cout << "Connection closed" << std::endl;
}

int
main(int  argc,
    char** argv)
{
    double mx0, my0, mz0;
    double mx, my, mz;
    double sx0, sy0, sz0;
    double sx, sy, sz;

    double tx, ty, tz;
    double fx, fy, fz;
    double time;
    double refTime = dhdGetTime();
    double Kslave = DEFAULT_K_SLAVE;
    double Kbox = DEFAULT_K_BOX;
    double scale = 1.0;
    bool   engaged = false;
    int    done = 0;
    int    master, slave;


    // message


    // open and initialize 2 devices
    for (int dev = 0; dev < 1; dev++) {

        // open device
        if (drdOpenID(dev) < 0) {
            printf("error: not enough devices found\n");
            dhdSleep(2.0);
            for (int j = 0; j <= dev; j++) drdClose(j);
            return -1;
        }

        // exclude some device types that have not been fully tested with 'mirror'
        bool incompatible = false;
        switch (dhdGetSystemType()) {
        case DHD_DEVICE_SIGMA331:
        case DHD_DEVICE_SIGMA331_LEFT:
            incompatible = true;
            break;
        }

        // check that device is supported
        if (incompatible || !drdIsSupported()) {
            printf("error: unsupported device (%s)\n", dhdGetSystemName(dev));
            dhdSleep(2.0);
            for (int j = 0; j <= dev; j++) drdClose(j);
            return -1;
        }

        // initialize Falcon by hand if necessary
        if (!drdIsInitialized() && dhdGetSystemType() == DHD_DEVICE_FALCON) {
            printf("please initialize Falcon device...\r"); fflush(stdout);
            while (!drdIsInitialized()) dhdSetForce(0.0, 0.0, 0.0);
            printf("                                  \r");
            dhdSleep(0.5);
        }

        // initialize if necessary
        if (!drdIsInitialized(dev) && (drdAutoInit(dev) < 0)) {
            printf("error: initialization failed (%s)\n", dhdErrorGetLastStr());
            dhdSleep(2.0);
            for (int j = 0; j <= dev; j++) drdClose(j);
            return -1;
        }

        // start robot control loop
        if (drdStart(dev) < 0) {
            printf("error: control loop failed to start properly (%s)\n", dhdErrorGetLastStr());
            dhdSleep(2.0);
            for (int j = 0; j <= dev; j++) drdClose(j);
            return -1;
        }
    }

    // default role assignment
    master = 0;
    slave = 1;

    // prefer Falcon as master
    if (dhdGetSystemType(0) != DHD_DEVICE_FALCON && dhdGetSystemType(1) == DHD_DEVICE_FALCON) {
        master = 1;
        slave = 0;
    }



    dhdEmulateButton(DHD_ON, master);

    ushort mastersn, slavesn;
    dhdGetSerialNumber(&mastersn, master);
    dhdGetSerialNumber(&slavesn, slave);
    printf("%s haptic device [sn: %04d] as master\n", dhdGetSystemName(master), mastersn);
    printf("%s haptic device [sn: %04d] as slave\n", dhdGetSystemName(slave), slavesn);

    // display instructions
    printf("\n");
    printf("press 's' to decrease scaling factor\n");
    printf("      'S' to increase scaling factor\n");
    printf("      'k' to decrease virtual stiffness\n");
    printf("      'K' to increase virtual stiffness\n");
    printf("      'q' to quit\n\n");

    // center both devices
    drdMoveToPos(0.0, 0.0, 0.0, false, master);
    //drdMoveToPos(0.0, 0.0, 0.0, true, slave);
    while (drdIsMoving(master) || drdIsMoving(slave)) drdWaitForTick(master);

    // initialize slave target position to current position
    drdGetPositionAndOrientation(&tx, &ty, &tz, NULL, NULL, NULL, NULL, NULL, slave);

    // stop regulation on master, stop motion filters on slave
    drdStop(true, master);
    dhdSetForce(0.0, 0.0, 0.0, master);
    //drdEnableFilter(false, slave);

    struct forceData {
        /*int fX;
        int fY;
        int fZ;*/
        double fX;
        double fY;
        double fZ;
        int64_t sendTime;
        double dt;
        //double fX0;
        //double fY0;
        //double fZ0;
        //bool eng;
    };
    int encoders[DHD_MAX_DOF] = {};

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
