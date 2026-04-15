import RightSideBar from "@/components/RightSideBar";
import SensorDisplay from "@/components/SensorDisplay";
import { Text, View } from "react-native";

export default function Index() {
  return (
    <View
      style={{
        flex: 1,
        flexDirection:"row",
      }}
    >
      <View style={{flex: 4, padding: 24, justifyContent:"space-evenly", flexDirection: "row", flexWrap:"wrap"}}>
        <SensorDisplay sensorInformation={"payload?.message"} sensorName="Thermometer" sensorUnit="C"/>
        <SensorDisplay sensorInformation={"payload?.message"} sensorName="Thermometer" sensorUnit="C"/>
      </View>

      <RightSideBar/>
    </View>
  );
}
