#############################################################
#
# Music Player Daemon (MPD)
#
#############################################################
MPD_SOURCE=mpd-0.16.8.tar.bz2
MPD_DIR:=$(BUILD_DIR)/mpd-0.16.8

$(DL_DIR)/$(MPD_SOURCE):
	$(WGET) -P $(DL_DIR) $(DOWNLOAD_SITE)/$(MPD_SOURCE)

$(MPD_DIR)/.unpacked: $(DL_DIR)/$(MPD_SOURCE)
	tar -C $(BUILD_DIR) -xjf $(DL_DIR)/$(MPD_SOURCE)
	$(PATCH) $(MPD_DIR) $(BASE_DIR)/packages/mpd mpd\*.patch
	touch $(MPD_DIR)/.unpacked

$(MPD_DIR)/.configured: $(MPD_DIR)/.unpacked
	(cd $(MPD_DIR);\
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -std=gnu99" \
		LDFLAGS="$(TARGET_LDFLAGS) -lz" \
		PKG_CONFIG_LIBDIR=$(EPREFIX)/lib/pkgconfig \
		SQLITE_CFLAGS=-I$(EPREFIX)/include \
		SQLITE_LIBS="-L$(EPREFIX)/lib -lsqlite3" \
		MAD_CFLAGS=-I$(EPREFIX)/include \
		MAD_LIBS="-L$(EPREFIX)/lib -lmad" \
		./configure \
		--target=arm-none-linux-gnueabi \
		--host=arm-none-linux-gnueabi \
		--build=$(GNU_HOST_NAME) \
		--prefix=/usr \
		--sysconfdir=/etc \
		--localstatedir=/var \
		--enable-curl \
		--enable-mad \
		--enable-id3 \
		--enable-flac \
		--enable-sqlite \
		--enable-alsa \
		--disable-oss \
		--disable-ffmpeg \
	);
	touch $(MPD_DIR)/.configured

$(MPD_DIR)/src/mpd: $(MPD_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(MPD_DIR)
	touch -c $(MPD_DIR)/src/mpd
	
$(TARGET_DIR)/usr/bin/mpd: $(MPD_DIR)/src/mpd
	cp -dpf $(MPD_DIR)/src/mpd $(TARGET_DIR)/usr/bin
	-$(TARGET_STRIP) $(TARGET_DIR)/usr/bin/mpd
	cp -dpf $(BASE_DIR)/packages/mpd/mpd.conf $(TARGET_DIR)/etc
	cp -dpf $(BASE_DIR)/packages/mpd/mpd $(TARGET_DIR)/etc/rc.d/init.d
	touch -c $(TARGET_DIR)/usr/bin/mpd

mpd: glib curl libmad libid3tag flac sqlite alsa_lib $(TARGET_DIR)/usr/bin/mpd

mpd-source: $(DL_DIR)/$(MPD_SOURCE)

mpd-clean:
	-$(MAKE) -C $(MPD_DIR) clean
	-@rm -f $(TARGET_DIR)/usr/bin/mpd
	-@rm -f $(TARGET_DIR)/etc/rc.d/init.d/mpd

mpd-dirclean:
	rm -rf $(MPD_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(PACKAGE_MPD)),y)
TARGETS+=mpd
endif
