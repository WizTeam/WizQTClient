QT -= gui

TEMPLATE = lib
CONFIG += staticlib
CONFIG += warn_off

macx {
    DEFINES += CRYPTOPP_DISABLE_ASM
    QMAKE_CXXFLAGS += -Wno-tautological-compare -Wno-unused-value -Wno-switch -Wno-string-plus-int -Wno-unused-variable -Wno-unused-parameter -Wno-everything
    QMAKE_CFLAGS += -Wno-unused-value -Wno-switch -Wno-string-plus-int -Wno-unused-variable -Wno-unused-parameter -Wno-everything
}

SOURCES += \
3way.cpp		cbcmac.cpp		eccrypto.cpp		hmac.cpp		osrng.cpp		rw.cpp			tftables.cpp \
adhoc.cpp		ccm.cpp			ecp.cpp			hrtimer.cpp		panama.cpp		safer.cpp		tiger.cpp \
adler32.cpp		channels.cpp		elgamal.cpp		ida.cpp			pch.cpp			salsa.cpp		tigertab.cpp \
algebra.cpp		cmac.cpp		emsa2.cpp		idea.cpp		pkcspad.cpp		seal.cpp		trdlocal.cpp \
algparam.cpp		cpu.cpp			eprecomp.cpp		integer.cpp		polynomi.cpp		seed.cpp		ttmac.cpp \
arc4.cpp		crc.cpp			esign.cpp		iterhash.cpp		pssr.cpp		serpent.cpp		twofish.cpp \
asn.cpp			cryptlib.cpp		files.cpp		luc.cpp			pubkey.cpp		sha.cpp			validat1.cpp \
authenc.cpp		cryptlib_bds.cpp	filters.cpp		mars.cpp		queue.cpp		shacal2.cpp		validat2.cpp \
base32.cpp		datatest.cpp		fips140.cpp		marss.cpp		rabin.cpp		shark.cpp		validat3.cpp \
base64.cpp		default.cpp		fipsalgt.cpp		md2.cpp			randpool.cpp		sharkbox.cpp		vmac.cpp \
basecode.cpp		des.cpp			fipstest.cpp		md4.cpp			rc2.cpp			simple.cpp		wait.cpp \
bench.cpp		dessp.cpp		gcm.cpp			md5.cpp			rc5.cpp			skipjack.cpp		wake.cpp \
bench2.cpp		dh.cpp			gf256.cpp		misc.cpp		rc6.cpp			socketft.cpp		whrlpool.cpp \
bfinit.cpp		dh2.cpp			gf2_32.cpp		modes.cpp		rdtables.cpp		sosemanuk.cpp		winpipes.cpp \
blowfish.cpp		dll.cpp			gf2n.cpp		mqueue.cpp		regtest.cpp		square.cpp		xtr.cpp \
blumshub.cpp		dlltest.cpp		gfpcrypt.cpp		mqv.cpp			rijndael.cpp		squaretb.cpp		xtrcrypt.cpp \
camellia.cpp		dsa.cpp			gost.cpp		nbtheory.cpp		ripemd.cpp		strciphr.cpp		zdeflate.cpp \
cast.cpp		eax.cpp			gzip.cpp		network.cpp		rng.cpp			tea.cpp			zinflate.cpp \
casts.cpp		ec2n.cpp		hex.cpp			oaep.cpp		rsa.cpp			test.cpp		zlib.cpp




# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

