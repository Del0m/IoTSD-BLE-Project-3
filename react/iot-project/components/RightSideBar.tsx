"use client";

import { Text, StyleSheet, View, Button, TextInput, ScrollView} from "react-native";
import SentMessage, { MessageProp } from "./SentMessage";
import { useEffect, useRef, useState } from "react";

const Style = StyleSheet.create({

    sidebar: {
        display:"flex",
        flexDirection:"column",
        justifyContent:"flex-end",
        gap: 12,
        flexWrap:"wrap",
        width: 300,
        backgroundColor:"#cacaca",
    },
    textSection: {
        display:"flex",
        flexDirection:"row",
        gap:4,
    },
    scrollView: {
        flex:1,
        padding: 12, 
        width:"100%", 
        alignSelf:"flex-end"
    },
    textInput: {
        backgroundColor: "white",
        color:"black",
        borderWidth:2,
        borderRadius:12,
        borderColor:"#c5c5c5",
        width: "100%",
        minHeight: 10,
        height: 50
    }
})

export default function RightSideBar() {

    // TODO: onSetMessageUpdate send out an mqtt message from MQTTClient.tsx
    const [messages, setMessages] = useState<MessageProp[]>([]);
    const [text, setText] = useState("");
    const scrollViewReference = useRef<ScrollView | null>(null);

    useEffect(() => {
        // if a new message pops up, scroll to the bottom
        if(scrollViewReference.current) {
            scrollViewReference.current.scrollToEnd({animated: true});
        }
    }, [messages])

    const handlePress = () => {
        // empty out the text, send it into the messages
        if(text.length > 0) {
            setMessages(previous => [
                ...previous,
                {message: text, timeStamp: new Date()}
            ]);
            setText("");
        }
    }
    return (
        <View style={Style.sidebar}>
            <Text style={{textAlign:"center", fontSize: 24}}>
                Messages
            </Text>
            <ScrollView style = {Style.scrollView} ref={scrollViewReference}>
                {
                    messages.map((msg, idx) => (
                        <SentMessage key={idx} 
                        message={msg.message} 
                        timeStamp={msg.timeStamp}
                        />
                    ))
                }
            </ScrollView>
            <View style={Style.textSection}>
                <TextInput style={Style.textInput} multiline value={text} onChangeText={value => setText(value)}/>
                <Button title="Send" onPress={handlePress}/>
            </View>
        </View>
    )
}
