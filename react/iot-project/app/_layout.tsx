import { MQTTProvider } from "@/components/MQTTClient";
import { Stack } from "expo-router";


export const sensors = ["thermometer a", "thermometer b"];
export default function RootLayout() {
  return (
    <MQTTProvider sensors={sensors}>
      <Stack/>
    </MQTTProvider>
  )
}
