
set(katefilebrowserplugin_PART_SRCS katefilebrowserplugin.cpp katefilebrowserconfig.cpp katefilebrowser.cpp katebookmarkhandler.cpp katefilebrowserdialog.cpp )

kde4_add_plugin(katefilebrowserplugin ${katefilebrowserplugin_PART_SRCS})

target_link_libraries(katefilebrowserplugin ${KDE4_KFILE_LIBS} ${KDE4_KPARTS_LIBS} kateinterfaces )

install(TARGETS katefilebrowserplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES katefilebrowserplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

