"use client";

import mqtt, { MqttClient } from "mqtt";
import { createContext, useContext, useEffect, useRef, useState } from "react";
import { BoardDisplayProp, SensorDisplay } from "./SensorDisplay";


type MQTTContextType = {
  client: MqttClient | null;
  status: string;
  messages: { [topic: string]: string };
  connect: () => void;
  disconnect: () => void;
  subscribe: (topic: string) => void;
  unsubscribe: (topic: string) => void;
  publish: (topic: string, message: string) => void;
};

const MQTTContext = createContext<MQTTContextType | undefined>(undefined);
export const MQTTProvider: React.FC<{children: React.ReactNode, sensors: BoardDisplayProp[]}> = ({ children, sensors }) => {
    const [client, setClient] = useState<MqttClient | null>(null);
    const [status, setStatus] = useState("disconnected");
    const [messages, setMessages] = useState<{ [sensor: string]: string }>({});
    const subscriptions = useRef<Set<string>>(new Set());

    // connect to the broker
    const connect = () => {
        if(client) return; // already connected

        const newClient = mqtt.connect("wss://broker.hivemq.com:8884/mqtt", {});
        setClient(newClient);
        newClient.on('connect', () => {
            setStatus("connected");
            newClient.subscribe("WebToMesh");
            newClient.subscribe("MeshToWeb");

            console.log("got to connecting!");
        });    
        newClient.on('disconnect', () => {
            setStatus("disconnected");
        })
        newClient.on('message', (topic, message) => {
            if (topic === "MeshToWeb") {
                try {
                    const payload = JSON.parse(message.toString());

                    const board = sensors.find((board) => board.name === payload.boardName);
                    if(board)
                    {
                        const updates: Record<string, string> = {};
                        payload.sensors.forEach((sensor: any) => {
                            updates[`${payload.boardName}:${sensor.sensorName}`] = String(sensor.value);
                            updates[`${payload.boardName}:${sensor.sensorName}:frequency`] = String(sensor.frequency || 'n/a');

                        });
                        setMessages((prev) => ({ ...prev, ...updates }));
                        console.log(`successfully received: ${payload.toString()}`);
                    }
                    else {
                        console.log(`failed to receive: ${payload.toString()}`);
                    }
                } catch (error) {
                   console.log(`Please enter a json format. Entered ${message.toString()}`); 
                }
            }
        });
        
        newClient.on('error', (err) => {
            console.error('Connection error: ', err);
            newClient.end();
        });
    }

    // disconnect from broker
    const disconnect = () => {
        if(client) {
            client.end(() => {
                console.log("Connection terminated");
                setStatus("disconnected");
                setMessages({});
                setClient(null);
                subscriptions.current.clear();
            });
        }
    };

    // subscribe to topic
    const subscribe = (topic: string) => {
        if(client && status === "connected" && !subscriptions.current.has(topic)) {
            client.subscribe(topic);
            subscriptions.current.add(topic);
        }
    };

    // unsubscribe to topic
    const unsubscribe = (topic: string) => {
        if(client && subscriptions.current.has(topic)) {
            client.unsubscribe(topic);
            subscriptions.current.delete(topic);

            // clean out messages from the topic
            setMessages((prev) => {
                const copy = { ...prev };
                delete copy[topic];
                return copy;
            })
        }
    };

    // publish a message
    const publish = (topic: string, message: string) => {
        if(client && status === "connected") {
            client.publish(topic, message);
        }
    };

    // clean up on unmount
    useEffect(() => {
        return () => {
            if (client) {
                client.end();
            }
        };
    }, [client]);

    return (
        <MQTTContext.Provider
            value={{client, status, messages, connect, disconnect, subscribe, unsubscribe, publish }}
        >
            {children}
        </MQTTContext.Provider>
    );
};
export const useMQTT = () => {
    const context = useContext(MQTTContext);
    if (!context) throw new Error("useMQTT needs to be used with a provider");
    return context;
}
