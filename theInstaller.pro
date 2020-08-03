TEMPLATE = subdirs

CONFIG(installer, applib|installer) {
    message("Building an installer")
    SUBDIRS = installer
} else:CONFIG(applib, applib|installer) {
    message("Building libtheinstaller")
    SUBDIRS = libtheinstaller
}
