"use client";

import { useEffect, useState } from "react";
import { Text, StyleSheet, View, Button} from "react-native";
import mqtt from "mqtt";
import type { IClientOptions, MqttClient } from "mqtt";
import { useMQTT } from "./MQTTClient";
import Subscription from "./Subscription";

export type SensorDisplay = {
    sensorName: string,
    sensorUnit?: string,
    frequency? : number,
    show?: boolean
}
export type BoardDisplayProp = {
    name: string,
    sensors: SensorDisplay[];
}
export const styles = StyleSheet.create({
    header: {
        fontSize: 20,
        fontWeight: "bold",
    },
    container: {
        alignItems: "center",
        width: 300,
        height: 200,
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
        justifyContent: "center",
        //flex: 1,
        gap: 12,
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


export default function SensorDisplay({name, sensors}: BoardDisplayProp): React.ReactNode {
    // for MQTT
    const { connect, disconnect, publish, subscribe, messages, status } = useMQTT();
    const [led0, setLed0] = useState(false);
    const [led1, setLed1] = useState(false);

    // send a message when we enable leds
    useEffect(() => {
    if (status === "connected") {
        publish(
        "WebToMesh",
        JSON.stringify({
            boardName: name,
            led: "led0",
            state: led0,
        })
        );
    }
    }, [led0]);

    useEffect(() => {
    if (status === "connected") {
        publish(
        "WebToMesh",
        JSON.stringify({
            boardName: name,
            led: "led1",
            state: led1,
        })
        );
    }
    }, [led1]);

    return (
        <View style={styles.container}>
            <Text style={styles.mainText}>
                {name} Information:
            </Text>
            <View style={styles.secondaryContainer}>
            {sensors.map((sensor) => {
                const sensorValue = messages[`${name}:${sensor.sensorName}`];
                const sensorFrequency = messages[`${name}:${sensor.sensorName}:frequency`];

                return ( sensor.show !== false ?
                <Text key={sensor.sensorName} style={styles.secondaryText}>
                    {sensor.sensorName}: {sensorValue || "No data"} {sensor.sensorUnit || ""} {sensor.sensorUnit?.length !== 0 ? `${sensorFrequency || 'n/a'} ms` : ``}
                </Text> : null
                );
            })}
                <View style={styles.buttonRow}>
                    <Button title={`${led0 ? `disable` : `enable`} led0`} onPress={() => {setLed0(!led0)}}/>
                    <Button title={`${led1 ? `disable` : `enable`} led1`} onPress={() => {setLed1(!led1)}}/>
                </View>
            </View>
        </View>
    );

}