



########### next target ###############

set(katefastopenplugin_PART_SRCS plugin_katefastopen.cpp )


kde4_add_plugin(katefastopenplugin ${katefastopenplugin_PART_SRCS})



target_link_libraries(katefastopenplugin  ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_KPARTS_LIBS} kateinterfaces )

install(TARGETS katefastopenplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES ui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kate/plugins/katefastopen )
install( FILES katefastopen.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
