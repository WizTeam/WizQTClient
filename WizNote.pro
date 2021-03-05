TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    lib/zlib \
    lib/quazip \
    lib/cryptopp \
    src

quazip.depends = zlib
