# TODO: port away from QT3_SUPPORT!
# TODO: port to KNEWSTUFF3


if( KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)
    message(STATUS "Disabling katefiletemplates, which depends on deprecated knewstuff2. TODO: port to knewstuff3.")
else()

add_definitions(-DQT3_SUPPORT)

########### next target ###############

set(katefiletemplates_PART_SRCS filetemplates.cpp )

kde4_add_plugin(katefiletemplates ${katefiletemplates_PART_SRCS})

target_link_libraries(katefiletemplates ${KDE4_KNEWSTUFF2_LIBS} kateinterfaces ${KDE4_KTEXTEDITOR_LIBS} )

install(TARGETS katefiletemplates  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES ui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kate/plugins/katefiletemplates )
install( FILES katetemplate.xml  DESTINATION  ${DATA_INSTALL_DIR}/katepart/syntax )
install( FILES katefiletemplates.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES katefiletemplates.knsrc DESTINATION ${CONFIG_INSTALL_DIR} )

endif()
