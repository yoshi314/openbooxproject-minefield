#############################################################
#
# Network Time Protocol (NTP) client
#
#############################################################
NTPCLIENT_SOURCE=ntpclient_2010_365.tar.gz
NTPCLIENT_DIR:=$(BUILD_DIR)/ntpclient-2010

$(DL_DIR)/$(NTPCLIENT_SOURCE):
	$(WGET) -P $(DL_DIR) $(DOWNLOAD_SITE)/$(NTPCLIENT_SOURCE)

$(NTPCLIENT_DIR)/.unpacked: $(DL_DIR)/$(NTPCLIENT_SOURCE)
	tar -C $(BUILD_DIR) -xzf $(DL_DIR)/$(NTPCLIENT_SOURCE)
	touch $(NTPCLIENT_DIR)/.unpacked

$(NTPCLIENT_DIR)/ntpclient: $(NTPCLIENT_DIR)/.unpacked
	$(MAKE) CC=$(TARGET_CC) -C $(NTPCLIENT_DIR)
	touch -c $(NTPCLIENT_DIR)/ntpclient
	
$(TARGET_DIR)/usr/bin/ntpclient: $(NTPCLIENT_DIR)/ntpclient
	cp -dpf $(NTPCLIENT_DIR)/ntpclient $(TARGET_DIR)/usr/bin
	-$(TARGET_STRIP) $(TARGET_DIR)/usr/bin/ntpclient
	cp -dpf $(BASE_DIR)/packages/ntpclient/ntp $(TARGET_DIR)/etc/rc.d/init.d
	touch -c $(TARGET_DIR)/usr/bin/ntpclient

ntpclient: $(TARGET_DIR)/usr/bin/ntpclient

ntpclient-source: $(DL_DIR)/$(NTPCLIENT_SOURCE)

ntpclient-clean:
	-$(MAKE) -C $(NTPCLIENT_DIR) clean
	-@rm -f $(TARGET_DIR)/usr/bin/ntpclient
	-@rm -f $(TARGET_DIR)/etc/rc.d/init.d/ntp

ntpclient-dirclean:
	rm -rf $(NTPCLIENT_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(PACKAGE_NTPCLIENT)),y)
TARGETS+=ntpclient
endif
