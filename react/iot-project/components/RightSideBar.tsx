"use client";

import { Text, StyleSheet, View, Button, TextInput, ScrollView, Pressable} from "react-native";
import SentMessage, { MessageProp } from "./SentMessage";
import { useEffect, useRef, useState } from "react";
import { useMQTT } from "./MQTTClient";
import SimpleLineIcons from '@expo/vector-icons/SimpleLineIcons';
import EnableSideBar from "./EnableSideBar";

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
    hiddenSidebar: {
        display:"flex",
        flexDirection:"column",
        justifyContent:"flex-start",
        verticalAlign:"top",    
        gap: 12,
        flexWrap:"wrap",
        width: 50,
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

    const [hidden, setHidden] = useState(false);
    const [textMessages, setTextMessages] = useState<MessageProp[]>([]);
    const [text, setText] = useState("");
    const scrollViewReference = useRef<ScrollView | null>(null);
    const { connect, disconnect, subscribe, messages, status, publish } = useMQTT();

    useEffect(() => {
        // if a new message pops up, scroll to the bottom
        if(scrollViewReference.current) {
            scrollViewReference.current.scrollToEnd({animated: true});
        }
        // update by sending message to mqtt client
        if(status === "connected" && subscribe.length > 0) {
            publish("mesh", textMessages[textMessages.length - 1].message);
        }
        
    }, [textMessages])

    const handlePress = () => {
        // empty out the text, send it into the messages
        if(text.length > 0) {
            setTextMessages(previous => [
                ...previous,
                {message: text, timeStamp: new Date()}
            ]);
            setText("");
        }
    }
    return ( hidden ? 
        <View style={Style.sidebar}>
            <View style={{flexDirection: "row", gap: 16}}>
                <EnableSideBar hidden={hidden} setHidden={ setHidden }/>
                <Text style={{textAlign:"center", fontSize: 24}}>
                    Messages 
                </Text>
            </View>


            <ScrollView style = {Style.scrollView} ref={scrollViewReference}>
                {
                    textMessages.map((msg: MessageProp, idx: number) => (
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
        </View> :
        <View style={Style.hiddenSidebar}>
            <EnableSideBar hidden={hidden} setHidden={ setHidden }/>
        </View>
    )
}
