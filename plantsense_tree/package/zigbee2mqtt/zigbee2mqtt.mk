################################################################################
#
# zigbee2mqtt
#
################################################################################

# ZIGBEE2MQTT_VERSION=
ZIGBEE2MQTT_SITE=https://github.com/Koenkk/zigbee2mqtt.git
ZIGBEE2MQTT_SITE_METHOD = git

define ZIGBEE2MQTT_BUILD_CMDS
	npm ci --prefix $(@D)
	npm run build --prefix $(@D)
endef

define ZIGBEE2MQTT_TARGET_INSTALL_CMDS
	mkdir /opt/zigbee2mqtt
	cp $(@D)/* $(TARGET_DIR)/opt/zigbee2mqtt
	cp -r $(@D)/data $(TARGET_DIR)/opt/zigbee2mqtt/data
	cp -r $(@D)/dist $(TARGET_DIR)/opt/zigbee2mqtt/dist
	cp -r $(@D)/lib $(TARGET_DIR)/opt/zigbee2mqtt/lib
	cp -r $(@D)/node_modules $(TARGET_DIR)/opt/zigbee2mqtt/node_modules
	cp -r $(@D)/scripts $(TARGET_DIR)/opt/zigbee2mqtt/scripts
	cp -r $(@D)/images $(TARGET_DIR)/opt/zigbee2mqtt/images
	## 
endef

$(eval $(generic-package))