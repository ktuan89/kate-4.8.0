set(katerecentfileviewerplugin_PART_SRCS plugin_katerecentfileviewer.cpp )

kde4_add_plugin(katerecentfileviewerplugin ${katerecentfileviewerplugin_PART_SRCS})

target_link_libraries(katerecentfileviewerplugin  ${KDE4_KDEUI_LIBS} kateinterfaces )

########### install files ###############
install(TARGETS katerecentfileviewerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES ui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kate/plugins/katerecentfileviewer )
install( FILES katerecentfileviewer.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

