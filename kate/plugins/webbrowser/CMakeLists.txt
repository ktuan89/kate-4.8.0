set(katewebbrowserplugin_PART_SRCS plugin_katewebbrowser.cpp )

kde4_add_plugin(katewebbrowserplugin ${katewebbrowserplugin_PART_SRCS})

target_link_libraries(katewebbrowserplugin  ${KDE4_KDEUI_LIBS} ${KDE4_KDEWEBKIT_LIBS} kateinterfaces )

########### install files ###############
install(TARGETS katewebbrowserplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES ui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kate/plugins/katewebbrowser )
install( FILES katewebbrowser.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

