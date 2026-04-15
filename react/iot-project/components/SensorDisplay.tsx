"use client";

import { useEffect, useState } from "react";
import { Text, StyleSheet, View, Button} from "react-native";
import mqtt from "mqtt";
import type { IClientOptions, MqttClient } from "mqtt";

type SensorDisplayProp = {
    sensorInformation: string,
    sensorName: string,
    sensorUnit?: string
}
const styles = StyleSheet.create({
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
        gap: 6, 
        backgroundColor: "#3669bc", 
        flexDirection: "column",
        alignSelf: "flex-end",
        display:"flex",
        borderRadius:"6px"
    },
    buttonRow: {
        alignItems: "center",
        justifyContent:"space-between", 
        flex: 1,
        display:"flex",
        flexDirection: "row"
    },
    mainText: {
        color: "#F8FAFC"
    },
    secondaryText: {
        color: "#94A3B8"
    },
    successText: {
        color: "#10B981"
    },
    negativeButton: {
        color: "red"
    }

});
function useMQTTSubscribe(client: any, topic: any, onMessage: any) {
  useEffect(() => {
    if (!client || !client.connected) return;
    const handleMsg = (receivedTopic: any, message: any) => {
      if (receivedTopic === topic) {
        onMessage(message.toString());
      }
    };
    client.subscribe(topic);
    client.on('message', handleMsg);
    return () => {
      client.unsubscribe(topic);
      client.off('message', handleMsg);
    };
  }, [client, topic, onMessage]);
}


export default function SensorDisplay({sensorName, sensorUnit}: SensorDisplayProp): React.ReactNode {
    const [sensorInformation, setSensorInformation] = useState("");
    const [client, setClient] = useState<MqttClient | null>(null);

    const [connectStatus, setConnectStatus] = useState("");
    const [payload, setPayload] = useState<{ topic: string; message: string } | null>(null);
    
    // for the button
    const [enable, setEnable] = useState(true);
    const handlePress = () => {
        if (!client) {
            const newClient = mqtt.connect("wss://test.mosquitto.org:8081", {});
            setClient(newClient);

            newClient.on('connect', () => {
                setEnable(false);
                newClient.subscribe("mesh");
                console.log("got to connecting!");
            });

            newClient.on('disconnect', () => {
                setEnable(true);
            })
            newClient.on('message', (topic, message) => {
                if (topic === "mesh") {
                    let recievedMessage = message.toString();
                    let command = recievedMessage.split(":");
                    if(command[0] === sensorName)
                    {
                        recievedMessage = recievedMessage.replace(command[0] + ":", "");
                        setPayload({ topic, message: recievedMessage });
                    }
                    else {
                        console.log(command[1]);
                    }
                    //console.log("trying to set the payload!");
                }
            });

            newClient.on('error', (err) => {
                console.error('Connection error: ', err);
                newClient.end();
            });
        }
    };
        // Clean up on unmount
    useEffect(() => {
        return () => {
            if (client) {
                client.end();
            }
        };
    }, [client]);
    const handleDisconnect = () => {
        if(client) {
            client.end(() => {
                console.log("Connection terminated");
                setEnable(true);
                setClient(null);
            });
        }
    }

    return (
        <View style={styles.container}>
            <Text style={styles.mainText}>
                {sensorName} Information:
            </Text>
            <View style={styles.secondaryContainer}>
                <Text style={styles.secondaryText}>
                    Sensor Data: {payload?.message || 'No data'} {sensorUnit}
                </Text>

                <div style={styles.buttonRow}>
                    <Button title="Subscribe" onPress={handlePress} disabled={!enable}/>
                    <Button title="Disconnect" onPress={handleDisconnect} disabled={enable} color={styles.negativeButton.color}/>
                </div>
            </View>
        </View>
    );

}