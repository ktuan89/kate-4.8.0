set(katefilestoreplugin_PART_SRCS plugin_katefilestore.cpp )

kde4_add_plugin(katefilestoreplugin ${katefilestoreplugin_PART_SRCS})

target_link_libraries(katefilestoreplugin  ${KDE4_KDEUI_LIBS} kateinterfaces ${KDE4_KTEXTEDITOR_LIBS})

########### install files ###############
install(TARGETS katefilestoreplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
# install( FILES ui.rc DESTINATION  ${DATA_INSTALL_DIR}/kate/plugins/katefilestore )
install( FILES katefilestore.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

