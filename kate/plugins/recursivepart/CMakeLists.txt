set(katerecursivepartplugin_PART_SRCS plugin_katerecursivepart.cpp )

kde4_add_plugin(katerecursivepartplugin ${katerecursivepartplugin_PART_SRCS})

target_link_libraries(katerecursivepartplugin  ${KDE4_KDEUI_LIBS} kateinterfaces )

########### install files ###############
install(TARGETS katerecursivepartplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
# install( FILES ui.rc DESTINATION  ${DATA_INSTALL_DIR}/kate/plugins/katerecursivepart )
install( FILES katerecursivepart.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

