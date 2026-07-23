#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "motor_control.h"


Motor::Motor(int id, std::string name, SMS_STS* bus) 
{
    motor_id = id;
    motor_name = name;
    serial_bus = bus;
}

// Function to ping a servo to test communication
bool Motor::ping() 
{
    int ID = serial_bus->Ping(0xFE);
	if(ID!=-1){
        std::cout<<"ID:"<<ID<<std::endl;
	}else{
        std::cout<<"Ping servo ID error!"<<std::endl;
        return false;
	}
	return true;
}

void Motor::enableTorque() 
{
    serial_bus->EnableTorque(motor_id, 1);
}

void Motor::queueMovement(int position) 
{
    serial_bus->RegWritePosEx(motor_id, position, speed, acceleration);//Servo (ID1) with maximum speed V=2400 (steps/second), acceleration A=50 (50*100 steps/second^2), move to position P1=4095
    vTaskDelay(pdMS_TO_TICKS(2));
    // serial_bus->RegWriteAction();
}

void Motor::queue_reset_motor_pos() 
{
    serial_bus->RegWritePosEx(motor_id, 0, speed, acceleration);//Servo (ID1) with maximum speed V=2400 (steps/second), acceleration A=50 (50*100 steps/second^2), move to position P0=0
    vTaskDelay(pdMS_TO_TICKS(2));
    // serial_bus->RegWriteAction();
    // std::cout<<"pos = "<<0<<std::endl;
    // usleep(2187*1000);//[(P1-P0)/V]*1000+[V/(A*100)]*1000
}


// Function to initialize the servo
bool setup(SMS_STS* bus, uart_port_t uartNum, int txPin, int rxPin)
{
    if (bus == nullptr) {
        return false;
    }

    return bus->begin(1000000, uartNum, txPin, rxPin);
}

void close_bus(SMS_STS* bus) 
{
    std::cout << "Closing motor" << std::endl;
    bus->end();
}


void execute_queued_movements(SMS_STS* bus) 
{
    bus->RegWriteAction();
    vTaskDelay(pdMS_TO_TICKS(2187));//[(P1-P0)/V]*1000+[V/(A*100)]*1000
}