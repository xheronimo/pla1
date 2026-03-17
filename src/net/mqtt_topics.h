#pragma once

// ==============================
// BASE
// ==============================
#define MQTT_TOPIC_BASE          "device"

// ==============================
// SISTEMA
// ==============================
#define MQTT_TOPIC_STATUS        MQTT_TOPIC_BASE "/status"
#define MQTT_TOPIC_HEARTBEAT     MQTT_TOPIC_BASE "/heartbeat"

// ==============================
// SEÑALES
// ==============================
#define MQTT_TOPIC_SIGNAL        MQTT_TOPIC_BASE "/signal"
#define MQTT_TOPIC_SIGNAL_VALUE  MQTT_TOPIC_BASE "/signal/value"

// ==============================
// ALARMAS
// ==============================
#define MQTT_TOPIC_ALARM         MQTT_TOPIC_BASE "/alarm/event"


#define MQTT_TOPIC_ALARM_ACK   "alarms/ack"

#define MQTT_TOPIC_ALARM_STATE "alarms/state"
#define MQTT_TOPIC_ALARM_SNAPSHOT "alarms/snapshot"

#define MQTT_TOPIC_ALARM_EVENT   "alarms/event"


