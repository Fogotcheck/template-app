idf_component_register(SRCS "main.c"
                    "BaseProtocol/BaseProtocol.c"
                    "RegsMap.c"
                    "CRC_polinoms/crcPolinoms.c"
                    "Usart/usart.c"
                    "Wifi/wifi.c"
                    "mqtt/mqttClient.c"
                    "ADC/myadc.c"
                    "AlphaBettaFilt/calman.c"
                    INCLUDE_DIRS ".")