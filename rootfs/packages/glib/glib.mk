#############################################################
#
# GLib
#
#############################################################
GLIB_SOURCE:=glib-2.12.9.tar.bz2
GLIB_DIR:=$(BUILD_DIR)/glib-2.12.9

$(DL_DIR)/$(GLIB_SOURCE):
	 $(WGET) -P $(DL_DIR) $(DOWNLOAD_SITE)/$(GLIB_SOURCE)

$(GLIB_DIR)/.unpacked: $(DL_DIR)/$(GLIB_SOURCE)
	tar -C $(BUILD_DIR) -xjf $(DL_DIR)/$(GLIB_SOURCE)
	touch $(GLIB_DIR)/.unpacked

$(GLIB_DIR)/.configured: $(GLIB_DIR)/.unpacked
	(cd $(GLIB_DIR); rm -rf config.cache; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		ac_cv_func_posix_getpwuid_r=yes \
		glib_cv_stack_grows=no \
		glib_cv_uscore=no \
		./configure \
		--target=arm-none-linux-gnueabi \
		--host=arm-none-linux-gnueabi \
		--build=$(GNU_HOST_NAME) \
		--prefix=/ \
		--exec-prefix=$(EPREFIX) \
		--includedir=$(EPREFIX)/include \
		--localstatedir=/var \
		--datarootdir=/usr/share \
		--enable-shared=yes \
		--enable-static=yes \
		--with-libiconv=gnu \
	);
	touch $(GLIB_DIR)/.configured

$(GLIB_DIR)/glib/.libs/libglib-2.0.so: $(GLIB_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(GLIB_DIR)
	touch -c $(GLIB_DIR)/glib/.libs/libglib-2.0.so

$(HOST_DIR)$(EPREFIX)/lib/libglib-2.0.so: $(GLIB_DIR)/glib/.libs/libglib-2.0.so
	$(MAKE) DESTDIR=$(HOST_DIR) -C $(GLIB_DIR) install
	touch -c $(HOST_DIR)$(EPREFIX)/lib/libglib-2.0.so

$(TARGET_DIR)$(EPREFIX)/lib/libglib-2.0.so: $(HOST_DIR)$(EPREFIX)/lib/libglib-2.0.so
	cp -dpf $(HOST_DIR)$(EPREFIX)/lib/libglib-2.0.so* $(TARGET_DIR)$(EPREFIX)/lib
	-$(TARGET_STRIP) $(TARGET_DIR)$(EPREFIX)/lib/libglib-2.0.so*
	cp -dpf $(HOST_DIR)$(EPREFIX)/lib/libgthread-2.0.so* $(TARGET_DIR)$(EPREFIX)/lib
	-$(TARGET_STRIP) $(TARGET_DIR)$(EPREFIX)/lib/libgthread-2.0.so*
	touch -c $(TARGET_DIR)$(EPREFIX)/lib/libglib-2.0.so

glib: libiconv $(TARGET_DIR)$(EPREFIX)/lib/libglib-2.0.so

glib-source: $(DL_DIR)/$(GLIB_SOURCE)

glib-clean:
	$(MAKE) DESTDIR=$(HOST_DIR) CC=$(TARGET_CC) -C $(GLIB_DIR) uninstall
	-$(MAKE) -C $(GLIB_DIR) clean
	-@rm -f $(TARGET_DIR)$(EPREFIX)/lib/libglib-2.0.so*

glib-dirclean:
	rm -rf $(GLIB_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(PACKAGE_GLIB)),y)
TARGETS+=glib
endif
