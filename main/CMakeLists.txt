idf_component_register(SRCS "app_main.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver
                    REQUIRES esp_wifi
                    REQUIRES esp_netif
                    REQUIRES nvs_flash
                    REQUIRES protocol_examples_common
                    REQUIRES mqtt
                    REQUIRES esp_adc
                    REQUIRES IOconfigs
                    REQUIRES LCD
                    REQUIRES log
                    )
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common
    EXTRA_COMPONENT_DIRS coqueiro2/TesteMQTT/components/IOconfigs)

