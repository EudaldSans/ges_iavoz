# Edit following two lines to set component requirements (see docs)
set(component ges_iavoz)

idf_component_register( SRCS 
                            
                            "ges_iavoz.cc" 
                            "ges_iavoz_main.cc" 
                            "ges_iavoz_audio_provider.cc" 
                            "ges_iavoz_feature_provider.cc" 
                            "ges_iavoz_command_recognizer.cc" 
                            "switch_lite_I_low_latency_conv.cc" 
                            "ringbuf.c"
                        INCLUDE_DIRS "."

                        REQUIRES "esp-nn" tflite-lib
                    )

# register_component()

