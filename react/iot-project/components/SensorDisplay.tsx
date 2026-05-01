"use client";

import { useEffect, useState } from "react";
import { Text, StyleSheet, View, Button} from "react-native";
import mqtt from "mqtt";
import type { IClientOptions, MqttClient } from "mqtt";
import { useMQTT } from "./MQTTClient";
import Subscription from "./Subscription";

type SensorDisplayProp = {
    sensorName: string,
    sensorUnit?: string
}
export const styles = StyleSheet.create({
    header: {
        fontSize: 20,
        fontWeight: "bold",
    },
    container: {
        alignItems: "center",
        width: 250,
        height: 150,
        gap: 6, 
        backgroundColor: "#4c5363", 
        flexDirection: "column",
        display:"flex",
        borderRadius:"12px",
        shadowRadius: 6,
        shadowOffset: {width: 0, height: 2}
    },
    secondaryContainer: {
        alignItems: "center",
        padding:24,
        height: "80%",
        width: "100%",
        gap: 6, 
        backgroundColor: "#3669bc", 
        flexDirection: "column",
        alignSelf: "flex-end",
        display:"flex",
        borderRadius:"6px"
    },
    buttonRow: {
        alignItems: "center",
        flex: 1,
        display:"flex",
        flexDirection: "row",
        verticalAlign:"middle",
    },
    mainText: {
        color: "#F8FAFC"
    },
    secondaryText: {
        color: "#94A3B8",
        textAlign: "center",
    },
    successText: {
        color: "#10B981"
    },
    negativeButton: {
        color: "red"
    }

});


export default function SensorDisplay({sensorName, sensorUnit}: SensorDisplayProp): React.ReactNode {
    // for MQTT
    const { connect, disconnect, subscribe, messages, status } = useMQTT();
    const value = messages[sensorName];

    return (
        <View style={styles.container}>
            <Text style={styles.mainText}>
                {sensorName} Information:
            </Text>
            <View style={styles.secondaryContainer}>
                <Text style={styles.secondaryText}>
                    Sensor Data: {value || 'No data'} {sensorUnit + "\n"}
                </Text>

                <div style={styles.buttonRow}>
                </div>
            </View>
        </View>
    );

}