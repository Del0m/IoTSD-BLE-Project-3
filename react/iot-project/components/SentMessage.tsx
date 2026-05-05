"use client";

import { View, Text } from "react-native";


export type MessageProp = {
    message: string;
    timeStamp: Date;
}

export default function SentMessage({ message, timeStamp }: MessageProp) {
    return(
        <View style = {{
            display:"flex",
            backgroundColor:"white",
            borderRadius:6,
            shadowRadius:6,
            padding: 16,
            flexDirection:"column",
            flexWrap:"wrap",
            marginBottom:6,
            alignSelf:"stretch",
            width:"100%"

        }}>
            <Text>{message}</Text>
            
            <View style={{
                position:'absolute',
                top:0, right: 0,
            }}>
                <Text style= {{
                    fontSize: 12,
                    color:"#b8b8b8",
                    padding:2,                    
                }}>
                    {timeStamp.toLocaleTimeString()}
                </Text>
            </View>
        </View>
    )
}