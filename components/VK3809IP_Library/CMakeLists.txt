
# set(COMPONENT_SRCS "./VK3809IP_Library/src/vk3809ip.cpp")
# set(COMPONENT_ADD_INCLUDEDIRS "./VK3809IP_Library/src")
# register_component()

idf_component_register(SRCS "src/vk3809ip.cpp"
                    INCLUDE_DIRS "src"
                    )