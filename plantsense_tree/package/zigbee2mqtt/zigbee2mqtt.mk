################################################################################
#
# zigbee2mqtt
#
################################################################################

# ZIGBEE2MQTT_VERSION=
ZIGBEE2MQTT_SITE=https://github.com/Koenkk/zigbee2mqtt.git
ZIGBEE2MQTT_SITE_METHOD = git

define ZIGBEE2MQTT_BUILD_CMDS
	# pass
endef

define ZIGBEE2MQTT_TARGET_INSTALL_CMDS
	mkdir /opt/zigbee2mqtt
	## 
endef

$(eval $(generic-package))