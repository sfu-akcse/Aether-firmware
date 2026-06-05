#include <cstring>
#include <iostream>
#include <STServo/SCServo.h>

#include "motor_control.h"

int main() {
    const char* motor_address = "/dev/ttyACM0";
    SMS_STS st;
    char option = 'a';
    std::cout << "Starting Aether Motor Test..." << std::endl;
    
    setup(&st, motor_address);

    Motor test_motor = Motor(1, "test_motor", &st);
    
    // Call the functions defined in motor_control.cpp

    while(option != 'q') {
        std::cout << "Please enter '1' for ping test, '2' for motor test, and 'q' to exit: " << std::endl;
        std::cin >> option;
        
        switch(option) {
            case '1':
                test_motor.ping();
                std::cout << "Ping complete!" << std::endl;
                break;
            case '2':
                test_motor.queueMovement(4095);
                std::cout << "pos = " << 4095 <<std::endl;
                execute_queued_movements(&st);
                test_motor.queue_reset_motor_pos();
                std::cout << "pos = " << 0 <<std::endl;
                execute_queued_movements(&st);
                std::cout << "Motor test complete!" << std::endl;
                break;
            case 'q':
                close_bus(&st);
                std::cout << "Exiting loop" << std::endl;
                break;
            default:
                std::cout << "Invalid option selected, please try again!" << std::endl;
        }
    }
    
    
    std::cout << "Test complete." << std::endl;
    return 0;
}