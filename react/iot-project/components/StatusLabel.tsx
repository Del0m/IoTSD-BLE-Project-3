"use client";
import { Badge } from "@react-navigation/elements";
import { useMQTT } from "./MQTTClient";
import { View } from "react-native";

export default function StatusLabel() {
    const { status } = useMQTT();
    return(
        <View style={{
            alignItems: "center",
            gap: 24,
            justifyContent:"center"
        }}>
            <Badge visible={true} style={{
                justifyContent:"center",
                backgroundColor: (status === "connected") ? "green" : "red",
                fontSize: 20,
                gap:24,
            }}>
                {`Status: ${status}`}
            </Badge>
        </View>

    );
}