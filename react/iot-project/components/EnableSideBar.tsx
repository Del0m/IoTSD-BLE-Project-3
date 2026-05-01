"use client";

import SimpleLineIcons from "@expo/vector-icons/SimpleLineIcons";
import { Dispatch, SetStateAction } from "react";
import { Pressable } from "react-native";


type Prop = {
  hidden: boolean;
  setHidden: (value: boolean) => void;
};
export default function EnableSideBar({ hidden, setHidden }: Prop) {
    return (
        <Pressable 
            onPress={() => setHidden(!hidden)} 
            style={{
                backgroundColor:"#007AFF", 
                borderRadius: 6, 
                padding: 2
                }}
            >
            <SimpleLineIcons name="menu" size={24} color="white"/>
        </Pressable>

    );
}