#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

class Motor {
private:
    int motor_id;
    std::string motor_name;
    SMS_STS* serial_bus; // A pointer to the shared communication line
    bool torque_enabled;
    const int speed = 2400;
    const int acceleration = 50;

public:
    // Constructor
    Motor(int id, std::string name, SMS_STS* bus);

    // Motor actions
    bool ping();
    void enableTorque();
    void queueMovement(int position);
    void queue_reset_motor_pos();
};

// Global functions for the servo and its connection
bool setup(SMS_STS* bus, const char* address);
void close_bus(SMS_STS* bus);
void execute_queued_movements(SMS_STS* bus);

#endif