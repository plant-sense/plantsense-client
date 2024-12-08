---
title: "Z2m samples"
---

# Health Check

## Query

A:
zigbee2mqtt/bridge/request/health_check
P:
// empty

## Response

A:
zigbee2mqtt/bridge/response/health_check
P:

```json
{
    "data": {
        "healthy" : true
    },
    "status" : "ok"
}
```

# Devices

## Respone

**persistent**
A:
zigbee2mqtt/bridge/devices
P:

```json
    [
    {
        "ieee_address":"0x00158d00018255df",
        "type":"Router",
        "network_address":29159,
        "supported":true,
        "disabled": false,
        "friendly_name":"my_plug",
        "description":"this plug is in the kitchen",
        "endpoints":{"1":{"bindings":[],"configured_reportings":[],"clusters":{"input":["genOnOff","genBasic"],"output":[]}}},
        "definition":{
            "model":"ZNCZ02LM",
            "vendor":"Xiaomi",
            "description":"Mi power plug ZigBee",
            "options": [], 
            "exposes": []  
        },
        "power_source":"Mains (single phase)",
        "date_code":"02-28-2017",
        "model_id":"lumi.plug",
        "scenes": [{"id": 3, "name": "Chill scene"}],
        "interviewing":false,
        "interview_completed":true
    },
    {
        "ieee_address":"0x90fd9ffffe6494fc",
        "type":"Router",
        "network_address":57440,
        "supported":true,
        "disabled": false,
        "friendly_name":"my_bulb",
        "endpoints":{"1":{"bindings":[],"configured_reportings":[],"clusters":{"input":["genOnOff","genBasic","genLevelCtrl"],"output":["genOta"]}}},
        "definition":{
            "model":"LED1624G9",
            "vendor":"IKEA",
            "description":"TRADFRI LED bulb E14/E26/E27 600 lumen, dimmable, color, opal white",
            "options": [], 
            "exposes": [
                {
                    "type": "numeric",
                    "name": "temperature",
                    "label": "Temperature",
                    "property": "temperature",
                    "unit": "°C"
                },
                {
                    "type": "soil_moisture",
                    "name": "soil_moisture",
                    "label": "Soil Moisture",
                    "property": "soil_moisture",
                    "unit": "%"
                },
                {
                    "type": "enum",
                    "name": "temperature_unit",
                    "label": "temperature_unit",
                    "property": "temperature_unit",
                    "values": ["celsius", "fahrenheit"],
                    "access": 5
                }
            ]  // see exposes/options below
        },
        "power_source":"Mains (single phase)",
        "software_build_id":"1.3.009",
        "model_id":"TRADFRI bulb E27 CWS opal 600lm",
        "scenes": [],
        "date_code":"20180410",
        "interviewing":false,
        "interview_completed":true
    },
    {
        "ieee_address":"0x00169a00022256da",
        "type":"Router",
        "endpoints":{
          "1":{
            "bindings":[
              {"cluster":"genOnOff","target":{"type":"endpoint","endpoint":1,"ieee_address":"0x000b57fffec6a5b3"}},
              {"cluster":"genOnOff","target":{"type":"group","id":1}},
            ],
            "configured_reportings":[
              {"cluster":"genOnOff","attribute":"onOff","maximum_report_interval":10,"minimum_report_interval":1,"reportable_change":1}
            ],
            "clusters":{"input":["genBasic","msIlluminanceMeasurement"],"output":["genOnOff"]}
          }
        },
        "network_address":22160,
        "supported":false,
        "disabled": false,
        "friendly_name":"my_sensor",
        "definition":null,
        "power_source":"Battery",
        "date_code":"04-28-2019",
        "model_id":null,
        "scenes": [],
        "interviewing":false,
        "interview_completed":true
    },
    {
        "ieee_address":"0x00124b00120144ae",
        "type":"Coordinator",
        "network_address":0,
        "supported":false,
        "disabled": false,
        "endpoints":{"1":{"bindings":[],"configured_reportings":[],"clusters":{"input":[],"output":[]}}},
        "friendly_name":"Coordinator",
        "definition":null,
        "power_source":null,
        "date_code":null,
        "scenes": [],
        "model_id":null,
        "interviewing":false,
        "interview_completed":true
    },
]

```

# Sensor reading

A:
zigbee2mqtt/FRIENDLY_NAME
P:

```json
{
    "temperature": 21.37,
    "soil_moisture": 69
}
```
