import { MQTTProvider } from "@/components/MQTTClient";
import { BoardDisplayProp } from "@/components/SensorDisplay";
import { Stack } from "expo-router";


export const sensors: BoardDisplayProp[] = [
  {
  name: "Bluetooth Board 1",
  sensors: [
    { sensorName: "thermometer", sensorUnit: "C"},
    { sensorName: "humidity", sensorUnit: "RH"},
    { sensorName: "lux", sensorUnit: "lumen/m^2"},
    { sensorName: "led0", sensorUnit: "off / on", show: false},
    { sensorName: "led1", sensorUnit: "off / on", show: false}
  ]
  },
  {
  name: "Bluetooth Board 2",
  sensors: [
    { sensorName: "thermometer", sensorUnit: "C"},
    { sensorName: "humidity", sensorUnit: "RH"},
    { sensorName: "lux", sensorUnit: "lumen/m^2"},
    { sensorName: "led0", sensorUnit: "off / on", show: false},
    { sensorName: "led1", sensorUnit: "off / on", show: false}
  ]
  },
]

export default function RootLayout() {
  return (
    <MQTTProvider sensors={sensors}>
      <Stack/>
    </MQTTProvider>
  )
}
