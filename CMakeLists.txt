idf_component_register(SRCS 
                        "src/createtransitions.c"
                        "src/debug.c"
                        "src/esp32_sam.c"
                        "src/processframes.c"
                        "src/reciter.c"
                        "src/render.c"
                        "src/sam.c"
                    INCLUDE_DIRS "src/include/"
                    PRIV_INCLUDE_DIRS "src/priv_include/"
                    REQUIRES "log")
