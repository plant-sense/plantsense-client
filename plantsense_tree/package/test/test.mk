################################################################################
#
# test
#
################################################################################

TEST_VERSION = 1.0
TEST_SITE = $(BR2_EXTERNAL_plantsense_tree_PATH)/package/test/src
TEST_INSTALL_STAGING=NO
TEST_SITE_METHOD = local

# define NANOMQ_INSTALL_STAGING_CMDS
# 	pwd
# 	ls -la .
# endef

define TEST_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
endef

define TEST_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/test $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
