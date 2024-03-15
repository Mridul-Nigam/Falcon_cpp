#include <iostream>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "dhdc.h"
#include "drdc.h"
#define DEFAULT_K_SLAVE 500.0
#define DEFAULT_K_BOX 500.0
#define MIN_SCALE           0.2
#define MAX_SCALE           5.0

#define MIN(a,b) ((a)<(b))?(a):(b)
#define MAX(a,b) ((a)>(b))?(a):(b)

typedef websocketpp::server<websocketpp::config::asio> server;
int i = 1;
void on_message(server* s, websocketpp::connection_hdl hdl, server::message_ptr msg) {
    try {

        std::string data = msg->get_payload();
        std::cout << "Received Message: " << data << std::endl;

        // Parse the received data as a comma-separated list of integers
        std::vector<double> array;
        std::istringstream iss(data);
        std::string num;
        while (std::getline(iss, num, ',')) {
            array.push_back(std::stod(num));
        }
        std::cout << "i : " << i << std::endl;
        i++;
        
        
        //--  -------------------------------------
        //std::cout << "Received Array:";
        //for (const auto& element : array) {
        //    std::cout << " " << element;
        //}
        //--  -------------------------------------
        
        
        //-----------------------------------------------------------------------------
        std::cout << "before drdpos" << array[1] << array[2] << array[3] << std::endl;
        drdTrackPos(array[1], array[2], array[3]);
        //-----------------------------------------------------------------------------

        std::cout << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in on_message(): " << e.what() << std::endl;
    }
}
int main(int  argc,
    char** argv)
{
    double mx0, my0, mz0;
    double mx = 0.0, my = 0.0, mz = 0.0;
    double sx0, sy0, sz0 = 0.0;
    double sx, sy, sz;
    double tx, ty, tz;
    double fx, fy, fz;
    double time;
    double refTime = dhdGetTime();
    double Kslave = DEFAULT_K_SLAVE;
    double Kbox = DEFAULT_K_BOX;
    double scale = 1;
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
    master = 1;
    slave = 0;

    // prefer Falcon as master
    if (dhdGetSystemType(0) != DHD_DEVICE_FALCON && dhdGetSystemType(1) == DHD_DEVICE_FALCON) {
        master = 0;
        slave = 1;
    }

    ushort mastersn, slavesn;
    dhdGetSerialNumber(&mastersn, master);
    dhdGetSerialNumber(&slavesn, slave);

    drdMoveToPos(0.0, 0.0, 0.0, false, slave);

    // initialize slave target position to current position
    //--  drdGetPositionAndOrientation(&tx, &ty, &tz, NULL, NULL, NULL, NULL, NULL, slave);

    // stop regulation on master, stop motion filters on slave
    drdStop(true, master);
    dhdSetForce(0.0, 0.0, 0.0, master);
    drdEnableFilter(true, slave);
    int flag = 0;
    // master slave loop


    double time1 = 0.0;
    int fl = 0;
    int duration = 0;
    double ax, vx, jk;

   //-- while (!done) {
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
        }
        catch (const std::exception& e) {
            std::cerr << "Exception in main(): " << e.what() << std::endl;
        }

        duration++;
        if (fl < 1) {
            time1 = drdGetTime();
            fl = 1;
        }
        //svr.listen("localhost", 8082);



    //-- }

    time = dhdGetTime();
    if (time - refTime > 0.04)
    {

        if (dhdKbHit())
        {
            switch (dhdKbGet())
            {
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

    // close the connection
    drdClose(slave);
    drdClose(master);

    // exit
    printf("\ndone.\n");
    return 0;

}
