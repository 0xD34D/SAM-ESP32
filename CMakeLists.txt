idf_component_register(SRCS 
                        "src/debug.c"
                        "src/createtransitions.c"
                        "src/processframes.c"
                        "src/reciter.c"
                        "src/render.c"
                        "src/sam.c"
                    INCLUDE_DIRS "src/"
                    REQUIRES "freertos" "log")
