# Yet Another Protocol (YAP)

The YAP is being developed for the communication between the Zumo Robot and a client application.

## Table of Contents

- [Network Architecture](#network-architecture)
- [Frame](#frame)
- [Control Channel](#control-channel-channel-0)
  - [SYNC](#sync-d0--0x00)
  - [SYNC_RSP](#sync_rsp-d0--0x01)
  - [SCRB](#scrb-d0--0x02)
  - [SCRB_RSP](#scrb_rsp-d0--0x03)
- [Internal Architecture](#internal-architecture)

---

## Network Architecture

- Server-Client Architecture
- One-to-one. One Server to one client.

---

## Frame

The Protocol sends and received Frames of the following form:

```cpp
/** Data container of the Frame Fields */
typedef union _Frame
{
    struct _Fields
    {
        /** Header */
        union _Header
        {
            struct _HeaderFields
            {
                /** Channel ID */
                uint8_t m_channel;

                /** Frame Checksum */
                uint8_t m_checksum;

            } __attribute__((packed)) headerFields;

            /** Raw Header Data*/
            uint8_t rawHeader[HEADER_LEN];

        } __attribute__((packed)) header;

        /** Payload */
        struct _Payload
        {
            /** Data of the Frame */
            uint8_t m_data[MAX_DATA_LEN];

        } __attribute__((packed)) payload;

    } __attribute__((packed)) fields;

    /** Raw Frame Data */
    uint8_t raw[MAX_FRAME_LEN] = {0U};

} __attribute__((packed)) Frame;
```

### Header

#### Channel Field

- Length: 1 Byte.
- Channel on which the data is being sent.
- [Channel 0](#control-channel-channel-0) is reserved for the server.
- Channels 1 to 255 are "Data Channels".
- The Application can publish or subscribe to any of these channels using the channel's name.
- Client suscribes to a channel using [Channel 0](#control-channel-channel-0).

#### Checksum Field

- Simple Checksum.
- Applied to previous fields.
- checksum = sum(Channel + Data Bytes) % UINT8_MAX

### Payload Field

- Data Length: Set by the Channel definition.
- Contains Application Data Bytes.

---

## Control Channel (Channel 0)

- Control Channel: Exchange of Commands. Can not be used to send data.
- Channel 0 has no **external** Callback. The state of the server can be polled by the application through getter functions ( or similar depending on the implementation).
- D0 (Data Byte 0) is used as a Command Byte. Defines the command that is being sent.
- Even-number *Commands* in D0 are used as Commands, while uneven-number *Commands* are used as the response to the immediately previous (n-1) command.

### SYNC (D0 = 0x00)

- Server sends SYNC Command with current timestamp.
- Client responds with [SYNC_RSP](#sync_rsp-d0--0x01).
- Server can calculate Round-Trip-Time.
- SYNC Package must be sent periodically depending on current [State](#state-machine). The period is also used as a timeout for the previous SYNC.
- Used as a "Heartbeat" or "keep-alive" by the client.

### SYNC_RSP (D0 = 0x01)

- Client Response to [SYNC](#sync-d0--0x00).
- Data Payload is the same timestamp as in SYNC Command.

### SCRB (D0 = 0x02)

- Client sends the name of the channel it wants to suscribe to.
- Server responds with the channel number, DLC, and the channel name of the requested channel, if the channel is found and valid. If channel is not found, the response has channel number = 0, DLC = 0, and the channel name.

### SCRB_RSP (D0 = 0x03)

- Server Response to [SCRB](#scrb-d0--0x02).
- Channel Number on Data Byte 1 (D1).
- Channel DLC on Data Byte 2 (D2)
- Channel Name on the following bytes

---

## Internal Architecture

### Data

- Information is sent directly from application to the Serial Driver. No queueing or buffering.
- The Protocol can send a maximum of 255 Bytes.

### Channels

```cpp
/**
 * Channel Definition.
 */
struct Channel
{
    char            m_name[CHANNEL_NAME_MAX_LEN]; /**< Name of the channel. */
    uint8_t         m_dlc;                        /**< Payload length of channel */
    ChannelCallback m_callback;                   /**< Callback to provide received data to the application. */

    /**
     * Channel Constructor.
     */
    Channel() : m_name{0U}, m_dlc(0U), m_callback(nullptr)
    {
    }
};
```

- Channel has 3 members: Name, DLC, and callback function.

### Channel Creation and Subscription

![CreateSubscribeSequence](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonAlcer/main/doc/YAP/SubscribeSequence.puml)

#### Channel Creation

- Application initializes a channel with a name and a DLC, protocol looks for a free channel number and returns its channel number to the application.
- If no channel is free, it returns 0 as it is an invalid Data Channel.

#### Channel Subscription

- Application can subscribe to a remote data channel by its name and a callback to the function that must be called when data is received in said channel.
- Function has no return value, as the response from the server is asynchron.

### Callback

```cpp
/**
 * Channel Notification Prototype Callback.
 * Provides the received data in the respective channel to the application.
 */
typedef void (*ChannelCallback)(const uint8_t* payload, const uint8_t payloadSize);
```

- Callback passes only a pointer to the received Buffer. Data must be copied by application.
- Memory is freed by the protocol after the callback is done.
- DLC is passed as payloadSize to the application.

### State Machine

#### Out-of-Sync

- Client is disconnected/does not respond to SYNC Command.
- No Data is sent in this state.
- SYNC Period set to 1 second.

#### Synced

- Client is connected and responds to SYNC Commands.
- SYNC Period set to 5 seconds.
