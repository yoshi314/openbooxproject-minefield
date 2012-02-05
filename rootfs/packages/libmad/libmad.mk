LIBMAD_SOURCE=libmad-0.15.1b.tar.gz
LIBMAD_DIR=$(BUILD_DIR)/libmad-0.15.1b
LIBMAD_CFLAGS=-fPIC

$(DL_DIR)/$(LIBMAD_SOURCE):
	mkdir -p $(DL_DIR)
	$(WGET) -P $(DL_DIR) $(DOWNLOAD_SITE)/$(LIBMAD_SOURCE)

$(LIBMAD_DIR)/.unpacked: $(DL_DIR)/$(LIBMAD_SOURCE)
	mkdir -p $(BUILD_DIR)
	tar -C $(BUILD_DIR) -zxf $(DL_DIR)/$(LIBMAD_SOURCE)
	touch $(LIBMAD_DIR)/.unpacked

$(LIBMAD_DIR)/.configured: $(LIBMAD_DIR)/.unpacked
	(cd $(LIBMAD_DIR);\
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		./configure \
		--target=arm-none-linux-gnueabi \
		--host=arm-none-linux-gnueabi \
		--build=$(GNU_HOST_NAME) \
		--prefix=/ \
		--exec-prefix=$(EPREFIX) \
		--includedir=$(EPREFIX)/include \
		--localstatedir=/var \
		--enable-fpm=arm \
		--disable-aso \
		--disable-debugging \
		--enable-shared \
		--enable-static \
	);
	touch $(LIBMAD_DIR)/.configured

$(LIBMAD_DIR)/.libs/libmad.so.0.2.1: $(LIBMAD_DIR)/.configured
	eval "perl -i -p -e 's/\-march=i486/\-march=armv4/g' $(LIBMAD_DIR)/Makefile"
	$(MAKE) CC=$(TARGET_CC) -C $(LIBMAD_DIR) all

$(HOST_DIR)$(EPREFIX)/lib/libmad.so.0.2.1: $(LIBMAD_DIR)/.libs/libmad.so.0.2.1
	$(MAKE) DESTDIR=$(HOST_DIR) -C $(LIBMAD_DIR) install
	touch -c $(HOST_DIR)$(EPREFIX)/lib/libmad.so.0.2.1

$(TARGET_DIR)$(EPREFIX)/lib/libmad.so.0.2.1: $(HOST_DIR)$(EPREFIX)/lib/libmad.so.0.2.1
	cp -dpf $(HOST_DIR)$(EPREFIX)/lib/libmad.so* $(TARGET_DIR)$(EPREFIX)/lib/
	$(TARGET_STRIP) --strip-unneeded $(TARGET_DIR)$(EPREFIX)/lib/libmad.so*
	touch -c $(TARGET_DIR)$(EPREFIX)/lib/libmad.so.0.2.1

libmad: $(TARGET_DIR)$(EPREFIX)/lib/libmad.so.0.2.1

libmad-source: $(DL_DIR)/$(LIBMAD_SOURCE)

libmad-clean:
	-$(MAKE) -C  $(LIBMAD_DIR) clean
	-@rm -f $(TARGET_DIR)$(EPREFIX)/lib/libmad.so*
	-@rm -f $(HOST_DIR)$(EPREFIX)/lib/libmad.so*
	-@rm -f $(HOST_DIR)$(EPREFIX)/lib/libmad.a
	-@rm -f $(LIBMAD_DIR)/.configured

libmad-dirclean:
	rm -rf $(LIBMAD_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(PACKAGE_LIBMAD)),y)
TARGETS+=libmad
endif
