idf_component_register(
    SRCS
        "SCS.cpp"
        "SCSerial.cpp"
        "SMS_STS.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_uart
)