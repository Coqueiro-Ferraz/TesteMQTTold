idf_component_register(SRCS "connect.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver
                    REQUIRES esp_wifi
                    REQUIRES esp_netif
                    )