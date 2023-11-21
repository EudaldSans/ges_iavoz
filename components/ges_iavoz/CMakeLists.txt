# Edit following two lines to set component requirements (see docs)
set(component ges_iavoz)

idf_component_register( SRCS 
                            
                            "ges_connect.c"
                            "ges_events.c"
                            "ges_iavoz.cc" 
                            "ges_iavoz_main.cc" 
                            "ges_iavoz_audio_provider.cc" 
                            "ges_iavoz_feature_provider.cc" 
                            "ges_iavoz_command_recognizer.cc" 
                            "model1.cc"
                            "ges_iavoz_command_responder.cc"
                            "ringbuf.c"
                        INCLUDE_DIRS "."

                        REQUIRES "esp-nn" tflite-lib libfvad
                        PRIV_REQUIRES nvs_flash
                    )

# register_component()

