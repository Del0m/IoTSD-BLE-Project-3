"use client";
import { useEffect, useState } from "react";
import { Text, StyleSheet, View, Button} from "react-native";
import mqtt from "mqtt";
import type { IClientOptions, MqttClient } from "mqtt";
import { useMQTT } from "./MQTTClient";
import { styles } from "./SensorDisplay";
import { Badge } from "@react-navigation/elements";


export default function Subscription() {
    const { connect, disconnect, subscribe, messages, status } = useMQTT();
    return (
        <div style={styles.buttonRow}>
            <Button title="Subscribe" onPress={() => { connect(); subscribe("mesh"); }} disabled={status === "connected"}/>
            <Button title="Disconnect" onPress={() => { disconnect() }} disabled={status !== "connected"} color={styles.negativeButton.color}/>
        </div>
    );
}