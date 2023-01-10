# Edit following two lines to set component requirements (see docs)
set(component xmfcc)

idf_component_register( SRCS 
                            "xmfcc.cpp"
                        INCLUDE_DIRS "."
                    )

# register_component()

