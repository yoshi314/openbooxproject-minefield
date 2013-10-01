SQLITE_SOURCE=sqlite-amalgamation-3080002.zip
SQLITE_DIR=$(BUILD_DIR)/sqlite-amalgamation-3080002

$(DL_DIR)/$(SQLITE_SOURCE):
#	$(WGET) -P $(DL_DIR) $(DOWNLOAD_SITE)/$(SQLITE_SOURCE)
	$(WGET) -P $(DL_DIR) http://www.sqlite.org/2013/sqlite-amalgamation-3080002.zip

$(SQLITE_DIR)/.unpacked:$(DL_DIR)/$(SQLITE_SOURCE)
	mkdir -p $(SQLITE_DIR)
	unzip -d $(BUILD_DIR) $(DL_DIR)/$(SQLITE_SOURCE)
	touch $(SQLITE_DIR)/.unpacked

$(SQLITE_DIR)/libsqlite3.so.0.8.6:$(SQLITE_DIR)/.unpacked
	(cd $(SQLITE_DIR);$(TARGET_CC) -fpic -g -c -Wall sqlite3.c)
	(cd $(SQLITE_DIR);$(TARGET_CC) -shared -Wl,-soname,libsqlite3.so.0 -o libsqlite3.so.0.8.6 sqlite3.o -lpthread -ldl)
	(cd $(SQLITE_DIR);ln -s libsqlite3.so.0.8.6 libsqlite3.so.0)
	(cd $(SQLITE_DIR);ln -s libsqlite3.so.0.8.6 libsqlite3.so)

$(SQLITE_DIR)/sqlite3:$(SQLITE_DIR)/libsqlite3.so.0.8.6
	(cd $(SQLITE_DIR);$(TARGET_CC) sqlite3.h shell.c sqlite3.o -o sqlite3 -lpthread -ldl)


$(HOST_DIR)$(EPREFIX)/lib/libsqlite3.so.0.8.6:$(SQLITE_DIR)/libsqlite3.so.0.8.6
	cp -dpf $(SQLITE_DIR)/sqlite3.h $(HOST_DIR)$(EPREFIX)/include
	cp -dpf $(SQLITE_DIR)/sqlite3ext.h $(HOST_DIR)$(EPREFIX)/include
	cp -dpf $(SQLITE_DIR)/libsqlite3.so* $(HOST_DIR)$(EPREFIX)/lib
	touch -c $(HOST_DIR)$(EPREFIX)/lib/libsqlite3.so.0.8.6

$(HOST_DIR)$(EPREFIX)/bin/sqlite3:$(SQLITE_DIR)/sqlite3
	cp -dpf $(SQLITE_DIR)/sqlite3 $(HOST_DIR)$(EPREFIX)/bin

$(TARGET_DIR)$(EPREFIX)/lib/libsqlite3.so.0.8.6:$(HOST_DIR)$(EPREFIX)/lib/libsqlite3.so.0.8.6
	cp -dpf $(HOST_DIR)$(EPREFIX)/lib/libsqlite3.so* $(TARGET_DIR)$(EPREFIX)/lib
	-$(TARGET_STRIP) $(TARGET_DIR)$(EPREFIX)/lib/libsqlite3.so.0.8.6
	touch -c $(TARGET_DIR)$(EPREFIX)/lib/libsqlite3.so.0.8.6

$(TARGET_DIR)$(EPREFIX)/bin/sqlite3:$(HOST_DIR)$(EPREFIX)/bin/sqlite3
	cp -dpf $(HOST_DIR)$(EPREFIX)/bin/sqlite3 $(TARGET_DIR)$(EPREFIX)/bin

sqlite: $(TARGET_DIR)$(EPREFIX)/lib/libsqlite3.so.0.8.6 $(TARGET_DIR)$(EPREFIX)/bin/sqlite3

sqlite-app: $(SQLITE_DIR)/sqlite3

sqlite-source:$(DL_DIR)/$(SQLITE_SOURCE)

sqlite-clean:
	-@rm -f $(SQLITE_DIR)/libsqlite3.so*
	-@rm -f $(HOST_DIR)$(EPREFIX)/lib/libsqlite3.so*
	-@rm -f $(TARGET_DIR)$(EPREFIX)/lib/libsqlite3.so*
	
sqlite-dirclean:
	rm -rf $(SQLITE_DIR)


#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(PACKAGE_SQLITE)),y)
TARGETS+=sqlite
endif
