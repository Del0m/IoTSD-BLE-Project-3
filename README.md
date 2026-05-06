# Project 3 -- Final Project
For our project, we had a series of design decisions while making this project. Me and Nicholas learned quite a bit, managing TCP / IP protocols, and BLE mesh protocols. Let's start from the ground up and talk about how this project works.
## Versioning Information
### BLE Mesh Board SDKs
Simplicity SDK Suite v2025.12.
- EmberZNet 9.0.1.0
- Multiprotocol 1.0.1.0
- OpenThread 3.0.1.0 (GitHub-61e43cffb)
- Platform 5.2.0.0, SDK
- Wi-Fi SDK 4.0.1-alpha.71
- Z-Wave SDK 8.0.1.0
### Wi-Fi Board SDKs
 Simplicity SDK Suite v2025.6.3
 - Bluetooth 10.1.2
 - Bluetooth Mesh 9.1.0
 - Connect 4.1.3
 - EmberZNet 8.2.2.0
 - Micrium OS Kernel 5.18.02
 - OpenThread 2.7.3.0 (GitHub-fb0446f53)
 - Platform 5.2.3.0
 - RAIL 2.19.3
 - USB 1.5.1.0
 - Wi-Fi SDK 3.5.2
 - Wi-SUN 2.9.2
 - Z-Wave SDK 7.24.3.0
Version: 2025.6.3

## Components & Design Decisions
### EFR32xG24 Explorer Kits
For these two kits, they took part in the BLE mesh. Each of them had a daisy-chained VEML7700 Lux sensor, connected to a SHT40 temperature and RH sensor. For the BLE mesh, there was a proxy and a node, where the node exclusively communicated to the BTmesh proxy. The poxy node has a BLE connection that was suppopsed connect to the SiWWx917Y. The information was sent by notifications by updating the GATT table. The different functions are only really created by the use of the controller, they actually hold the smae proxy_mesh project. If a node were to get a measurement period command through the mesh network, then it functionally becomes a node, and responds back over a mesh sequence. The GATT server on the node is there, but completely dormant. The proxy is able to get its own readings and store them locally.

### SiWx917Y Explorer Kit
This Wi-Fi kit was responsible for communicating with the proxy node, acting as the proxy between the BLE mesh, to the Google Firebase and MQTT broker. For starters, we utilized (bluetooth protocol stuff). From there, when it comes to our Google Firebase, we used an open-permission Real-Time Database to send messages over. Messages would only be sent if they were receieved from the MQTT client that listened to our topic, *WebToMesh*. Whenever we had to relay messages from the website, to the BLE mesh From there, we would make a HTTP Post request to our database to publish the information. Our requests were based in raw text files, as to make it as readable as possible for the BLE devices. Whenever information would be received from the GATT database, depending on the information, it will be sent through this pipeline to update the website.

### React Website
For our react website, we ended up using React Native. This is a mobile-first platform that allows for easy production between desktop and mobile applications. We used Expo as our components library, as they had a ubitqutious setup between mobile and desktop web development. 

Our React Website had two purposes. First off, was to make a websocket to our MQTT broker. This purpose was required in order to properly communicate with our SiWx917Y Explorer Kit, as it utilized MQTT to talk between our website and the boards. We found a particularly useful library, MQTT, that handled all of this quite easily. It subscribed to both *WebToMesh* for sending, and *MeshToWeb* for receiving. From there, our second purpose was to make a website that was easily nagivable to handle the MQTT request and read data from the sensors. We made a React Object in order to have something that lists all of the data of the boards, and we made two of these objects on a justified-evenly div that holds all of our data for both BLE boards. Including this, we alos have a text messaging bar to communicate betwee the website and the boards. This is done by the SiW917Y communicating between thew two. We have a specific command system that is sent in order to blink the leds on the boards, and to keep track of a counter as per assignment requirements.

## Closing Note
In short, this project handled various protocols such bluetooth low energy, bluetooth mesh, MQTT and HTTPS. Working with all of them, we managed to unite them into one project that has high applicability out in the field. 
