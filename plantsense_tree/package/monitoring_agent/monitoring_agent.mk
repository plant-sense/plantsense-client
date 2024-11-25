################################################################################
#
# monitoring_agent
#
################################################################################

MONITORING_AGENT_VERSION = 0.1
MONITORING_AGENT_SITE = $(BR2_EXTERNAL_plantsense_tree_PATH)/package/minitoring_agent/src
MONITORING_AGENT_METHOD = local

define MONITORING_AGENT_BUILD_CMDS
	$(MAKE) -C $(@D)
endef

define MONITORING_AGENT_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/monitoring_agent $(TARGET_DIR)/bin
endef

$(eval $(generic-package))
