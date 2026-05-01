"use client";
import { Badge } from "@react-navigation/elements";
import { useMQTT } from "./MQTTClient";

export default function StatusLabel() {
    const { status } = useMQTT();
    return(
        <div style={{
            verticalAlign:"middle",
            gap: 24,
            justifyContent:"center"
        }}>
            <Badge visible={true} style={{
                justifyContent:"center",
                backgroundColor: (status === "connected") ? "green" : "red",
                fontSize: 20,
                gap:24,
            }}>
                Status: {status}
            </Badge>
        </div>

    );
}