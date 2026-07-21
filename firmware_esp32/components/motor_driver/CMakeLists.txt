idf_component_register(
    SRCS "motor_control.cpp"
    INCLUDE_DIRS "."
    REQUIRES STServo # motor_driver uses the SMS_STS class from STServo           
)