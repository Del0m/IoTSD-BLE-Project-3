import { MQTTProvider, useMQTT } from "@/components/MQTTClient";
import RightSideBar from "@/components/RightSideBar";
import SensorDisplay from "@/components/SensorDisplay";
import { Text, View } from "react-native";
import { useNavigation } from "expo-router";
import { useEffect, useState } from "react";
import { Button } from "@react-navigation/elements";
import Subscription from "@/components/Subscription";
import { sensors } from "./_layout";
import StatusLabel from "@/components/StatusLabel";
export default function Index() {
  const navigation = useNavigation();
  const [hidden, setHidden] = useState(false);

  useEffect(() => {
    navigation.setOptions({
      title: "IoT Project 3",
      headerLeft: () => <Subscription/>,
      headerRight: () => <StatusLabel/>
    });
  }, [navigation]);


  return (
      <View
        style={{
          flex: 1,
          flexDirection:"row",
        }}
      >
        <View style={{flex: 4, padding: 24, justifyContent:"space-evenly", flexDirection: "row", flexWrap:"wrap"}}>
          {sensors.map((sensor) => (
            <SensorDisplay 
            key={sensor.name}
            name={sensor.name} 
            sensors={sensor.sensors}
            />
          ))}
        </View>
        <RightSideBar/>
      </View>
  );
}
